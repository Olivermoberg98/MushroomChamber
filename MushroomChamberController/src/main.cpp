#include <Arduino.h>
#include "sensors.h"
#include "actuators.h"
#include "led.h"
#include "config.h"
#include "wifi_comm.h"

// Global configuration and current phase
MushroomConfig currentConfig;
GrowthPhase currentPhase = INCUBATION; // Change as needed: INCUBATION, PRIMORDIA_FORMATION, FRUITING
GrowthPhase oldPhase = currentPhase; // Store the previous phase for comparison
PhaseConfig activePhaseConfig;

void setup() {
  Serial.begin(115200);

  // Set the mushroom type
  currentConfig = getMushroomConfig(SHIITAKE);
  activePhaseConfig = getActivePhaseConfig();
  
  // Initialize sensors and actuators
  setupSensors();
  setupActuators();
  setupLeds();
  // setupTime();

  // Initialize WiFi
  wifiSetup("#Telia-DA3228", "fc736346d1dST2A1", "http://192.168.1.126:3001");
}

void loop() {
  float temp, humidity, pressure;

  // Read environmental data
  temp = readTemperature();
  humidity = readHumidity();
  pressure = readPressure();

  // Print to serial
  Serial.print("Phase: ");
  Serial.print(growthPhaseToString(currentPhase));
  Serial.print(" | Temp: ");
  Serial.print(temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  // Handle WiFi connection retry logic
  wifiRetryLoop();

  if (wifiConnected()) {
    bool success = sendSensorData(humidity, temp, pressure);
    if (success) {
      Serial.println("✅ Data sent successfully!");
    } else {
      Serial.printf("❌ Failed to send data: %s\n", getLastError().c_str());
    }

    // --- Get current growth phase ---
    GrowthPhase newPhase = getCurrentPhase();
    
    // If ther current phase has changed, update the active config
    if (currentPhase != newPhase) {
      oldPhase = currentPhase;      // Store old phase
      currentPhase = newPhase;      // Update current phase
      activePhaseConfig = getActivePhaseConfig(); // Update config for new phase
    }
  } else {
    Serial.printf("WiFi Status: %s\n", getWiFiStatusString().c_str());
  }

  // --- Control system based on phase config ---
  updateActuators(humidity, temp, pressure);
  controlLighting(activePhaseConfig);     // Pass in active config with light timing/color

  delay(2000); // Loop delay
}
