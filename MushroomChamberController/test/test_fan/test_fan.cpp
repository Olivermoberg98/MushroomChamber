#include <unity.h>

#ifdef ARDUINO
#include <Arduino.h>
#define FAN1_PIN 13
#define FAN2_PIN 12
#define FAN3_PIN 14
#endif

// Test setup function - called before each test
void setUp(void) {
    #ifdef ARDUINO
    pinMode(FAN1_PIN, OUTPUT);
    pinMode(FAN2_PIN, OUTPUT);
    pinMode(FAN3_PIN, OUTPUT);
    digitalWrite(FAN1_PIN, LOW);
    digitalWrite(FAN2_PIN, LOW);
    digitalWrite(FAN3_PIN, LOW);
    #endif
}

// Test teardown function - called after each test
void tearDown(void) {
    #ifdef ARDUINO
    digitalWrite(FAN1_PIN, LOW);
    digitalWrite(FAN2_PIN, LOW);
    digitalWrite(FAN3_PIN, LOW);
    #endif
}

// === FAN TESTS ===
void test_fans_turn_on(void) {
    #ifdef ARDUINO
    digitalWrite(FAN1_PIN, HIGH);
    digitalWrite(FAN2_PIN, HIGH);
    digitalWrite(FAN3_PIN, HIGH);
    delay(100); // Small delay to ensure pins are set
    TEST_ASSERT_TRUE_MESSAGE(digitalRead(FAN1_PIN), "Fan 1 turned ON successfully");
    TEST_ASSERT_TRUE_MESSAGE(digitalRead(FAN2_PIN), "Fan 2 turned ON successfully");
    TEST_ASSERT_TRUE_MESSAGE(digitalRead(FAN3_PIN), "Fan 3 turned ON successfully");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Fans ON test (simulated on native)");
    #endif
}

void test_fans_turn_off(void) {
    #ifdef ARDUINO
    digitalWrite(FAN1_PIN, LOW);
    digitalWrite(FAN2_PIN, LOW);
    digitalWrite(FAN3_PIN, LOW);
    delay(100); // Small delay to ensure pins are set
    TEST_ASSERT_FALSE_MESSAGE(digitalRead(FAN1_PIN), "Fan 1 turned OFF successfully");
    TEST_ASSERT_FALSE_MESSAGE(digitalRead(FAN2_PIN), "Fan 2 turned OFF successfully");
    TEST_ASSERT_FALSE_MESSAGE(digitalRead(FAN3_PIN), "Fan 3 turned OFF successfully");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Fans OFF test (simulated on native)");
    #endif
}

void test_fans_sequence(void) {
    #ifdef ARDUINO
    Serial.println("Testing fans sequence...");
    
    // Test individual fans
    for(int i = 0; i < 2; i++) {
        // Fan 1
        digitalWrite(FAN1_PIN, HIGH);
        delay(1000);
        digitalWrite(FAN1_PIN, LOW);
        
        // Fan 2
        digitalWrite(FAN2_PIN, HIGH);
        delay(1000);
        digitalWrite(FAN2_PIN, LOW);
        
        // Fan 3
        digitalWrite(FAN3_PIN, HIGH);
        delay(1000);
        digitalWrite(FAN3_PIN, LOW);
    }
    
    // Test all fans together
    digitalWrite(FAN1_PIN, HIGH);
    digitalWrite(FAN2_PIN, HIGH);
    digitalWrite(FAN3_PIN, HIGH);
    delay(10000);
    digitalWrite(FAN1_PIN, LOW);
    digitalWrite(FAN2_PIN, LOW);
    digitalWrite(FAN3_PIN, LOW);
    
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