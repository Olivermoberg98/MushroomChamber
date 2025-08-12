#include <Arduino.h>
#include "sensors.h"
#include "actuators.h"
#include "led.h"
#include "config.h"
#include "wifi_comm.h"

// Global configuration and current phase
MushroomConfig currentConfig;
GrowthPhase currentPhase = INCUBATION; // Change as needed: INCUBATION, PRIMORDIA_FORMATION, FRUITING

void setup() {
  Serial.begin(115200);

  // Set the mushroom type (example: Shiitake)
  currentConfig = getMushroomConfig(SHIITAKE);
  
  // Initialize sensors and actuators
  setupSensors();
  setupActuators();
  setupLeds();
  setupTime();

  // Initialize WiFi
  wifiSetup("#Telia", "fc22e1", "f3"); // Replace with your WiFi credentials
}

void loop() {
  float temp, humidity, pressure;

  // Read sensors
  temp = readTemperature();
  humidity = readHumidity();
  pressure = readPressure();

  // Print to serial
  Serial.print("Phase: ");
  if (currentPhase == INCUBATION) Serial.print("Incubation");
  else if (currentPhase == PRIMORDIA_FORMATION) Serial.print("Primordia");
  else if (currentPhase == FRUITING) Serial.print("Fruiting");
  Serial.print(" | Temp: ");
  Serial.print(temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  // Handle WiFi connection retry logic (non-blocking)
  wifiRetryLoop();

  if (wifiConnected()) {
    String jsonData = createSensorJson(68.2, 22.5, 1013.1);
    bool success = sendPostRequest("erferv", jsonData);
    if (success) {
      Serial.println("Data sent successfully!");
    } else {
      Serial.println("Failed to send data.");
    }
  }

  // --- Get active config for the current phase ---
  PhaseConfig activeConfig = getActivePhaseConfig();

  // --- Control system based on phase config ---
  controlVentilationCycle(humidity, pressure);        // Manages fan and humidifier pause logic
  controlHumidity(humidity, activeConfig);  // Pass in current humidity and active config
  controlLighting(activeConfig);     // Pass in active config with light timing/color

  delay(3000); // Loop delay
}
