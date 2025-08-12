#include "wifi_comm.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi configuration
static WiFiConfig config;
static WiFiStatus currentStatus = WiFiStatus::DISCONNECTED;
static String lastError = "";

// Retry logic state
static unsigned long lastAttemptTime = 0;
static unsigned int currentRetries = 0;
static const unsigned long DEFAULT_RETRY_INTERVAL = 10000; // 10 seconds
static const unsigned int DEFAULT_MAX_RETRIES = 5;

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
  
  Serial.print("WiFi setup complete for SSID: ");
  Serial.println(config.ssid);
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
      // Stay in failed state until manual reset
      break;
      
    case WiFiStatus::CONNECTED:
      // This case is handled above
      break;
  }
}

bool wifiConnected() {
  return WiFi.status() == WL_CONNECTED && currentStatus == WiFiStatus::CONNECTED;
}

WiFiStatus getWiFiStatus() {
  return currentStatus;
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

  Serial.printf("Sending POST to: %s\n", serverUrl);
  Serial.printf("Payload: %s\n", jsonPayload.c_str());

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.printf("POST Response code: %d\n", httpResponseCode);
    
    if (httpResponseCode >= 200 && httpResponseCode < 300) {
      String response = http.getString();
      Serial.printf("Response: %s\n", response.c_str());
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
  String json = createSensorJson(humidity, temperature, pressure);
  return sendPostRequest(config.serverUrl.c_str(), json);
}

String createSensorJson(float humidity, float temperature, float pressure) {
  JsonDocument doc;

  doc["timestamp"] = millis();
  doc["device_id"] = WiFi.macAddress();
  doc["humidity"] = humidity;
  doc["temperature"] = temperature;
  doc["pressure"] = pressure;
  doc["wifi_rssi"] = WiFi.RSSI();

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