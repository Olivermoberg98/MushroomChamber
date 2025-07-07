#include <Arduino.h>
#include "sensors.h"
#include "actuators.h"
#include "led.h"
#include "config.h"

MushroomConfig currentConfig;

void setup() {
  Serial.begin(115200);
  
  // Initialize sensors and actuators
  setupSensors();
  setupActuators();
  setupLeds();
  setupTime();
}

void loop() {
  float temp, humidity, pressure;
  
  // Read temperature, humidity, and pressure
  temp = readTemperature();  
  humidity = readHumidity();  
  pressure = readPressure(); 
  
  // Print the data
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %");
  Serial.print(", Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  // Control fan/LEDs/humidifier based on sensor data and mushroom type
  MushroomConfig currentConfig = getMushroomConfig(OYSTER); // Example for Oyster mushroom

  delay(2000); // Wait for 2 seconds before the next loop iteration
}
