#include <unity.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "wifi_comm.h"

void setUp(void) {
    Serial.println("\n--- Offline Test Setup ---");
}

void tearDown(void) {
    Serial.println("--- Offline Test Teardown ---");
}

// Test JSON creation with various data types
void test_json_creation_comprehensive() {
    Serial.println("Testing comprehensive JSON creation...");
    
    // Test normal values
    String json1 = createSensorJson(45.6f, 23.4f, 1013.25f);
    TEST_ASSERT_FALSE(json1.isEmpty());
    
    // Test integer values (should work as floats)
    String json2 = createSensorJson(50, 25, 1000);
    TEST_ASSERT_FALSE(json2.isEmpty());
    
    // Test zero values
    String json3 = createSensorJson(0.0f, 0.0f, 0.0f);
    TEST_ASSERT_FALSE(json3.isEmpty());
    
    // Test negative values
    String json4 = createSensorJson(-10.5f, -20.3f, -5.0f);
    TEST_ASSERT_FALSE(json4.isEmpty());
    
    // Verify JSON structure for one of them
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json1);
    
    TEST_ASSERT_EQUAL_INT(DeserializationError::Ok, error.code());
    TEST_ASSERT_TRUE(!doc["humidity"].isNull());
    TEST_ASSERT_TRUE(!doc["temperature"].isNull());
    TEST_ASSERT_TRUE(!doc["pressure"].isNull());
    TEST_ASSERT_TRUE(!doc["timestamp"].isNull());
    TEST_ASSERT_TRUE(!doc["device_id"].isNull());
    TEST_ASSERT_TRUE(!doc["wifi_rssi"].isNull());

    // Or even better, combine with type checking:
    TEST_ASSERT_TRUE(doc["humidity"].is<float>());
    TEST_ASSERT_TRUE(doc["temperature"].is<float>());
    TEST_ASSERT_TRUE(doc["pressure"].is<float>());
    TEST_ASSERT_TRUE(doc["timestamp"].is<unsigned long>());
    TEST_ASSERT_TRUE(doc["device_id"].is<const char*>());
    TEST_ASSERT_TRUE(doc["wifi_rssi"].is<int>());
    
    // Verify values
    TEST_ASSERT_EQUAL_FLOAT(45.6f, doc["humidity"]);
    TEST_ASSERT_EQUAL_FLOAT(23.4f, doc["temperature"]);
    TEST_ASSERT_EQUAL_FLOAT(1013.25f, doc["pressure"]);
    
    Serial.printf("Sample JSON: %s\n", json1.c_str());
}

// Test configuration functions without network
void test_configuration_offline() {
    Serial.println("Testing configuration functions offline...");
    
    // Test WiFi setup (won't actually connect)
    wifiSetup("TestSSID", "TestPassword", "http://test.example.com/api");
    
    // Should be in disconnected state initially
    WiFiStatus status = getWiFiStatus();
    TEST_ASSERT_EQUAL(WiFiStatus::DISCONNECTED, status);
    
    String statusStr = getWiFiStatusString();
    TEST_ASSERT_EQUAL_STRING("DISCONNECTED", statusStr.c_str());
    
    // Test configuration setters
    setRetryInterval(5000);
    setMaxRetries(3);
    setServerUrl("http://new-server.com/api");
    
    // Test error handling
    String error = getLastError();
    // Initially should be empty
    TEST_ASSERT_TRUE(error.isEmpty());
    
    Serial.println("All offline configuration tests passed");
}

// Test JSON parsing and validation
void test_json_validation() {
    Serial.println("Testing JSON validation...");
    
    // Create a test JSON
    String json = createSensorJson(42.5f, 21.3f, 1015.7f);
    
    // Parse it back
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    TEST_ASSERT_EQUAL_INT(DeserializationError::Ok, error.code());
    
    // Validate all expected fields exist
    const char* expectedFields[] = {
        "timestamp", "device_id", "humidity", 
        "temperature", "pressure", "wifi_rssi"
    };
    
    int numFields = sizeof(expectedFields) / sizeof(expectedFields[0]);
    
    for (int i = 0; i < numFields; i++) {
        TEST_ASSERT_TRUE(!doc[expectedFields[i]].isNull());
        Serial.printf("✓ Field '%s' present\n", expectedFields[i]);
    }
    
    // Validate data ranges (reasonable sensor values)
    float humidity = doc["humidity"];
    float temperature = doc["temperature"];
    float pressure = doc["pressure"];
    
    TEST_ASSERT_EQUAL_FLOAT(42.5f, humidity);
    TEST_ASSERT_EQUAL_FLOAT(21.3f, temperature);
    TEST_ASSERT_EQUAL_FLOAT(1015.7f, pressure);
    
    // Validate timestamp is reasonable (should be recent)
    unsigned long timestamp = doc["timestamp"];
    unsigned long currentTime = millis();
    TEST_ASSERT_TRUE(timestamp <= currentTime);
    TEST_ASSERT_TRUE(currentTime - timestamp < 1000); // Should be within last second
    
    Serial.println("JSON validation completed successfully");
}

// Test WiFi status transitions (without actual WiFi)
void test_status_transitions_offline() {
    Serial.println("Testing WiFi status transitions offline...");
    
    // Start fresh
    wifiSetup("OfflineTest", "password", "http://example.com");
    
    // Should start as DISCONNECTED
    TEST_ASSERT_EQUAL(WiFiStatus::DISCONNECTED, getWiFiStatus());
    
    // Test status string conversion
    TEST_ASSERT_EQUAL_STRING("DISCONNECTED", getWiFiStatusString().c_str());
    
    // Since we can't actually connect, just verify the status functions work
    TEST_ASSERT_FALSE(wifiConnected());
    
    Serial.println("Status transition tests completed");
}

// Test edge cases and error conditions
void test_edge_cases_offline() {
    Serial.println("Testing edge cases offline...");
    
    // Test with empty/null-like values
    String json1 = createSensorJson(0.0f, 0.0f, 0.0f);
    TEST_ASSERT_FALSE(json1.isEmpty());
    
    // Test with extreme values
    String json2 = createSensorJson(999.99f, -273.15f, 2000.0f);
    TEST_ASSERT_FALSE(json2.isEmpty());
    
    // Validate both JSONs parse correctly
    JsonDocument doc1, doc2;
    TEST_ASSERT_EQUAL((int)DeserializationError::Ok,
                      (int)deserializeJson(doc1, json1).code());
    TEST_ASSERT_EQUAL((int)DeserializationError::Ok,
                      (int)deserializeJson(doc2, json2).code());
    
    // Test configuration with extreme values
    setRetryInterval(1); // Very short
    setRetryInterval(60000); // Very long
    setMaxRetries(0); // No retries
    setMaxRetries(100); // Many retries
    
    // These should not crash
    TEST_ASSERT_TRUE(true);
    
    Serial.println("Edge case tests completed");
}

// Test memory usage and performance
void test_memory_performance_offline() {
    Serial.println("Testing memory and performance offline...");
    
    unsigned long startTime = millis();
    
    // Create multiple JSON objects rapidly
    for (int i = 0; i < 100; i++) {
        String json = createSensorJson(
            i * 0.5f,           // humidity
            20.0f + i * 0.1f,   // temperature  
            1000.0f + i         // pressure
        );
        
        // Verify each JSON is valid
        TEST_ASSERT_FALSE(json.isEmpty());
        
        // Parse every 10th one to verify structure
        if (i % 10 == 0) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, json);
            TEST_ASSERT_EQUAL_INT(DeserializationError::Ok, error.code());
        }
    }
    
    unsigned long elapsed = millis() - startTime;
    Serial.printf("Created 100 JSON objects in %lu ms\n", elapsed);
    
    // Should be reasonably fast (less than 1 second for 100 objects)
    TEST_ASSERT_TRUE(elapsed < 1000);
    
    Serial.println("Memory and performance tests completed");
}

// Test function parameter validation
void test_parameter_validation_offline() {
    Serial.println("Testing parameter validation offline...");
    
    // Test WiFi setup with different parameter combinations
    wifiSetup("", "", ""); // Empty strings
    TEST_ASSERT_EQUAL(WiFiStatus::DISCONNECTED, getWiFiStatus());
    
    wifiSetup("ValidSSID", "ValidPassword", "http://valid.url");
    TEST_ASSERT_EQUAL(WiFiStatus::DISCONNECTED, getWiFiStatus());
    
    // Test configuration parameter ranges
    setRetryInterval(0);     // Minimum
    setRetryInterval(UINT32_MAX); // Maximum
    
    setMaxRetries(0);        // Minimum  
    setMaxRetries(UINT16_MAX); // Maximum
    
    // Test URL setting with various formats
    setServerUrl("http://example.com");
    setServerUrl("https://secure.example.com/api/v1");
    setServerUrl(""); // Empty URL
    
    // All should complete without crashing
    TEST_ASSERT_TRUE(true);
    
    Serial.println("Parameter validation tests completed");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== WiFi Communication Offline Test Suite ===");
    Serial.println("These tests run without requiring network connectivity");
    Serial.println("===============================================\n");
    
    UNITY_BEGIN();
    
    // All tests can run offline
    RUN_TEST(test_json_creation_comprehensive);
    RUN_TEST(test_configuration_offline);
    RUN_TEST(test_json_validation);
    RUN_TEST(test_status_transitions_offline);
    RUN_TEST(test_edge_cases_offline);
    RUN_TEST(test_memory_performance_offline);
    RUN_TEST(test_parameter_validation_offline);
    UNITY_END();
}

void loop() {
    // Tests run once in setup, nothing needed in loop
    delay(1000);
}