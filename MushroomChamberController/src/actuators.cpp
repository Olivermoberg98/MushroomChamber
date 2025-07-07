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
 
//  --- Global Configuration ---
extern MushroomConfig currentConfig;


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

void controlHumidity(float currentHumidity) {
  static bool humidifierOn = false;

  float low = currentConfig.targetHumidity - currentConfig.humidityTolerance;
  float high = currentConfig.targetHumidity + currentConfig.humidityTolerance;

  if (!humidifierOn && currentHumidity < low) {
    turnOnHumidifier();
    humidifierOn = true;
  } else if (humidifierOn && currentHumidity > high) {
    turnOffHumidifier();
    humidifierOn = false;
  }
}


void controlFan(bool turnOn) {
  // Control fan based on ...
}
