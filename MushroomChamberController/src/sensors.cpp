// sensors.cpp

#include "sensors.h"

DHT dht(DHTPIN, DHTTYPE);

void setupSensors() {
  dht.begin();
}

void readTemperatureHumidity(float &temp, float &humidity) {
  temp = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}
