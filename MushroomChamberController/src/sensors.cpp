#include "sensors.h"
#include <Adafruit_BME280.h>
#include <Wire.h>


// --- BME280 Setup ---
Adafruit_BME280 bme;
#define BME_ADDR 0x76

// Roughly 40 cm of wire to the sensor, so the bus runs below the 100 kHz default
#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_CLOCK 50000

// The sensor is not always ready the instant the ESP32 is, and a single failed
// begin() used to leave the chamber blind for the whole run
#define INIT_ATTEMPTS 5
#define INIT_RETRY_DELAY 200
#define REINIT_INTERVAL 5000

static bool sensorReady = false;

void setupSensors() {
  Serial.println("Initializing sensors...");
  Wire.begin(I2C_SDA, I2C_SCL, I2C_CLOCK);

  for (int i = 0; i < INIT_ATTEMPTS && !sensorReady; i++) {
    sensorReady = bme.begin(BME_ADDR);
    if (!sensorReady) delay(INIT_RETRY_DELAY);
  }

  Serial.println(sensorReady ? "✅ BME280 ready" : "Could not find BME280 sensor!");
}

// Retry init periodically so a sensor that drops out recovers without a reboot
static bool sensorAvailable() {
  static unsigned long lastAttempt = 0;

  if (!sensorReady && millis() - lastAttempt > REINIT_INTERVAL) {
    lastAttempt = millis();
    sensorReady = bme.begin(BME_ADDR);
  }
  return sensorReady;
}

float readTemperature() {
  if (!sensorAvailable()) return NAN;

  float value = bme.readTemperature();
  if (isnan(value)) sensorReady = false; // re-init on the next read
  return value;
}

float readHumidity() {
  return sensorAvailable() ? bme.readHumidity() : NAN;
}

float readPressure() {
  return sensorAvailable() ? bme.readPressure() / 100.0F : NAN; // in hPa
}
