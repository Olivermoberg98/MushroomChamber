#ifndef WIFI_COMM_H
#define WIFI_COMM_H

#include <Arduino.h>

extern const char* ssid;
extern const char* password;
extern const char* serverUrl;  // Your dashboard server API endpoint

void wifiSetup(const char* ssid, const char* password);
void wifiRetryLoop();
bool wifiConnected();
bool sendPostRequest(const char* serverUrl, const String& jsonPayload);
String createSensorJson(float humidity, float temperature, float pressure);

#endif