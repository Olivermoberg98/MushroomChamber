#ifndef ACTUATORS_H
#define ACTUATORS_H

void setupActuators();

void turnFanOn();
void turnFanOff();

void turnHumidifierOn();
void turnHumidifierOff();

void controlHumidity(float currentHumidity, const PhaseConfig& config);
void controlVentilationCycle();
void controlLighting(const PhaseConfig& config);

void setLEDColor(uint8_t r, uint8_t g, uint8_t b);

float fanOnDuration;   // in seconds or minutes
float fanOffDuration;
bool fanAlwaysOn;

#endif
