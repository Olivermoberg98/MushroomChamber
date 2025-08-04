#include "wifi_comm.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "your-ssid";
const char* password = "your-password";
const char* serverUrl = "http://your.server/api";

// Retry logic state
static unsigned long lastAttemptTime = 0;
static const unsigned long retryIntervalMs = 10000; // 10 seconds between retries
static bool wifiConnecting = false;

void wifiSetup(const char* ssid, const char* password) {
  ssid = ssid;
  password = password;

  WiFi.mode(WIFI_STA);

  if (WiFi.status() != WL_CONNECTED && !wifiConnecting) {
    Serial.print("Starting WiFi connection to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    wifiConnecting = true;
    lastAttemptTime = millis();
  }
}

// Call this frequently in your loop to handle retries without blocking
void wifiRetryLoop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (wifiConnecting) {
      Serial.print("WiFi connected! IP: ");
      Serial.println(WiFi.localIP());
      wifiConnecting = false;
    }
    return;
  }

  unsigned long now = millis();
  if (wifiConnecting && (now - lastAttemptTime >= retryIntervalMs)) {
    Serial.println("WiFi not connected, retrying...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    lastAttemptTime = now;
  } else if (!wifiConnecting) {
    // Not connected and not trying — start connection
    WiFi.begin(ssid, password);
    wifiConnecting = true;
    lastAttemptTime = now;
  }
}

bool wifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool sendPostRequest(const char* serverUrl, const String& jsonPayload) {
  if (!wifiConnected()) {
    Serial.println("WiFi not connected, can't send POST");
    return false;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.print("POST Response code: ");
    Serial.println(httpResponseCode);
    http.end();
    return true;
  } else {
    Serial.print("Error on POST: ");
    Serial.println(http.errorToString(httpResponseCode));
    http.end();
    return false;
  }
}

String createSensorJson(float humidity, float temperature, float pressure) {
  JsonDocument doc;

  doc["humidity"] = humidity;
  doc["temperature"] = temperature;
  doc["pressure"] = pressure;

  String output;
  serializeJson(doc, output);
  return output;
}
