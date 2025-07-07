#ifndef SENSORS_H
#define SENSORS_H

void setupSensors();
float readTemperature();
float readHumidity();
float readPressure(); // Only for BME280

#endif
