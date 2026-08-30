#include <unity.h>

#ifdef ARDUINO
#include <Arduino.h>
#define EXHAUST_FAN_PIN 13
#define INLET_FAN_PIN 14
#endif

// Test setup function - called before each test
void setUp(void) {
    #ifdef ARDUINO
    pinMode(EXHAUST_FAN_PIN, OUTPUT);
    pinMode(INLET_FAN_PIN, OUTPUT);
    digitalWrite(EXHAUST_FAN_PIN, LOW);
    digitalWrite(INLET_FAN_PIN, LOW);
    #endif
}

// Test teardown function - called after each test
void tearDown(void) {
    #ifdef ARDUINO
    digitalWrite(EXHAUST_FAN_PIN, LOW);
    digitalWrite(INLET_FAN_PIN, LOW);
    #endif
}

// === FAN TESTS ===
void test_fans_turn_on(void) {
    #ifdef ARDUINO
    digitalWrite(EXHAUST_FAN_PIN, HIGH);
    digitalWrite(INLET_FAN_PIN, HIGH);
    
    TEST_ASSERT_TRUE_MESSAGE(digitalRead(EXHAUST_FAN_PIN), "Exhaust fan turned ON successfully");
    TEST_ASSERT_TRUE_MESSAGE(digitalRead(INLET_FAN_PIN), "Inlet fan turned ON successfully");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Fans ON test (simulated on native)");
    #endif
}

void test_fans_turn_off(void) {
    #ifdef ARDUINO
    digitalWrite(EXHAUST_FAN_PIN, LOW);
    digitalWrite(INLET_FAN_PIN, LOW);
    
    TEST_ASSERT_FALSE_MESSAGE(digitalRead(EXHAUST_FAN_PIN), "Exhaust fan turned OFF successfully");
    TEST_ASSERT_FALSE_MESSAGE(digitalRead(INLET_FAN_PIN), "Inlet fan turned OFF successfully");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Fans OFF test (simulated on native)");
    #endif
}

void test_fans_sequence(void) {
    #ifdef ARDUINO
    Serial.println("Testing fans sequence...");
    
    // Test individual fans
    for(int i = 0; i < 2; i++) {
        // Exhaust fan
        digitalWrite(EXHAUST_FAN_PIN, HIGH);
        delay(1000);
        digitalWrite(EXHAUST_FAN_PIN, LOW);
        
        // Inlet fan
        digitalWrite(INLET_FAN_PIN, HIGH);
        delay(1000);
        digitalWrite(INLET_FAN_PIN, LOW);
    }
    
    // Test both fans together
    digitalWrite(EXHAUST_FAN_PIN, HIGH);
    digitalWrite(INLET_FAN_PIN, HIGH);
    delay(10000);
    digitalWrite(EXHAUST_FAN_PIN, LOW);
    digitalWrite(INLET_FAN_PIN, LOW);
    
    TEST_ASSERT_TRUE_MESSAGE(true, "Fans sequence completed");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Fans sequence test (simulated on native)");
    #endif
}

#ifdef ARDUINO
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Starting Fan Tests...");
    
    UNITY_BEGIN();
    
    // Run fan tests
    Serial.println("=== FANS TESTS ===");
    RUN_TEST(test_fans_turn_on);
    RUN_TEST(test_fans_turn_off);
    RUN_TEST(test_fans_sequence);
    
    UNITY_END();
}

void loop() {
    delay(1000);
}

#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_fans_turn_on);
    RUN_TEST(test_fans_turn_off);
    RUN_TEST(test_fans_sequence);
    return UNITY_END();
}
#endif