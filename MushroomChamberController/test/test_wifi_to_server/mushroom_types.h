#include <FastLED.h>
#ifndef MUSHROOM_TYPES_H
#define MUSHROOM_TYPES_H

enum GrowthPhase {
  INCUBATION,
  PRIMORDIA_FORMATION,
  FRUITING
};


struct PhaseConfig {
  float targetTemperature;
  float temperatureTolerance;

  float targetHumidity;
  float humidityTolerance;

  float targetPressure;
  float pressureTolerance;

  int lightStartHour;
  int lightEndHour;
  CRGB lightColor;
};

struct MushroomConfig {
  const char* name;
  PhaseConfig incubation;
  PhaseConfig primordiaFormation;
  PhaseConfig fruiting;
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