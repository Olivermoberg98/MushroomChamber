#include "config.h"
#include "led.h"
#include <FastLED.h>
#include <time.h>  // For real-time functions

// --- LED Strip ---
CRGB leds[NUM_LEDS];

// --- Global Configuration ---
extern MushroomConfig currentConfig;

void setupLeds() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void controlLighting() {
  static bool lightOn = false;

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

void setLEDColor(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}
