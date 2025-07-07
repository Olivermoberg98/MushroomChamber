#include "config.h"
#include <WiFi.h>
#include <time.h>
#include <FastLED.h>

extern MushroomConfig currentConfig;
extern GrowthPhase currentPhase;

PhaseConfig getActivePhaseConfig() {
  switch (currentPhase) {
    case INCUBATION:
      return currentConfig.incubation;
    case PRIMORDIA_FORMATION:
      return currentConfig.primordiaFormation;
    case FRUITING:
      return currentConfig.fruiting;
    default:
      return currentConfig.fruiting;
  }
}

MushroomConfig getMushroomConfig(MushroomType type) {
  switch (type) {
    case SHIITAKE:
      return {
        "Shiitake",
        // Incubation phase
        {
          22.0, 1.0,
          95.0, 5.0,
          1013.0, 8.0,
          0, 0, CRGB::Black  // No light
        },
        // Primordia formation
        {
          15.0, 1.0,
          95.0, 5.0,
          1013.0, 8.0,
          8, 12, CRGB::White  // short light
        },
        // Fruiting phase
        {
          17.0, 1.0,
          85.0, 5.0,
          1013.0, 8.0,
          8, 20, CRGB::White  // longer light
        }
      };
    default:
      return {/* default config */};
  }
}


void setupTime() {
  configTime(0, 0, "pool.ntp.org");  // UTC
  // Optional: wait until time is synced
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for NTP time...");
    delay(1000);
  }
  Serial.println("Time synced!");
}
