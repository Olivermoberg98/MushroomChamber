#include <unity.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "wifi_comm.h"

// Your Raspberry Pi configuration
const char* WIFI_SSID = "#Telia-DA3228";        // Replace with your WiFi name
const char* WIFI_PASSWORD = "fc736346d1dST2A1"; // Replace with your WiFi password
const char* SERVER_URL = "http://192.168.1.126";   // Your Raspberry Pi IP

// Test configuration
const unsigned long TEST_DURATION_MS = 60000; // 1 minute
const unsigned long SEND_INTERVAL_MS = 1000;  // 1 second

// Test statistics
int totalAttempts = 0;
int successfulSends = 0;
int failedSends = 0;

void setUp(void) {
    Serial.println("\n--- Server Communication Test Setup ---");
}

void tearDown(void) {
    Serial.println("--- Server Communication Test Teardown ---");
}

// Test WiFi connection to your network
void test_wifi_connection() {
    Serial.println("Testing WiFi connection...");
    
    // Setup WiFi with your credentials
    wifiSetup(WIFI_SSID, WIFI_PASSWORD, SERVER_URL);
    
    // Wait for connection with timeout
    unsigned long startTime = millis();
    unsigned long timeout = 30000; // 30 second timeout
    
    while (!wifiConnected() && (millis() - startTime) < timeout) {
        wifiRetryLoop();
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    
    // Verify we're connected
    TEST_ASSERT_TRUE(wifiConnected());
    TEST_ASSERT_EQUAL(WiFiStatus::CONNECTED, getWiFiStatus());
    
    // Print connection details
    printWiFiStatus();
    
    Serial.println("✓ WiFi connection successful!");
}

// Test getting current phase from server
void test_get_current_phase() {
    Serial.println("Testing phase retrieval from server...");
    
    // Make sure we're connected
    TEST_ASSERT_TRUE(wifiConnected());
    
    // Try to get current phase
    GrowthPhase phase = getCurrentPhase();
    
    // Should get a valid phase (any of the three valid phases)
    TEST_ASSERT_TRUE(phase == INCUBATION || 
                    phase == PRIMORDIA_FORMATION || 
                    phase == FRUITING);
    
    Serial.printf("✓ Retrieved phase: %s\n", growthPhaseToString(phase).c_str());
}

// Test single sensor data send
void test_single_sensor_send() {
    Serial.println("Testing single sensor data send...");
    
    TEST_ASSERT_TRUE(wifiConnected());
    
    // Send mock sensor data
    float testHumidity = 65.5;
    float testTemperature = 23.2;
    float testPressure = 1013.25;
    
    bool result = sendSensorData(testHumidity, testTemperature, testPressure);
    
    TEST_ASSERT_TRUE(result);
    
    if (result) {
        Serial.println("✓ Single sensor data send successful!");
    } else {
        Serial.printf("✗ Send failed: %s\n", getLastError().c_str());
    }
}

// Main test: Send data continuously for 1 minute
void test_continuous_data_send() {
    Serial.println("Starting continuous data send test (1 minute)...");
    Serial.println("Sending mock sensor data every second for 60 seconds");
    
    TEST_ASSERT_TRUE(wifiConnected());
    
    unsigned long startTime = millis();
    unsigned long lastSendTime = 0;
    
    // Reset statistics
    totalAttempts = 0;
    successfulSends = 0;
    failedSends = 0;
    
    while ((millis() - startTime) < TEST_DURATION_MS) {
        // Check if it's time to send data
        if ((millis() - lastSendTime) >= SEND_INTERVAL_MS) {
            // Generate mock sensor data with some variation
            float humidity = 50.0 + (rand() % 40);        // 50-90%
            float temperature = 20.0 + (rand() % 15);     // 20-35°C
            float pressure = 1000.0 + (rand() % 50);      // 1000-1050 hPa
            
            // Attempt to send data
            totalAttempts++;
            bool success = sendSensorData(humidity, temperature, pressure);
            
            if (success) {
                successfulSends++;
                Serial.printf("[%02d] ✓ Sent: H=%.1f%%, T=%.1f°C, P=%.1f hPa\n", 
                            totalAttempts, humidity, temperature, pressure);
            } else {
                failedSends++;
                Serial.printf("[%02d] ✗ Failed: %s\n", 
                            totalAttempts, getLastError().c_str());
            }
            
            lastSendTime = millis();
        }
        
        // Keep WiFi alive
        wifiRetryLoop();
        delay(100);
    }
    
    // Print final statistics
    Serial.println("\n=== Test Results ===");
    Serial.printf("Total attempts: %d\n", totalAttempts);
    Serial.printf("Successful sends: %d\n", successfulSends);
    Serial.printf("Failed sends: %d\n", failedSends);
    Serial.printf("Success rate: %.1f%%\n", 
                  (float)successfulSends / totalAttempts * 100.0);
    Serial.println("===================");
    
    // Test passes if we have at least 80% success rate
    float successRate = (float)successfulSends / totalAttempts;
    TEST_ASSERT_TRUE(successRate >= 0.8);
    
    // Should have sent approximately 60 attempts (1 per second for 1 minute)
    TEST_ASSERT_TRUE(totalAttempts >= 55 && totalAttempts <= 65);
    
    Serial.println("✓ Continuous data send test completed successfully!");
}

// Test server connectivity and response
void test_server_response_validation() {
    Serial.println("Testing server response validation...");
    
    TEST_ASSERT_TRUE(wifiConnected());
    
    // Send test data and validate response
    bool result = sendSensorData(42.0, 21.0, 1010.0);
    TEST_ASSERT_TRUE(result);
    
    // If we got here, the server responded positively
    Serial.println("✓ Server responded correctly to sensor data");
    
    // Test phase endpoint
    GrowthPhase phase = getCurrentPhase();
    Serial.printf("✓ Current phase from server: %s\n", 
                  growthPhaseToString(phase).c_str());
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== ESP32 to Raspberry Pi Server Test ===");
    Serial.printf("Target server: %s\n", SERVER_URL);
    Serial.println("==========================================\n");
    
    UNITY_BEGIN();
    
    // Run tests in sequence
    RUN_TEST(test_wifi_connection);
    RUN_TEST(test_get_current_phase);
    RUN_TEST(test_single_sensor_send);
    RUN_TEST(test_server_response_validation);
    RUN_TEST(test_continuous_data_send);
    
    UNITY_END();
    
    Serial.println("\n=== All tests completed ===");
    Serial.println("Check your Raspberry Pi server logs to see received data!");
}

void loop() {
    // Tests run once in setup, keep WiFi alive in loop
    wifiRetryLoop();
    delay(1000);
}