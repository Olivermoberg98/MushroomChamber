#ifndef WIFI_COMM_H
#define WIFI_COMM_H

#include <Arduino.h>

// WiFi connection status enum for better status tracking
enum class WiFiStatus {
  DISCONNECTED,
  CONNECTING,
  CONNECTED,
  CONNECTION_FAILED,
  RECONNECTING
};

// Structure to hold WiFi configuration
struct WiFiConfig {
  String ssid;
  String password;
  String serverUrl;
  unsigned long retryInterval;
  unsigned int maxRetries;
};

// WiFi management functions
void wifiSetup(const char* ssid, const char* password, const char* serverUrl);
void wifiRetryLoop();
bool wifiConnected();
WiFiStatus getWiFiStatus();
String getWiFiStatusString();

// HTTP communication functions
bool sendPostRequest(const char* serverUrl, const String& jsonPayload);
bool sendSensorData(float humidity, float temperature, float pressure);

// JSON utility functions
String createSensorJson(float humidity, float temperature, float pressure);

// Configuration functions
void setRetryInterval(unsigned long intervalMs);
void setMaxRetries(unsigned int retries);
void setServerUrl(const char* url);

// Status and debug functions
void printWiFiStatus();
String getLastError();

#endif