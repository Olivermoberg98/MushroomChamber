// main.cpp

#include <Arduino.h>
#include "sensors.h"
#include "actuators.h"

void setup() {
  Serial.begin(115200);
  
  // Initialize sensors and actuators
  setupSensors();
  setupActuators();
}

void loop() {
  float temp, humidity;
  
  // Read temperature and humidity
  readTemperatureHumidity(temp, humidity);
  
  // Print the data
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  // Control fan/LEDs based on conditions
  if (humidity > 70) {
    controlFan(true);  // Turn on fan if humidity is high
  } else {
    controlFan(false); // Otherwise, turn it off
  }
  
  delay(2000);  // Wait before reading again
}
