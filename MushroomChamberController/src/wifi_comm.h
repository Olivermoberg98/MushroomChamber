#ifndef WIFI_COMM_H
#define WIFI_COMM_H

#include <Arduino.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverUrl = "http://192.168.1.93:3001/api/data";  // Your dashboard server API endpoint

void wifiSetup(const char* ssid, const char* password);
bool wifiConnected();
bool sendPostRequest(const char* serverUrl, const String& jsonPayload);
String createSensorJson(float humidity, float temperature, float pressure);
void wifiRetryLoop();

#endif