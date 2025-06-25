// sensors.h

#ifndef SENSORS_H
#define SENSORS_H

#include "DHT.h"

#define DHTPIN 4           // DHT sensor connected to GPIO4
#define DHTTYPE DHT22      // Use DHT22 for better accuracy

// Function prototypes
void setupSensors();
void readTemperatureHumidity(float &temp, float &humidity);

#endif
