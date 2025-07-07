#include "actuators.h"
#include "config.h"
#include <FastLED.h>

// --- Fan & Humidifier ---
#define FAN_PIN 16
#define HUMIDIFIER_PIN 17

// --- LED Strip ---
#define LED_PIN     5
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

// --- Global Configuration ---
extern MushroomConfig currentConfig;
extern GrowthPhase currentPhase; 

// --- Ventilation Cycle Config ---
struct VentilationCycle {
  unsigned long intervalMs = 10UL * 60UL * 1000UL;  // every 10 min
  unsigned long durationMs = 45UL * 1000UL;         // fan on for 45 sec
};

VentilationCycle ventilation = {};

// --- Internal ventilation state ---
static unsigned long lastVentilationTime = 0;
static bool ventilating = false;
static unsigned long ventilationStartTime = 0;

// --- Humidifier State ---
static bool humidifierOn = false;

void setupActuators() {
  pinMode(FAN_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(HUMIDIFIER_PIN, LOW);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void turnFanOn() {
  digitalWrite(FAN_PIN, HIGH);
}

void turnFanOff() {
  digitalWrite(FAN_PIN, LOW);
}

void turnOnHumidifier() {
  digitalWrite(HUMIDIFIER_PIN, HIGH);
}

void turnOffHumidifier() {
  digitalWrite(HUMIDIFIER_PIN, LOW);
}

// --- Smart Fan-Humidifier Coordination ---
void controlVentilationCycle() {
  unsigned long now = millis();

  if (!ventilating && (now - lastVentilationTime > ventilation.intervalMs)) {
    // Start ventilation
    turnFanOn();
    turnOffHumidifier(); // pause humidity control during ventilation
    ventilating = true;
    ventilationStartTime = now;
    lastVentilationTime = now;
  }

  if (ventilating && (now - ventilationStartTime > ventilation.durationMs)) {
    // End ventilation
    turnFanOff();
    ventilating = false;
    // Don't turn humidifier on here — wait for RH logic
  }
}

// --- Independent Humidity Control ---
void controlHumidity(float currentHumidity, const PhaseConfig& config) {
  if (ventilating) return;

  float low = config.targetHumidity - config.humidityTolerance;
  float high = config.targetHumidity + config.humidityTolerance;

  if (!humidifierOn && currentHumidity < low) {
    turnOnHumidifier();
    humidifierOn = true;
  } else if (humidifierOn && currentHumidity > high) {
    turnOffHumidifier();
    humidifierOn = false;
  }
}
