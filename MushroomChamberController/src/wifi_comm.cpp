#include "wifi_comm.h"
#include "actuators.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_system.h>
#include <time.h>
#include "mushroom_types.h"
#include "led.h"
#include "config.h"

// Owned by main.cpp; reported so the dashboard can draw the target band without
// the server keeping its own copy of the species tables
extern PhaseConfig activePhaseConfig;

// WiFi configuration
static WiFiConfig config;
static WiFiStatus currentStatus = WiFiStatus::DISCONNECTED;
static String lastError = "";

// Retry logic state
static unsigned long lastAttemptTime = 0;
static unsigned int currentRetries = 0;
static const unsigned long DEFAULT_RETRY_INTERVAL = 10000; // 10 seconds
static const unsigned int DEFAULT_MAX_RETRIES = 5;
// A router reboot outlasts the retry budget above, so the failed state has to
// expire rather than latch.
static const unsigned long FAILED_RETRY_INTERVAL = 60000;
static unsigned int reconnectCount = 0;

void wifiSetup(const char* ssid, const char* password, const char* serverUrl) {
  config.ssid = String(ssid);
  config.password = String(password);
  config.serverUrl = String(serverUrl);
  config.retryInterval = DEFAULT_RETRY_INTERVAL;
  config.maxRetries = DEFAULT_MAX_RETRIES;
  
  currentStatus = WiFiStatus::DISCONNECTED;
  currentRetries = 0;
  lastError = "";

  WiFi.mode(WIFI_STA);

  // Association is the biggest current draw on the board; trading a few dB of
  // uplink margin for a lower peak keeps a marginal supply from collapsing
  WiFi.setTxPower(WIFI_POWER_13dBm);
  WiFi.setSleep(true);

  Serial.print("WiFi setup complete for SSID: ");
  Serial.println(config.ssid);
  delay(1000);
}

void wifiRetryLoop() {
  wl_status_t wifiStat = WiFi.status();
  
  // Update current status based on WiFi status
  if (wifiStat == WL_CONNECTED) {
    if (currentStatus != WiFiStatus::CONNECTED) {
      Serial.print("WiFi connected! IP: ");
      Serial.println(WiFi.localIP());
      currentStatus = WiFiStatus::CONNECTED;
      currentRetries = 0;
      lastError = "";
    }
    return;
  }

  unsigned long now = millis();
  
  // Handle connection attempts
  switch (currentStatus) {
    case WiFiStatus::DISCONNECTED:
      Serial.print("Starting WiFi connection to ");
      Serial.println(config.ssid);
      WiFi.begin(config.ssid.c_str(), config.password.c_str());
      currentStatus = WiFiStatus::CONNECTING;
      lastAttemptTime = now;
      break;
      
    case WiFiStatus::CONNECTING:
      if (now - lastAttemptTime >= config.retryInterval) {
        currentRetries++;
        if (currentRetries >= config.maxRetries) {
          Serial.println("Max retries reached, marking as failed");
          currentStatus = WiFiStatus::CONNECTION_FAILED;
          lastError = "Max connection retries exceeded";
          return;
        }
        
        Serial.printf("WiFi connection attempt %d/%d failed, retrying...\n", 
                     currentRetries, config.maxRetries);
        WiFi.disconnect();
        WiFi.begin(config.ssid.c_str(), config.password.c_str());
        currentStatus = WiFiStatus::RECONNECTING;
        lastAttemptTime = now;
      }
      break;
      
    case WiFiStatus::RECONNECTING:
      if (now - lastAttemptTime >= config.retryInterval) {
        currentRetries++;
        if (currentRetries >= config.maxRetries) {
          currentStatus = WiFiStatus::CONNECTION_FAILED;
          lastError = "Max reconnection retries exceeded";
          return;
        }
        
        WiFi.disconnect();
        WiFi.begin(config.ssid.c_str(), config.password.c_str());
        lastAttemptTime = now;
      }
      break;
      
    case WiFiStatus::CONNECTION_FAILED:
      // Latching here strands the node until someone power-cycles it: a brief
      // outage is enough to lose it for the rest of the run.
      if (now - lastAttemptTime >= FAILED_RETRY_INTERVAL) {
        Serial.println("Retrying WiFi after backoff");
        currentStatus = WiFiStatus::DISCONNECTED;
        currentRetries = 0;
        lastAttemptTime = now;
      }
      break;

    case WiFiStatus::CONNECTED:
      // Reached only once WiFi.status() has stopped reporting a link, so the
      // connection dropped under us. Without this the machine sits in CONNECTED
      // forever and never calls WiFi.begin() again.
      Serial.println("WiFi connection lost - reconnecting");
      reconnectCount++;
      currentStatus = WiFiStatus::DISCONNECTED;
      currentRetries = 0;
      lastAttemptTime = now;
      break;
  }
}

bool wifiConnected() {
  return WiFi.status() == WL_CONNECTED && currentStatus == WiFiStatus::CONNECTED;
}

WiFiStatus getWiFiStatus() {
  return currentStatus;
}

unsigned int getReconnectCount() {
  return reconnectCount;
}

String getWiFiStatusString() {
  switch (currentStatus) {
    case WiFiStatus::DISCONNECTED: return "DISCONNECTED";
    case WiFiStatus::CONNECTING: return "CONNECTING";
    case WiFiStatus::CONNECTED: return "CONNECTED";
    case WiFiStatus::CONNECTION_FAILED: return "CONNECTION_FAILED";
    case WiFiStatus::RECONNECTING: return "RECONNECTING";
    default: return "UNKNOWN";
  }
}

// Add this helper function to convert string to enum
GrowthPhase stringToGrowthPhase(const String& phaseStr) {
  if (phaseStr == "Incubation") {
    return INCUBATION;
  } else if (phaseStr == "Primordia") {
    return PRIMORDIA_FORMATION;
  } else if (phaseStr == "Fruiting") {
    return FRUITING;
  } else {
    // Default fallback
    Serial.printf("⚠️ Unknown phase '%s', defaulting to INCUBATION\n", phaseStr.c_str());
    return INCUBATION;
  }
}

// Add this helper function to convert enum to string (useful for debugging)
String growthPhaseToString(GrowthPhase phase) {
  switch (phase) {
    case INCUBATION: return "Incubation";
    case PRIMORDIA_FORMATION: return "Primordia";
    case FRUITING: return "Fruiting";
    default: return "Unknown";
  }
}

// Updated getCurrentPhase function
GrowthPhase getCurrentPhase() {
  HTTPClient http;
  String phaseUrl = config.serverUrl + "/api/phase";
  
  http.begin(phaseUrl.c_str());
  http.addHeader("User-Agent", "ESP32-Sensor");
  http.setTimeout(5000);

  // Serial.printf("Getting phase from: %s\n", phaseUrl.c_str());

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    // Serial.printf("GET Response code: %d\n", httpResponseCode);
    
    if (httpResponseCode >= 200 && httpResponseCode < 300) {
      String response = http.getString();
      Serial.printf("Phase response: %s\n", response.c_str());
      
      // Parse JSON response to extract phase
      JsonDocument doc;
      deserializeJson(doc, response);
      String phaseStr = doc["phase"];
      
      http.end();
      return stringToGrowthPhase(phaseStr);
    } else {
      lastError = "HTTP error code: " + String(httpResponseCode);
      http.end();
      return INCUBATION; 
    }
  } else {
    String error = http.errorToString(httpResponseCode);
    lastError = "HTTP client error: " + error;
    Serial.printf("Error on GET: %s\n", error.c_str());
    http.end();
    return INCUBATION;
  }
}

bool sendPostRequest(const char* serverUrl, const String& jsonPayload) {
  if (!wifiConnected()) {
    lastError = "WiFi not connected";
    Serial.println("WiFi not connected, can't send POST");
    return false;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-Sensor");
  http.setTimeout(10000); // 10 second timeout

  // Serial.printf("Sending POST to: %s\n", serverUrl);
  // Serial.printf("Payload: %s\n", jsonPayload.c_str());

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.printf("POST Response code: %d\n", httpResponseCode);
    
    if (httpResponseCode >= 200 && httpResponseCode < 300) {
      String response = http.getString();
      // Serial.printf("Response: %s\n", response.c_str());
      http.end();
      return true;
    } else {
      lastError = "HTTP error code: " + String(httpResponseCode);
      http.end();
      return false;
    }
  } else {
    String error = http.errorToString(httpResponseCode);
    lastError = "HTTP client error: " + error;
    Serial.printf("Error on POST: %s\n", error.c_str());
    http.end();
    return false;
  }
}

bool sendSensorData(float humidity, float temperature, float pressure) {
  String sensorUrl = String(config.serverUrl) + "/api/sensor-data";
  String json = createSensorJson(humidity, temperature, pressure);
  return sendPostRequest(sensorUrl.c_str(), json);
}

String createSensorJson(float humidity, float temperature, float pressure) {
  JsonDocument doc;

  doc["timestamp"] = millis();
  doc["device_id"] = WiFi.macAddress();
  doc["humidity"] = humidity;
  doc["temperature"] = temperature;
  doc["pressure"] = pressure;
  doc["wifi_rssi"] = WiFi.RSSI();

  // Actuator state, so the logged readings can be attributed to a cause
  doc["state"] = getControllerState();
  doc["humidifier_on"] = isHumidifierOn();
  doc["fans_on"] = areFansOn();
  doc["inlet_fan_on"] = isInletFanOn();
  doc["exhaust_fan_on"] = isExhaustFanOn();
  doc["vent_duration_ms"] = getVentilationDuration();

  doc["target_temperature"] = activePhaseConfig.targetTemperature;
  doc["target_humidity"] = activePhaseConfig.targetHumidity;

  // Health counters. uptime_ms dropping back marks a silent reboot, free_heap
  // sliding marks a leak, and wifi_reconnects climbing marks a flaky link.
  doc["uptime_ms"] = millis();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["min_free_heap"] = ESP.getMinFreeHeap();
  doc["reset_reason"] = (int)esp_reset_reason();
  doc["wifi_reconnects"] = reconnectCount;

  // The strip is the biggest switched load and had no telemetry, so an LED
  // switch-on could never be lined up against a failure. local_hour also
  // exposes the unsynced-clock case, where the schedule runs off a junk hour
  // and the strip can come on at any time.
  doc["led_on"] = isLightOn();
  doc["time_synced"] = isTimeSynced();
  time_t nowEpoch;
  struct tm timeinfo;
  time(&nowEpoch);
  localtime_r(&nowEpoch, &timeinfo);
  doc["local_hour"] = timeinfo.tm_hour;

  String output;
  serializeJson(doc, output);
  return output;
}

void setRetryInterval(unsigned long intervalMs) {
  config.retryInterval = intervalMs;
}

void setMaxRetries(unsigned int retries) {
  config.maxRetries = retries;
}

void setServerUrl(const char* url) {
  config.serverUrl = String(url);
}

void printWiFiStatus() {
  Serial.println("=== WiFi Status ===");
  Serial.printf("SSID: %s\n", config.ssid.c_str());
  Serial.printf("Status: %s\n", getWiFiStatusString().c_str());
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
  Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
  Serial.printf("Retries: %d/%d\n", currentRetries, config.maxRetries);
  if (!lastError.isEmpty()) {
    Serial.printf("Last Error: %s\n", lastError.c_str());
  }
  Serial.println("==================");
}

String getLastError() {
  return lastError;
}