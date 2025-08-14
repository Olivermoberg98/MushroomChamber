#include "actuators.h"
#include "config.h"
#include <FastLED.h>
#include <algorithm>

// --- Fan & Humidifier ---
#define FAN1_PIN 13  // Updated pin numbers to match test configuration
#define FAN2_PIN 12
#define FAN3_PIN 14
#define HUMIDIFIER_PIN 17

// Score tracking
float lastScore = -1.0;
unsigned long lastScoreUpdate = 0;

// Adaptation parameters
unsigned long fanDurationMs = 10000;     // initial guess
unsigned long intervalBetweenFans = 300000; // initial interval (e.g., 5 min)

// --- Global Configuration ---
extern PhaseConfig currentConfig;
extern GrowthPhase currentPhase; 

// --- Environment Score Calculation ---
float environmentScore(float humidity, float pressure) {
  float hTarget = currentConfig.targetHumidity;
  float hTol = currentConfig.humidityTolerance;
  float pTarget = currentConfig.targetPressure;

  // Humidity penalty (quadratic if outside range)
  float hError = (humidity - hTarget) / hTol;
  float humidityPenalty = hError * hError;

  // Pressure penalty (penalize only if over)
  float pressurePenalty = std::max(0.0f, pressure - pTarget);
  pressurePenalty *= pressurePenalty;

  // Weighted score: higher is better
  return 1.0f / (1.0f + humidityPenalty + 0.01f * pressurePenalty);
}

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
    pinMode(FAN1_PIN, OUTPUT);
    pinMode(FAN2_PIN, OUTPUT);
    pinMode(FAN3_PIN, OUTPUT);
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    digitalWrite(FAN1_PIN, LOW);
    digitalWrite(FAN2_PIN, LOW);
    digitalWrite(FAN3_PIN, LOW);
    digitalWrite(HUMIDIFIER_PIN, LOW);
}

void turnFansOn() {
    digitalWrite(FAN1_PIN, HIGH);
    digitalWrite(FAN2_PIN, HIGH);
    digitalWrite(FAN3_PIN, HIGH);
}

void turnFansOff() {
    digitalWrite(FAN1_PIN, LOW);
    digitalWrite(FAN2_PIN, LOW);
    digitalWrite(FAN3_PIN, LOW);
}

void turnOnHumidifier() {
  digitalWrite(HUMIDIFIER_PIN, HIGH);
}

void turnOffHumidifier() {
  digitalWrite(HUMIDIFIER_PIN, LOW);
}

// --- Smart Fan-Humidifier Coordination ---
void controlVentilationCycle(float humidity, float pressure) {
  unsigned long now = millis();

  // --- Check if it's time to start ventilation ---
  if (!ventilating && (now - lastVentilationTime > intervalBetweenFans)) {
    // Start fan and pause humidifier
    turnFansOn();
    turnOffHumidifier();
    ventilating = true;

    // Update timers and log the current environment state
    ventilationStartTime = now;
    lastVentilationTime = now;
    lastScore = environmentScore(humidity, pressure);
    lastScoreUpdate = now;

    return; // Exit early to avoid evaluating stop logic in same loop
  }

  // --- Check if it's time to stop ventilation ---
  if (ventilating && (now - ventilationStartTime > fanDurationMs)) {
    turnFansOff();
    ventilating = false;

    // Calculate updated environment score
    float newScore = environmentScore(humidity, pressure);

    // --- Adaptive Fan Duration ---
    if (newScore > lastScore) {
      fanDurationMs = min(fanDurationMs + 1000UL, 60000UL); // max 60s
    } else {
      fanDurationMs = max(fanDurationMs - 1000UL, 5000UL);  // min 5s
    }

    // --- Adaptive Interval Between Fans ---
    if (newScore > lastScore) {
      intervalBetweenFans = max(intervalBetweenFans - 10000UL, 60000UL); // min 1 min
    } else {
      intervalBetweenFans = min(intervalBetweenFans + 10000UL, 900000UL); // max 15 min
    }

    lastScore = newScore;
    lastScoreUpdate = now;
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
