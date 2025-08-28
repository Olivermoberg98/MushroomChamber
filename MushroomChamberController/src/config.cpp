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
        { 25.0, 2.0, 75.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },
        { 14.0, 2.0, 95.0, 5.0, 1013.0, 8.0, 6, 10, CRGB::White },
        { 18.0, 2.0, 88.0, 5.0, 1013.0, 8.0, 8, 12, CRGB::White }
      };

    case OYSTER:
      return {
        "Oyster",
        { 25.0, 1.0, 88.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },
        { 14.0, 2.0, 95.0, 5.0, 1013.0, 8.0, 8, 12, CRGB::White },
        { 18.0, 2.0, 88.0, 5.0, 1013.0, 8.0, 8, 12, CRGB::White }
      };

    case KING_OYSTER:
      return {
        "King Oyster",
        { 23.0, 1.0, 88.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },
        { 16.0, 2.0, 95.0, 5.0, 1013.0, 8.0, 8, 12, CRGB::White },
        { 18.0, 2.0, 88.0, 5.0, 1013.0, 8.0, 8, 12, CRGB::White }
      };

    case SHIMEJI:
      return {
        "Shimeji",
        { 22.0, 1.0, 90.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },
        { 16.0, 2.0, 95.0, 5.0, 1013.0, 8.0, 8, 12, CRGB::White },
        { 17.0, 2.0, 90.0, 5.0, 1013.0, 8.0, 8, 12, CRGB::White }
      };

    case LIONS_MANE:
      return {
        "Lion's Mane",
        { 25.0, 1.0, 88.0, 5.0, 1013.0, 8.0, 0, 0, CRGB::Black },
        { 17.0, 1.0, 95.0, 5.0, 1013.0, 8.0, 6, 8, CRGB::White },
        { 18.0, 2.0, 90.0, 5.0, 1013.0, 8.0, 6, 8, CRGB::White }
      };
    default:
      return {/* default config */};
  }
}


// void setupTime() {
//   configTime(0, 0, "pool.ntp.org");  // UTC
//   // Optional: wait until time is synced
//   struct tm timeinfo;
//   while (!getLocalTime(&timeinfo)) {
//     Serial.println("Waiting for NTP time...");
//     delay(1000);
//   }
//   Serial.println("Time synced!");
// }
