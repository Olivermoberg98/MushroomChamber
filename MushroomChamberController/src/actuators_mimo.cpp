#include "actuators.h"
#include "config.h"
#include <Arduino.h>

// --- Pin Definitions ---
#define FAN1_PIN 13
#define FAN2_PIN 12
#define FAN3_PIN 14
#define HUMIDIFIER_PIN 17

// --- Global Configuration ---
extern GrowthPhase currentPhase;
extern PhaseConfig activePhaseConfig;

// --- Simple MIMO Controller State ---
struct MIMOState {
    // Current actuator states
    bool humidifierOn = false;
    float fanSpeed = 0.0f; // 0.0 to 1.0
    
    // Error tracking
    float humidityError = 0.0f;
    float temperatureError = 0.0f;
    float pressureError = 0.0f;
    
    // Simple learning weights (how much each actuator affects each variable)
    float humidifier_affects_humidity = 1.0f;     // Strong positive effect
    float humidifier_affects_temperature = 0.2f;  // Slight warming effect
    float fans_affect_humidity = -0.3f;           // Drying effect
    float fans_affect_temperature = -0.5f;        // Cooling effect
    float fans_affect_pressure = -1.0f;           // Strong pressure reduction
    
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
    pinMode(FAN1_PIN, OUTPUT);
    pinMode(FAN2_PIN, OUTPUT);
    pinMode(FAN3_PIN, OUTPUT);
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    
    digitalWrite(FAN1_PIN, LOW);
    digitalWrite(FAN2_PIN, LOW);
    digitalWrite(FAN3_PIN, LOW);
    digitalWrite(HUMIDIFIER_PIN, LOW);
}

void setFanSpeed(float speed) {
    speed = constrain(speed, 0.0f, 1.0f);
    mimoState.fanSpeed = speed;
    
    if (speed < 0.1f) {
        digitalWrite(FAN1_PIN, LOW);
        digitalWrite(FAN2_PIN, LOW);
        digitalWrite(FAN3_PIN, LOW);
    } else if (speed < 0.4f) {
        digitalWrite(FAN1_PIN, HIGH);
        digitalWrite(FAN2_PIN, LOW);
        digitalWrite(FAN3_PIN, LOW);
    } else if (speed < 0.7f) {
        digitalWrite(FAN1_PIN, HIGH);
        digitalWrite(FAN2_PIN, HIGH);
        digitalWrite(FAN3_PIN, LOW);
    } else {
        digitalWrite(FAN1_PIN, HIGH);
        digitalWrite(FAN2_PIN, HIGH);
        digitalWrite(FAN3_PIN, HIGH);
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
    
    // Fan decision: consider all three variables
    float fanDesire = 0.0f;
    fanDesire += humidityNeed * mimoState.fans_affect_humidity;    // Fans dry
    fanDesire += temperatureNeed * mimoState.fans_affect_temperature; // Fans cool
    fanDesire += pressureNeed * mimoState.fans_affect_pressure;   // Fans reduce pressure
    
    // Emergency overrides
    if (pressure > activePhaseConfig.targetPressure + 200.0f) {
        fanDesire = 1.0f; // Force high fan speed for high pressure
    }
    
    if (temperature > activePhaseConfig.targetTemperature + 3.0f) {
        fanDesire = max(fanDesire, 0.7f); // Force cooling
        humidifierDesire = min(humidifierDesire, 0.0f); // Stop humidifier
    }
    
    // --- Apply decisions ---
    setHumidifier(humidifierDesire > 0.3f);
    setFanSpeed(constrain(fanDesire, 0.0f, 1.0f));
    
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
            
            if (mimoState.fanSpeed > 0.5f && mimoState.temperatureError > 0) {
                mimoState.fans_affect_temperature *= 0.95f; // Reduce cooling effect
            }
        } else if (currentScore > lastScore) {
            // Performance improved, slightly increase weights
            if (mimoState.humidifierOn) {
                mimoState.humidifier_affects_humidity *= 1.02f;
                mimoState.humidifier_affects_humidity = min(mimoState.humidifier_affects_humidity, 2.0f);
            }
            
            if (mimoState.fanSpeed > 0.5f) {
                mimoState.fans_affect_temperature *= 1.02f;
                mimoState.fans_affect_temperature = min(mimoState.fans_affect_temperature, 1.0f);
            }
        }
        
        lastScore = currentScore;
        lastLearnTime = now;
    }
    
    // Debug output
    static unsigned long lastDebug = 0;
    if (now - lastDebug > 10000) { // Every 10 seconds
        Serial.printf("MIMO: H=%.1f/%.1f T=%.1f/%.1f P=%.0f/%.0f Fan=%.2f Hum=%s\n",
                     humidity, activePhaseConfig.targetHumidity,
                     temperature, activePhaseConfig.targetTemperature, 
                     pressure, activePhaseConfig.targetPressure,
                     mimoState.fanSpeed, mimoState.humidifierOn ? "ON" : "OFF");
        lastDebug = now;
    }
}

// --- Status functions ---
bool isHumidifierOn() { return mimoState.humidifierOn; }
bool areFansOn() { return mimoState.fanSpeed > 0.1f; }
float getCurrentFanSpeed() { return mimoState.fanSpeed; }
bool isVentilating() { return mimoState.fanSpeed > 0.1f; }

// --- Legacy compatibility functions ---
void turnFansOn() { setFanSpeed(1.0f); }
void turnFansOff() { setFanSpeed(0.0f); }
void turnOnHumidifier() { setHumidifier(true); }
void turnOffHumidifier() { setHumidifier(false); }