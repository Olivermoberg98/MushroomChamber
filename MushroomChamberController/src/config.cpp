#include "config.h"
#include <WiFi.h>
#include <time.h>
#include <FastLED.h>

MushroomConfig getMushroomConfig(MushroomType type) {
  switch (type) {
    case SHIITAKE:
      return {
        "Shiitake",
        16.0, 1.5,       // Temperature and tolerance
        90.0, 5.0,       // Humidity and tolerance
        1013.0, 5.0,     // Pressure (atmospheric default), tolerance
        6, 18,           // Light start/end
        CRGB::White      // Light color (neutral/cool daylight)
      };

    case OYSTER:
      return {
        "Oyster",
        22.0, 1.0,
        85.0, 5.0,
        1013.0, 5.0,
        8, 20,
        CRGB::Blue
      };

    default:
      return {
        "Default",
        20.0, 2.0,
        85.0, 5.0,
        1013.0, 5.0,
        8, 20,
        CRGB::White
      };
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
