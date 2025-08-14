#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "mushroom_types.h"

// --- Setup Function ---
void setupActuators();

// --- Main Control Function ---
void updateActuators(float humidity, float temperature, float pressure);

// --- Individual Control Functions ---
void setFanSpeed(float speed);        // 0.0 to 1.0
void setHumidifier(bool on);

// --- Status Query Functions ---
bool isHumidifierOn();
bool areFansOn();
float getCurrentFanSpeed();
bool isVentilating();

// --- Legacy Functions (for backward compatibility) ---
void turnFansOn();
void turnFansOff();
void turnOnHumidifier();
void turnOffHumidifier();

#endif