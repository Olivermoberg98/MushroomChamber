#include <unity.h>
#include <Arduino.h>

// Pin definition
#define HUMIDIFIER_PIN 15  // D15 on ESP32

void setUp(void) {
    // Set up runs before each test
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    digitalWrite(HUMIDIFIER_PIN, LOW);  // Ensure humidifier starts OFF
    delay(100);  // Small delay to ensure stable state
}

void tearDown(void) {
    // Tear down runs after each test
    digitalWrite(HUMIDIFIER_PIN, LOW);  // Always turn OFF after test
}

void test_humidifier_pin_initialization(void) {
    // Test that pin can be set as output
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    TEST_ASSERT_TRUE(true);  // If we reach here, pinMode worked
}

void test_humidifier_turn_on(void) {
    // Test turning humidifier ON
    digitalWrite(HUMIDIFIER_PIN, HIGH);
    delay(2000);  // Small delay for pin to settle
    
    // Read back the pin state
    int pinState = digitalRead(HUMIDIFIER_PIN);
    TEST_ASSERT_EQUAL(HIGH, pinState);
}

void test_humidifier_turn_off(void) {
    // First turn it on
    digitalWrite(HUMIDIFIER_PIN, HIGH);
    delay(10);
    
    // Then turn it off
    digitalWrite(HUMIDIFIER_PIN, LOW);
    delay(10);  // Small delay for pin to settle
    
    // Read back the pin state
    int pinState = digitalRead(HUMIDIFIER_PIN);
    TEST_ASSERT_EQUAL(LOW, pinState);
}

void test_humidifier_toggle_sequence(void) {
    // Test a sequence of on/off operations
    
    // Start OFF
    digitalWrite(HUMIDIFIER_PIN, LOW);
    delay(10);
    TEST_ASSERT_EQUAL(LOW, digitalRead(HUMIDIFIER_PIN));
    
    // Turn ON
    digitalWrite(HUMIDIFIER_PIN, HIGH);
    delay(10);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(HUMIDIFIER_PIN));
    
    // Turn OFF again
    digitalWrite(HUMIDIFIER_PIN, LOW);
    delay(10);
    TEST_ASSERT_EQUAL(LOW, digitalRead(HUMIDIFIER_PIN));
    
    // Turn ON again
    digitalWrite(HUMIDIFIER_PIN, HIGH);
    delay(10);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(HUMIDIFIER_PIN));
}

void test_humidifier_pwm_basic(void) {
    // Test basic PWM functionality (if you want to add intensity control later)
    // Set PWM frequency and resolution
    ledcSetup(0, 1000, 8);  // Channel 0, 1kHz, 8-bit resolution
    ledcAttachPin(HUMIDIFIER_PIN, 0);
    
    // Test 50% duty cycle
    ledcWrite(0, 128);  // 128 = 50% of 255
    delay(100);
    
    // Test full power
    ledcWrite(0, 255);  // 100% duty cycle
    delay(100);
    
    // Test off
    ledcWrite(0, 0);    // 0% duty cycle
    delay(100);
    
    // Clean up - detach PWM and return to digital mode
    ledcDetachPin(HUMIDIFIER_PIN);
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    digitalWrite(HUMIDIFIER_PIN, LOW);
    
    TEST_ASSERT_TRUE(true);  // If we reach here, PWM operations completed
}

void setup() {
    delay(2000);  // Give time for serial monitor to connect
    
    UNITY_BEGIN();
    
    // Run the tests
    RUN_TEST(test_humidifier_pin_initialization);
    RUN_TEST(test_humidifier_turn_on);
    RUN_TEST(test_humidifier_turn_off);
    RUN_TEST(test_humidifier_toggle_sequence);
    RUN_TEST(test_humidifier_pwm_basic);
    
    UNITY_END();
}

void loop() {
    // Empty loop - tests run once in setup()
    delay(1000);  // Just to keep the loop running
}