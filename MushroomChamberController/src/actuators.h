#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "mushroom_types.h"

void setupActuators();

void turnFanOn();
void turnFanOff();

void turnHumidifierOn();
void turnHumidifierOff();

void controlHumidity(float currentHumidity, const PhaseConfig& config);
void controlVentilationCycle(float humidity, float pressure);

#endif
