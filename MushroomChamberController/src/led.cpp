#include "config.h"
#include "led.h"
#include <FastLED.h>
#include <time.h>  // For real-time functions

// --- LED Strip ---
CRGB leds[NUM_LEDS];

// --- Global Configuration ---
extern MushroomConfig mushroomconfig;

void setupLeds() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_MAX_MILLIAMPS);
  FastLED.clear();
  FastLED.show();
}

// File scope so it can be reported: the strip is the largest switched load on
// the board and had no telemetry at all.
static bool lightOn = false;

void controlLighting(const PhaseConfig& currentConfig) {

  // Get current time
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  int hour = timeinfo.tm_hour;

  bool shouldBeOn = false;

  // Handles cases where light period wraps around midnight
  if (currentConfig.lightStartHour <= currentConfig.lightEndHour) {
    shouldBeOn = (hour >= currentConfig.lightStartHour && hour < currentConfig.lightEndHour);
  } else {
    // e.g., lightStartHour = 20, lightEndHour = 6
    shouldBeOn = (hour >= currentConfig.lightStartHour || hour < currentConfig.lightEndHour);
  }

  if (shouldBeOn && !lightOn) {
    setLEDColor(currentConfig.lightColor);
    lightOn = true;
  } else if (!shouldBeOn && lightOn) {
    setLEDColor(CRGB::Black);
    lightOn = false;
  }
}

bool isLightOn() {
  return lightOn;
}

void setLEDColor(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}
