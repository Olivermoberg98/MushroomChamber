#include "actuators.h"
#include "config.h"
#include <Arduino.h>

// --- Pin Definitions ---
#define EXHAUST_FAN1_PIN 13  // Exhaust fan 1
#define EXHAUST_FAN2_PIN 12  // Exhaust fan 2  
#define INLET_FAN_PIN 14     // Inlet fan
#define HUMIDIFIER_PIN 17

// --- Global Configuration ---
extern GrowthPhase currentPhase;
extern PhaseConfig activePhaseConfig;

// --- Simple MIMO Controller State ---
struct MIMOState {
    // Current actuator states
    bool humidifierOn = false;
    float exhaustFanSpeed = 0.0f; // 0.0 to 1.0 - controls both exhaust fans
    bool inletFanOn = false;      // Inlet fan on/off
    
    // Error tracking
    float humidityError = 0.0f;
    float temperatureError = 0.0f;
    float pressureError = 0.0f;
    
    // Simple learning weights (how much each actuator affects each variable)
    float humidifier_affects_humidity = 1.0f;     // Strong positive effect
    float humidifier_affects_temperature = 0.2f;  // Slight warming effect
    float exhaust_affects_humidity = -0.4f;       // Drying effect (removes humid air)
    float exhaust_affects_temperature = -0.3f;    // Slight cooling effect
    float exhaust_affects_pressure = -1.0f;       // Strong pressure reduction
    float inlet_affects_humidity = -0.1f;         // Slight drying (brings in outside air)
    float inlet_affects_temperature = 0.1f;       // Slight warming (air circulation)
    float inlet_affects_pressure = 0.3f;          // Increases pressure (adds air)
    
    // Deadbands to prevent oscillation
    float humidityDeadband = 2.0f;
    float temperatureDeadband = 1.0f;
    float pressureDeadband = 50.0f;
    
    // Timing
    unsigned long lastUpdate = 0;
    unsigned long updateInterval = 5000; // Update every 5 seconds
} mimoState;

// --- Simple Moving Average Filter ---
struct SimpleFilter {
    float values[5] = {0,0,0,0,0};
    int index = 0;
    bool filled = false;
    
    float update(float input) {
        values[index] = input;
        index = (index + 1) % 5;
        if (index == 0) filled = true;
        
        float sum = 0;
        int count = filled ? 5 : index + 1;
        for (int i = 0; i < count; i++) {
            sum += values[i];
        }
        return sum / count;
    }
};

SimpleFilter humidityFilter, temperatureFilter, pressureFilter;

void setupActuators() {
    Serial.println("Initializing actuators...");
    pinMode(EXHAUST_FAN1_PIN, OUTPUT);
    pinMode(EXHAUST_FAN2_PIN, OUTPUT);
    pinMode(INLET_FAN_PIN, OUTPUT);
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    
    digitalWrite(EXHAUST_FAN1_PIN, LOW);
    digitalWrite(EXHAUST_FAN2_PIN, LOW);
    digitalWrite(INLET_FAN_PIN, LOW);
    digitalWrite(HUMIDIFIER_PIN, LOW);
}

void setExhaustFans(float speed) {
    speed = constrain(speed, 0.0f, 1.0f);
    mimoState.exhaustFanSpeed = speed;
    
    if (speed < 0.1f) {
        // Both exhaust fans off
        digitalWrite(EXHAUST_FAN1_PIN, LOW);
        digitalWrite(EXHAUST_FAN2_PIN, LOW);
    } else if (speed < 0.6f) {
        // Low speed - only one exhaust fan
        digitalWrite(EXHAUST_FAN1_PIN, HIGH);
        digitalWrite(EXHAUST_FAN2_PIN, LOW);
    } else {
        // High speed - both exhaust fans
        digitalWrite(EXHAUST_FAN1_PIN, HIGH);
        digitalWrite(EXHAUST_FAN2_PIN, HIGH);
    }
}

void setInletFan(bool on) {
    if (on != mimoState.inletFanOn) {
        digitalWrite(INLET_FAN_PIN, on ? HIGH : LOW);
        mimoState.inletFanOn = on;
    }
}

void setHumidifier(bool on) {
    if (on != mimoState.humidifierOn) {
        digitalWrite(HUMIDIFIER_PIN, on ? HIGH : LOW);
        mimoState.humidifierOn = on;
    }
}

// --- Simple Environmental Score ---
float calculateEnvironmentalScore(float humidity, float temperature, float pressure) {
    float hError = abs(humidity - activePhaseConfig.targetHumidity) / activePhaseConfig.humidityTolerance;
    float tError = abs(temperature - activePhaseConfig.targetTemperature) / activePhaseConfig.temperatureTolerance;
    float pError = abs(pressure - activePhaseConfig.targetPressure) / 100.0f;
    
    return 1.0f / (1.0f + hError + tError + 0.5f * pError);
}

// --- Main MIMO Control Function ---
void updateActuators(float rawHumidity, float rawTemperature, float rawPressure) {
    unsigned long now = millis();
    
    // Rate limit updates
    if (now - mimoState.lastUpdate < mimoState.updateInterval) {
        return;
    }
    mimoState.lastUpdate = now;
    
    // Filter inputs
    float humidity = humidityFilter.update(rawHumidity);
    float temperature = temperatureFilter.update(rawTemperature);
    float pressure = pressureFilter.update(rawPressure);
    
    // Calculate errors
    mimoState.humidityError = activePhaseConfig.targetHumidity - humidity;
    mimoState.temperatureError = activePhaseConfig.targetTemperature - temperature;
    mimoState.pressureError = activePhaseConfig.targetPressure - pressure;
    
    // --- MIMO Decision Logic ---
    
    // Calculate "desired effects" for each variable
    float humidityNeed = 0.0f;
    float temperatureNeed = 0.0f;
    float pressureNeed = 0.0f;
    
    if (abs(mimoState.humidityError) > mimoState.humidityDeadband) {
        humidityNeed = constrain(mimoState.humidityError / 10.0f, -1.0f, 1.0f);
    }
    
    if (abs(mimoState.temperatureError) > mimoState.temperatureDeadband) {
        temperatureNeed = constrain(mimoState.temperatureError / 5.0f, -1.0f, 1.0f);
    }
    
    if (abs(mimoState.pressureError) > mimoState.pressureDeadband) {
        pressureNeed = constrain(mimoState.pressureError / 200.0f, -1.0f, 1.0f);
    }
    
    // --- Calculate optimal actuator settings ---
    
    // Humidifier decision: mainly for humidity, but consider temperature impact
    float humidifierDesire = humidityNeed * mimoState.humidifier_affects_humidity;
    
    // Don't run humidifier if it would make temperature worse
    if (temperatureNeed < -0.3f && mimoState.humidifier_affects_temperature > 0) {
        humidifierDesire *= 0.5f; // Reduce humidifier use when too hot
    }
    
    // Exhaust fan decision: mainly for pressure and humidity control
    float exhaustDesire = 0.0f;
    exhaustDesire += humidityNeed * mimoState.exhaust_affects_humidity;      // Remove humid air
    exhaustDesire += temperatureNeed * mimoState.exhaust_affects_temperature; // Remove warm air
    exhaustDesire += pressureNeed * mimoState.exhaust_affects_pressure;      // Reduce pressure
    
    // Inlet fan decision: help with air circulation and pressure balance
    float inletDesire = 0.0f;
    inletDesire += humidityNeed * mimoState.inlet_affects_humidity;          // Bring in fresh air
    inletDesire += temperatureNeed * mimoState.inlet_affects_temperature;    // Air circulation
    inletDesire += pressureNeed * mimoState.inlet_affects_pressure;          // Increase pressure if needed
    
    // Smart coordination: Balance inlet/exhaust for optimal airflow
    if (exhaustDesire > 0.5f && pressureNeed < -0.2f) {
        // High exhaust + low pressure = need inlet to balance
        inletDesire = max(inletDesire, 0.3f);
    } else if (pressureNeed > 0.2f) {
        // High pressure = reduce exhaust, possibly increase inlet
        exhaustDesire *= 0.7f;
        inletDesire = max(inletDesire, 0.2f);
    }
    
    // Emergency overrides
    if (pressure > activePhaseConfig.targetPressure + 200.0f) {
        exhaustDesire = 1.0f; // Force high exhaust
        inletDesire = 0.0f;   // Turn off inlet
    }
    
    if (temperature > activePhaseConfig.targetTemperature + 3.0f) {
        exhaustDesire = max(exhaustDesire, 0.7f); // Force exhaust cooling
        humidifierDesire = min(humidifierDesire, 0.0f); // Stop humidifier
    }
    
    // --- Apply decisions ---
    setHumidifier(humidifierDesire > 0.3f);
    setExhaustFans(constrain(exhaustDesire, 0.0f, 1.0f));
    setInletFan(inletDesire > 0.2f);
    
    // --- Simple adaptive learning ---
    static float lastScore = 0.0f;
    static unsigned long lastLearnTime = 0;
    
    if (now - lastLearnTime > 60000) { // Learn every minute
        float currentScore = calculateEnvironmentalScore(humidity, temperature, pressure);
        
        if (currentScore < lastScore && lastScore > 0) {
            // Performance got worse, adjust weights slightly
            if (mimoState.humidifierOn && mimoState.humidityError > 0) {
                mimoState.humidifier_affects_humidity *= 0.95f; // Reduce weight
            }
            
            if (mimoState.exhaustFanSpeed > 0.5f && mimoState.temperatureError > 0) {
                mimoState.exhaust_affects_temperature *= 0.95f; // Reduce cooling effect
            }
            
            if (mimoState.inletFanOn && mimoState.pressureError < 0) {
                mimoState.inlet_affects_pressure *= 0.95f; // Reduce pressure effect
            }
        } else if (currentScore > lastScore) {
            // Performance improved, slightly increase weights
            if (mimoState.humidifierOn) {
                mimoState.humidifier_affects_humidity *= 1.02f;
                mimoState.humidifier_affects_humidity = min(mimoState.humidifier_affects_humidity, 2.0f);
            }
            
            if (mimoState.exhaustFanSpeed > 0.5f) {
                mimoState.exhaust_affects_temperature *= 1.02f;
                mimoState.exhaust_affects_temperature = min(mimoState.exhaust_affects_temperature, 1.0f);
            }
            
            if (mimoState.inletFanOn) {
                mimoState.inlet_affects_pressure *= 1.02f;
                mimoState.inlet_affects_pressure = min(mimoState.inlet_affects_pressure, 1.0f);
            }
        }
        
        lastScore = currentScore;
        lastLearnTime = now;
    }
    
    // Debug output
    static unsigned long lastDebug = 0;
    if (now - lastDebug > 10000) { // Every 10 seconds
        Serial.printf("MIMO: H=%.1f/%.1f T=%.1f/%.1f P=%.0f/%.0f Exhaust=%.2f Inlet=%s Hum=%s\n",
                     humidity, activePhaseConfig.targetHumidity,
                     temperature, activePhaseConfig.targetTemperature, 
                     pressure, activePhaseConfig.targetPressure,
                     mimoState.exhaustFanSpeed, mimoState.inletFanOn ? "ON" : "OFF",
                     mimoState.humidifierOn ? "ON" : "OFF");
        lastDebug = now;
    }
}

// --- Status functions ---
bool isHumidifierOn() { return mimoState.humidifierOn; }
bool areFansOn() { return mimoState.exhaustFanSpeed > 0.1f || mimoState.inletFanOn; }
float getCurrentFanSpeed() { return mimoState.exhaustFanSpeed; }
bool isVentilating() { return mimoState.exhaustFanSpeed > 0.1f || mimoState.inletFanOn; }

// --- Legacy compatibility functions ---
void turnFansOn() { 
    setExhaustFans(1.0f); 
    setInletFan(true);
}
void turnFansOff() { 
    setExhaustFans(0.0f);
    setInletFan(false);
}
void turnOnHumidifier() { setHumidifier(true); }
void turnOffHumidifier() { setHumidifier(false); }

// --- New individual control functions ---
void setFanSpeed(float speed) { setExhaustFans(speed); } // Legacy compatibility