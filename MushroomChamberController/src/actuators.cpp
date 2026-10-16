#include "actuators.h"
#include "config.h"
#include <Arduino.h>

// --- Pin Definitions ---
#define EXHAUST_FAN_PIN 13
#define INLET_FAN_PIN 14
#define HUMIDIFIER_PIN 15

// Shut actuators down if no valid reading arrives within this window
#define SENSOR_TIMEOUT 30000
#define FAN_STAGGER_MS 250

// --- Global Configuration ---
extern GrowthPhase currentPhase;
extern PhaseConfig activePhaseConfig;

// --- Controller States ---
enum ControllerState {
  HUMIDIFYING,      // Building up humidity
  STABILIZING,      // Letting system settle
  VENTILATING,      // Fresh air exchange
  RECOVERING        // Rebuilding after ventilation
};

// --- Adaptive Controller ---
struct AdaptiveController {
  // Current state
  ControllerState state = STABILIZING;
  unsigned long stateStartTime = 0;
  
  // Actuator states
  bool humidifierOn = false;
  bool fansOn = false;
  // Tracked per pin so the dashboard reports each fan, even though the control
  // logic currently drives the pair together
  bool inletFanOn = false;
  bool exhaustFanOn = false;
  
  // Adaptive parameters (will self-tune)
  float humidityOvershoot = 1.0f;           // How much to overshoot target
  unsigned long humidifyDuration = 60000;   // How long to humidify (ms)
  unsigned long stabilizeDuration = 300000; // How long to stabilize (5 min)
  unsigned long ventilationDuration = 45000; // How long to ventilate (45 sec)
  unsigned long ventilationInterval = 300000; // How often to ventilate (5 min) - oysters need heavy FAE
  float wetVentMargin = 5.0f;               // Vent early above target plus this
  unsigned long wetVentGap = 120000;        // Min time between wet vents (2 min)
  float targetVentilationDrop = 3.0f;       // Aim for this %RH drop per vent

  // Continuous fog outruns what the air absorbs and pools as condensation
  unsigned long pulseOn = 20000;
  unsigned long pulseOff = 40000;

  // Dry tank detection
  unsigned long humidifyOnTime = 0;         // Fogging time without humidity rising
  float humidifyLow = 100.0f;               // Lowest humidity since it last rose
  unsigned long dryTankLimit = 480000;      // Give up after 8 min of fogging
  unsigned long dryTankRetry = 1800000;     // Probe again every 30 min
  bool humidifierFault = false;
  unsigned long faultTime = 0;

  // Learning variables
  unsigned long lastVentilationTime = 0;
  float humidityBeforeVentilation = 0.0f;
  float humidityAfterVentilation = 0.0f;
  float humidityBuildRate = 0.0f;           // %RH per second
  float humidityDecayRate = 0.0f;           // %RH per second
  
  // Safety limits
  unsigned long lastValidReading = 0;
  
  // Statistics for tuning
  int humidificationCycles = 0;
  int ventilationCycles = 0;
  unsigned long totalHumidifyTime = 0;
  
  // Filters for stability
  float filteredHumidity = 0.0f;
  float lastHumidity = 0.0f;
  bool firstReading = true;
} controller;

// --- Simple exponential filter ---
float filterValue(float newValue, float oldValue, float alpha = 0.3f) {
  return alpha * newValue + (1.0f - alpha) * oldValue;
}

void setupActuators() {
  Serial.println("Initializing Adaptive State Controller...");
  
  pinMode(EXHAUST_FAN_PIN, OUTPUT);
  pinMode(INLET_FAN_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  
  digitalWrite(EXHAUST_FAN_PIN, LOW);
  digitalWrite(INLET_FAN_PIN, LOW);
  digitalWrite(HUMIDIFIER_PIN, LOW);
  
  controller.stateStartTime = millis();
  controller.lastVentilationTime = millis();
  controller.lastValidReading = millis();
  
  Serial.println("✅ Adaptive controller initialized");
  Serial.printf("Initial parameters:\n");
  Serial.printf("  Humidity overshoot: %.1f%%\n", controller.humidityOvershoot);
  Serial.printf("  Stabilization time: %lu sec\n", controller.stabilizeDuration / 1000);
  Serial.printf("  Ventilation interval: %lu min\n", controller.ventilationInterval / 60000);
}

void setHumidifier(bool on) {
  if (on != controller.humidifierOn) {
    digitalWrite(HUMIDIFIER_PIN, on ? HIGH : LOW);
    controller.humidifierOn = on;
    Serial.printf("Humidifier: %s\n", on ? "ON" : "OFF");
  }
}

void setFans(bool on) {
  if (on != controller.fansOn) {
    digitalWrite(EXHAUST_FAN_PIN, on ? HIGH : LOW);
    controller.exhaustFanOn = on;
    delay(FAN_STAGGER_MS); // stagger so both motors don't draw inrush together
    digitalWrite(INLET_FAN_PIN, on ? HIGH : LOW);
    controller.inletFanOn = on;
    controller.fansOn = on;
    Serial.printf("Fans: %s\n", on ? "ON (inlet + exhaust)" : "OFF");
  }
}

void pulseHumidifier(unsigned long now) {
  if (controller.humidifierFault) {
    setHumidifier(false);
    return;
  }
  setHumidifier(now % (controller.pulseOn + controller.pulseOff) < controller.pulseOn);
}

const char* stateToString(ControllerState state) {
  switch (state) {
    case HUMIDIFYING: return "HUMIDIFYING";
    case STABILIZING: return "STABILIZING";
    case VENTILATING: return "VENTILATING";
    case RECOVERING: return "RECOVERING";
    default: return "UNKNOWN";
  }
}

void changeState(ControllerState newState, float currentHumidity) {
  if (newState != controller.state) {
    Serial.printf("\n🔄 State: %s → %s\n", 
                  stateToString(controller.state),
                  stateToString(newState));
    
    // Record humidity at state transitions for learning
    if (newState == VENTILATING) {
      controller.humidityBeforeVentilation = currentHumidity;
      // Stamp the clock on entry, not only on completion. A run cut short by the
      // emergency override never reached the completion branch, so ventilation
      // stayed overdue and re-fired the instant humidity climbed back out of the
      // emergency band - straight into the fans again.
      controller.lastVentilationTime = millis();
    }
    if (controller.state == VENTILATING && newState == RECOVERING) {
      controller.humidityAfterVentilation = currentHumidity;
      controller.ventilationCycles++;
      
      // Learn from this ventilation cycle
      float humidityDrop = controller.humidityBeforeVentilation - controller.humidityAfterVentilation;
      Serial.printf("📊 Ventilation impact: %.1f%% → %.1f%% (drop: %.1f%%)\n",
                    controller.humidityBeforeVentilation,
                    controller.humidityAfterVentilation,
                    humidityDrop);
    }
    
    controller.state = newState;
    controller.stateStartTime = millis();
  }
}

void updateActuators(float rawHumidity, float rawTemperature, float rawPressure) {
  static unsigned long lastUpdate = 0;
  static unsigned long lastStatusLog = 0;
  static unsigned long lastSensorWarn = 0;
  unsigned long now = millis();
  
  // Rate limit to once per second
  if (now - lastUpdate < 1000) {
    return;
  }
  unsigned long elapsed = now - lastUpdate;
  lastUpdate = now;

  // Reject invalid readings - a single NaN would poison the filter permanently
  if (isnan(rawHumidity)) {
    if (now - controller.lastValidReading > SENSOR_TIMEOUT) {
      setHumidifier(false);
      setFans(false);
      if (now - lastSensorWarn > 30000) {
        Serial.println("⚠️  No valid humidity reading - actuators disabled");
        lastSensorWarn = now;
      }
    }
    return;
  }
  controller.lastValidReading = now;

  // Filter humidity for stability
  if (controller.firstReading) {
    controller.filteredHumidity = rawHumidity;
    controller.lastHumidity = rawHumidity;
    controller.firstReading = false;
  } else {
    controller.filteredHumidity = filterValue(rawHumidity, controller.filteredHumidity, 0.2f);
  }
  
  float humidity = controller.filteredHumidity;
  
  unsigned long timeInState = now - controller.stateStartTime;
  unsigned long timeSinceVentilation = now - controller.lastVentilationTime;
  
  float targetHumidity = activePhaseConfig.targetHumidity;
  float humidityError = targetHumidity - humidity;
  
  // Calculate humidity change rate for learning
  float humidityDelta = humidity - controller.lastHumidity;
  controller.lastHumidity = humidity;
  
  // --- DRY TANK DETECTION ---
  // Fogging that never lifts humidity means the tank is empty. Running on
  // regardless burns out the transducer and holds the chamber in HUMIDIFYING.
  if (humidity >= targetHumidity - 2.0f || humidity > controller.humidifyLow + 3.0f) {
    controller.humidifyOnTime = 0;
    controller.humidifyLow = humidity;
  } else {
    controller.humidifyLow = min(controller.humidifyLow, humidity);
    if (controller.humidifierOn) controller.humidifyOnTime += elapsed;
  }
  if (!controller.humidifierFault && controller.humidifyOnTime > controller.dryTankLimit) {
    Serial.println("🚱 Humidifier isn't raising humidity - tank empty? Disabled");
    controller.humidifierFault = true;
    controller.faultTime = now;
    setHumidifier(false);
  } else if (controller.humidifierFault && now - controller.faultTime > controller.dryTankRetry) {
    Serial.println("💧 Retrying humidifier");
    controller.humidifierFault = false;
    controller.humidifyOnTime = 0;
  }

  // Fresh air on a fixed schedule regardless of humidity - oysters stall on
  // CO2 long before a brief humidity dip hurts them. Vents are kept short so
  // the humidifier only has a small loss to make up.
  bool ventOverdue = timeSinceVentilation > controller.ventilationInterval;
  // Air near saturation condenses on the bag - vent it out rather than waiting
  bool tooWet = humidity > targetHumidity + controller.wetVentMargin &&
                timeSinceVentilation > controller.wetVentGap;
  if (controller.state != VENTILATING && controller.state != RECOVERING &&
      (ventOverdue || tooWet)) {
    Serial.printf("🌬️  %s ventilation starting (%.1f%%)\n",
                  tooWet ? "Wet" : "Scheduled", humidity);
    changeState(VENTILATING, humidity);
  }

  // --- STATE MACHINE ---
  
  switch (controller.state) {
    
    case HUMIDIFYING: {
      pulseHumidifier(now);
      setFans(false);
      
      // Check if we've reached target + overshoot
      if (humidity >= targetHumidity + controller.humidityOvershoot) {
        Serial.printf("✅ Target reached: %.1f%% (target: %.1f%% + %.1f%% overshoot)\n",
                     humidity, targetHumidity, controller.humidityOvershoot);
        
        // Record humidification time for learning
        controller.totalHumidifyTime += timeInState;
        controller.humidificationCycles++;
        
        // Estimate build rate
        if (timeInState > 5000) { // Only if we ran for at least 5 seconds
          controller.humidifyDuration = timeInState * 1.2f; // Add 20% buffer for next time
          Serial.printf("📊 Learned humidify duration: %lu sec\n", controller.humidifyDuration / 1000);
        }
        
        changeState(STABILIZING, humidity);
      }
      // No timeout - dry tank detection stops a humidifier that isn't working
      break;
    }
    
    case STABILIZING: {
      setHumidifier(false);
      setFans(false);
      
      // Monitor humidity drift during stabilization
      if (timeInState > 10000 && abs(humidityDelta) > 0.05f) {
        float driftRate = humidityDelta / 1.0f; // per second
        controller.humidityDecayRate = filterValue(abs(driftRate), controller.humidityDecayRate, 0.1f);
      }
      
      // If humidity drops too low, restart humidification
      if (humidity < targetHumidity - 2.0f) {
        Serial.printf("📉 Humidity dropped to %.1f%% - restarting humidification\n", humidity);
        changeState(HUMIDIFYING, humidity);
      }
      // If humidity is very high and stable, extend stabilization
      else if (humidity > targetHumidity + 5.0f && timeInState > controller.stabilizeDuration) {
        Serial.printf("📈 High humidity (%.1f%%) - extending stabilization\n", humidity);
        controller.stateStartTime = now; // Reset timer
      }
      break;
    }
    
    case VENTILATING: {
      setHumidifier(false);
      setFans(true);
      
      // Stop ventilation after duration
      if (timeInState > controller.ventilationDuration) {
        Serial.printf("✅ Ventilation complete (%.1f sec)\n", timeInState / 1000.0f);
        controller.lastVentilationTime = now;
        
        // Adaptive ventilation duration based on humidity drop
        float expectedDrop = controller.targetVentilationDrop;
        float actualDrop = controller.humidityBeforeVentilation - humidity;
        
        if (actualDrop > expectedDrop * 1.5f) {
          // Dropped too much - reduce duration next time
          controller.ventilationDuration = max(15000UL, (unsigned long)(controller.ventilationDuration * 0.9f));
          Serial.printf("📊 Ventilation too strong - reducing to %lu sec\n", controller.ventilationDuration / 1000);
        } else if (actualDrop < expectedDrop * 0.5f) {
          // Didn't drop enough - increase duration
          // Ceiling raised for the single inlet/exhaust pair - one fan pushes
          // less air per second, so a full exchange needs a longer run
          controller.ventilationDuration = min(60000UL, (unsigned long)(controller.ventilationDuration * 1.1f));
          Serial.printf("📊 Ventilation too weak - increasing to %lu sec\n", controller.ventilationDuration / 1000);
        }
        
        changeState(RECOVERING, humidity);
      }
      break;
    }
    
    case RECOVERING: {
      setFans(false);

      // Recover until we're back near target. Checked before switching the
      // humidifier on so a wet vent doesn't pulse it for a tick.
      if (humidity >= targetHumidity - 1.0f) {
        Serial.printf("✅ Recovery complete: %.1f%% (target: %.1f%%)\n", humidity, targetHumidity);
        changeState(STABILIZING, humidity);
      }
      // Timeout
      else if (timeInState > 120000) { // 2 minutes max
        Serial.println("⚠️  Recovery timeout - moving to stabilization");
        changeState(STABILIZING, humidity);
      }
      else {
        pulseHumidifier(now);
      }
      break;
    }
  }
  
  // --- PERIODIC STATUS LOG (every 30 seconds) ---
  if (now - lastStatusLog > 30000) {
    Serial.println("\n========== Controller Status ==========");
    Serial.printf("State: %s (%.0f sec)\n", stateToString(controller.state), timeInState / 1000.0f);
    Serial.printf("Environment: H=%.1f%% (target %.1f%%), T=%.1f°C, P=%.0f hPa\n",
                 humidity, targetHumidity, rawTemperature, rawPressure);
    Serial.printf("Actuators: Humidifier=%s, Fans=%s\n",
                 controller.humidifierOn ? "ON " : "OFF",
                 controller.fansOn ? "ON" : "OFF");
    Serial.printf("Next ventilation in: %.1f min\n",
                 (controller.ventilationInterval - timeSinceVentilation) / 60000.0f);
    Serial.printf("Cycles: Humidify=%d, Ventilate=%d\n",
                 controller.humidificationCycles, controller.ventilationCycles);
    Serial.println("======================================\n");
    lastStatusLog = now;
  }
}

// --- Status Functions ---
bool isHumidifierOn() { return controller.humidifierOn; }
bool isHumidifierFault() { return controller.humidifierFault; }
bool areFansOn() { return controller.fansOn; }
bool isInletFanOn() { return controller.inletFanOn; }
bool isExhaustFanOn() { return controller.exhaustFanOn; }
float getCurrentFanSpeed() { return controller.fansOn ? 1.0f : 0.0f; }
bool isVentilating() { return controller.state == VENTILATING; }
const char* getControllerState() { return stateToString(controller.state); }
unsigned long getVentilationDuration() { return controller.ventilationDuration; }

// --- Manual Control Functions ---
void turnFansOn() { setFans(true); }
void turnFansOff() { setFans(false); }
void turnOnHumidifier() { setHumidifier(true); }
void turnOffHumidifier() { setHumidifier(false); }
void setFanSpeed(float speed) { setFans(speed > 0.5f); }