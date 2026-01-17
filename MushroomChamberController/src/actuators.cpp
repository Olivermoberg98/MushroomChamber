#include "actuators.h"
#include "config.h"
#include <Arduino.h>

// --- Pin Definitions ---
#define EXHAUST_FAN1_PIN 13
#define EXHAUST_FAN2_PIN 12
#define INLET_FAN_PIN 14
#define HUMIDIFIER_PIN 15

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
  
  // Adaptive parameters (will self-tune)
  float humidityOvershoot = 3.0f;           // How much to overshoot target
  unsigned long humidifyDuration = 60000;   // How long to humidify (ms)
  unsigned long stabilizeDuration = 300000; // How long to stabilize (5 min)
  unsigned long ventilationDuration = 30000; // How long to ventilate (30 sec)
  unsigned long ventilationInterval = 900000; // How often to ventilate (15 min)
  
  // Learning variables
  unsigned long lastVentilationTime = 0;
  float humidityBeforeVentilation = 0.0f;
  float humidityAfterVentilation = 0.0f;
  float humidityBuildRate = 0.0f;           // %RH per second
  float humidityDecayRate = 0.0f;           // %RH per second
  
  // Safety limits
  float criticalLowHumidity = 70.0f;        // Emergency humidify threshold
  float criticalHighTemp = 30.0f;           // Emergency ventilation threshold
  
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
  
  pinMode(EXHAUST_FAN1_PIN, OUTPUT);
  pinMode(EXHAUST_FAN2_PIN, OUTPUT);
  pinMode(INLET_FAN_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  
  digitalWrite(EXHAUST_FAN1_PIN, LOW);
  digitalWrite(EXHAUST_FAN2_PIN, LOW);
  digitalWrite(INLET_FAN_PIN, LOW);
  digitalWrite(HUMIDIFIER_PIN, LOW);
  
  controller.stateStartTime = millis();
  controller.lastVentilationTime = millis();
  
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
    digitalWrite(EXHAUST_FAN1_PIN, on ? HIGH : LOW);
    digitalWrite(EXHAUST_FAN2_PIN, on ? HIGH : LOW);
    digitalWrite(INLET_FAN_PIN, on ? HIGH : LOW);
    controller.fansOn = on;
    Serial.printf("Fans: %s\n", on ? "ON (all)" : "OFF");
  }
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
    if (controller.state == STABILIZING && newState == VENTILATING) {
      controller.humidityBeforeVentilation = currentHumidity;
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
  unsigned long now = millis();
  
  // Rate limit to once per second
  if (now - lastUpdate < 1000) {
    return;
  }
  lastUpdate = now;
  
  // Filter humidity for stability
  if (controller.firstReading) {
    controller.filteredHumidity = rawHumidity;
    controller.lastHumidity = rawHumidity;
    controller.firstReading = false;
  } else {
    controller.filteredHumidity = filterValue(rawHumidity, controller.filteredHumidity, 0.2f);
  }
  
  float humidity = controller.filteredHumidity;
  float temperature = rawTemperature;
  
  unsigned long timeInState = now - controller.stateStartTime;
  unsigned long timeSinceVentilation = now - controller.lastVentilationTime;
  
  float targetHumidity = activePhaseConfig.targetHumidity;
  float humidityError = targetHumidity - humidity;
  
  // Calculate humidity change rate for learning
  float humidityDelta = humidity - controller.lastHumidity;
  controller.lastHumidity = humidity;
  
  // --- EMERGENCY OVERRIDES (highest priority) ---
  
  // Critical low humidity - force humidifier on
  if (humidity < controller.criticalLowHumidity) {
    if (controller.state != HUMIDIFYING) {
      Serial.printf("🚨 EMERGENCY: Critical low humidity (%.1f%%) - forcing humidification\n", humidity);
      changeState(HUMIDIFYING, humidity);
    }
    setHumidifier(true);
    setFans(false);
    return;
  }
  
  // Critical high temperature - force ventilation
  if (temperature > controller.criticalHighTemp) {
    if (controller.state != VENTILATING) {
      Serial.printf("🚨 EMERGENCY: High temperature (%.1f°C) - forcing ventilation\n", temperature);
      changeState(VENTILATING, humidity);
    }
    setHumidifier(false);
    setFans(true);
    return;
  }
  
  // --- STATE MACHINE ---
  
  switch (controller.state) {
    
    case HUMIDIFYING: {
      setHumidifier(true);
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
      // Timeout safety (don't humidify forever)
      else if (timeInState > 180000) { // 3 minutes max
        Serial.println("⚠️  Humidification timeout - moving to stabilization");
        changeState(STABILIZING, humidity);
      }
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
      // Time for periodic ventilation?
      else if (timeSinceVentilation > controller.ventilationInterval) {
        Serial.println("🌬️  Scheduled ventilation starting");
        changeState(VENTILATING, humidity);
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
        float expectedDrop = targetHumidity * 0.15f; // Expect ~15% relative drop
        float actualDrop = controller.humidityBeforeVentilation - humidity;
        
        if (actualDrop > expectedDrop * 1.5f) {
          // Dropped too much - reduce duration next time
          controller.ventilationDuration = max(15000UL, (unsigned long)(controller.ventilationDuration * 0.9f));
          Serial.printf("📊 Ventilation too strong - reducing to %lu sec\n", controller.ventilationDuration / 1000);
        } else if (actualDrop < expectedDrop * 0.5f) {
          // Didn't drop enough - increase duration
          controller.ventilationDuration = min(60000UL, (unsigned long)(controller.ventilationDuration * 1.1f));
          Serial.printf("📊 Ventilation too weak - increasing to %lu sec\n", controller.ventilationDuration / 1000);
        }
        
        changeState(RECOVERING, humidity);
      }
      break;
    }
    
    case RECOVERING: {
      setHumidifier(true);
      setFans(false);
      
      // Recover until we're back near target
      if (humidity >= targetHumidity - 1.0f) {
        Serial.printf("✅ Recovery complete: %.1f%% (target: %.1f%%)\n", humidity, targetHumidity);
        changeState(STABILIZING, humidity);
      }
      // Timeout
      else if (timeInState > 120000) { // 2 minutes max
        Serial.println("⚠️  Recovery timeout - moving to stabilization");
        changeState(STABILIZING, humidity);
      }
      break;
    }
  }
  
  // --- PERIODIC STATUS LOG (every 30 seconds) ---
  if (now - lastStatusLog > 30000) {
    Serial.println("\n========== Controller Status ==========");
    Serial.printf("State: %s (%.0f sec)\n", stateToString(controller.state), timeInState / 1000.0f);
    Serial.printf("Environment: H=%.1f%% (target %.1f%%), T=%.1f°C, P=%.0f hPa\n",
                 humidity, targetHumidity, temperature, rawPressure);
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
bool areFansOn() { return controller.fansOn; }
float getCurrentFanSpeed() { return controller.fansOn ? 1.0f : 0.0f; }
bool isVentilating() { return controller.state == VENTILATING; }

// --- Manual Control Functions ---
void turnFansOn() { setFans(true); }
void turnFansOff() { setFans(false); }
void turnOnHumidifier() { setHumidifier(true); }
void turnOffHumidifier() { setHumidifier(false); }
void setFanSpeed(float speed) { setFans(speed > 0.5f); }