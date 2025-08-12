#include "sensors.h"
#include <Adafruit_BME280.h>


// --- BME280 Setup ---
Adafruit_BME280 bme;
#define BME_ADDR 0x76


void setupSensors() {
  Serial.println("Initializing sensors...");

  if (!bme.begin(BME_ADDR)) {
    Serial.println("Could not find BME280 sensor!");
  }
}

float readTemperature() {
  return bme.readTemperature();
}

float readHumidity() {
  return bme.readHumidity();
}

float readPressure() {
  return bme.readPressure() / 100.0F; // in hPa
}
