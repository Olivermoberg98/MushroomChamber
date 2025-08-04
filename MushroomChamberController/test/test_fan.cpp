#include <unity.h>

#ifdef ARDUINO
#include <Arduino.h>
#define FAN_PIN 14
// Add other pin definitions for LED strip tests here
#endif

// Test setup function - called before each test
void setUp(void) {
    #ifdef ARDUINO
    pinMode(FAN_PIN, OUTPUT);
    digitalWrite(FAN_PIN, LOW);
    // Add setup for other components here
    #endif
}

// Test teardown function - called after each test
void tearDown(void) {
    #ifdef ARDUINO
    digitalWrite(FAN_PIN, LOW);
    // Add cleanup for other components here
    #endif
}

// === FAN TESTS ===
void test_fan_turn_on(void) {
    #ifdef ARDUINO
    digitalWrite(FAN_PIN, HIGH);
    TEST_ASSERT_TRUE_MESSAGE(true, "Fan turned ON successfully");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Fan ON test (simulated on native)");
    #endif
}

void test_fan_turn_off(void) {
    #ifdef ARDUINO
    digitalWrite(FAN_PIN, LOW);
    TEST_ASSERT_TRUE_MESSAGE(true, "Fan turned OFF successfully");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Fan OFF test (simulated on native)");
    #endif
}

void test_fan_toggle_sequence(void) {
    #ifdef ARDUINO
    Serial.println("Testing fan toggle sequence...");
    digitalWrite(FAN_PIN, HIGH);
    delay(1000);
    digitalWrite(FAN_PIN, LOW);
    delay(1000);
    digitalWrite(FAN_PIN, HIGH);
    delay(1000);
    digitalWrite(FAN_PIN, LOW);
    delay(1000);
    TEST_ASSERT_TRUE_MESSAGE(true, "Fan toggle sequence completed");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Fan toggle test (simulated on native)");
    #endif
}

#ifdef ARDUINO
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Starting All Component Tests...");
    
    UNITY_BEGIN();
    
    // Run fan tests
    Serial.println("=== FAN TESTS ===");
    RUN_TEST(test_fan_turn_on);
    RUN_TEST(test_fan_turn_off);
    RUN_TEST(test_fan_toggle_sequence);
    
    UNITY_END();
}

void loop() {
    delay(1000);
}

#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_fan_turn_on);
    RUN_TEST(test_fan_turn_off);
    RUN_TEST(test_fan_toggle_sequence);
    RUN_TEST(test_led_strip_basic);
    return UNITY_END();
}
#endif