#include <fastled.h>
#ifndef MUSHROOM_TYPES_H
#define MUSHROOM_TYPES_H

struct MushroomConfig {
  const char* name;
  float targetTemperature;
  float temperatureTolerance;  // e.g., ±1°C

  float targetHumidity;
  float humidityTolerance;     // e.g., ±5%

  float targetPressure;
  float pressureTolerance;     // optional, for CO2-aware systems

  int lightStartHour;     // e.g., 8  (08:00)
  int lightEndHour;       // e.g., 20 (20:00)
  CRGB lightColor;
};

enum MushroomType {
  OYSTER,
  SHIITAKE,
  PORTOBELLO,
  BUTTON,
  ENOKI,
  KING_OYSTER,
  LIONS_MANE,
  MAITAKE,
  REISHI,
  CHAGA
};


#endif