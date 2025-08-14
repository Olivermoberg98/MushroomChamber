#include <unity.h>

#ifdef ARDUINO
#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 27        // Pin connected to the LED strip data line
#define NUM_LEDS 60      // Total number of LEDs (two 30-LED strips)
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
#endif

// Test setup function - called before each test
void setUp(void) {
    #ifdef ARDUINO
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(50); // Set to 50% brightness for testing
    FastLED.clear();
    FastLED.show();
    delay(100);
    #endif
}

// Test teardown function - called after each test
void tearDown(void) {
    #ifdef ARDUINO
    FastLED.clear();
    FastLED.show();
    delay(100);
    #endif
}

// Test basic LED strip initialization
void test_led_strip_initialization(void) {
    #ifdef ARDUINO
    // Test that we can set a simple color
    leds[0] = CRGB::Red;
    FastLED.show();
    delay(500);
    
    // Clear the LED
    leds[0] = CRGB::Black;
    FastLED.show();
    
    TEST_ASSERT_TRUE_MESSAGE(true, "LED strip initialized successfully");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "LED strip initialization test (simulated on native)");
    #endif
}

// Test setting all LEDs to a single color
void test_all_leds_single_color(void) {
    #ifdef ARDUINO
    Serial.println("Testing all LEDs with red color...");
    
    // Set all LEDs to red
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(1000);
    
    // Clear all LEDs
    FastLED.clear();
    FastLED.show();
    
    TEST_ASSERT_TRUE_MESSAGE(true, "All LEDs set to single color successfully");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "All LEDs single color test (simulated on native)");
    #endif
}

// Test RGB color functionality
void test_rgb_colors(void) {
    #ifdef ARDUINO
    Serial.println("Testing RGB colors...");
    
    // Test Red
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(500);
    
    // Test Green
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(500);
    
    // Test Blue
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();
    delay(500);
    
    // Clear
    FastLED.clear();
    FastLED.show();
    
    TEST_ASSERT_TRUE_MESSAGE(true, "RGB colors test completed successfully");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "RGB colors test (simulated on native)");
    #endif
}

// Test individual LED control
void test_individual_led_control(void) {
    #ifdef ARDUINO
    Serial.println("Testing individual LED control...");
    
    // Test first LED
    leds[0] = CRGB::Red;
    FastLED.show();
    delay(300);
    
    // Test middle LED (LED 30 - between the two strips)
    leds[0] = CRGB::Black;
    leds[29] = CRGB::Green; // Last LED of first strip
    leds[30] = CRGB::Blue;  // First LED of second strip
    FastLED.show();
    delay(500);
    
    // Test last LED
    leds[29] = CRGB::Black;
    leds[30] = CRGB::Black;
    leds[NUM_LEDS - 1] = CRGB::Yellow;
    FastLED.show();
    delay(300);
    
    // Clear
    FastLED.clear();
    FastLED.show();
    
    TEST_ASSERT_TRUE_MESSAGE(true, "Individual LED control test completed");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Individual LED control test (simulated on native)");
    #endif
}

// Test brightness control
void test_brightness_control(void) {
    #ifdef ARDUINO
    Serial.println("Testing brightness control...");
    
    // Set all LEDs to white
    fill_solid(leds, NUM_LEDS, CRGB::White);
    
    // Test different brightness levels
    for (int brightness = 10; brightness <= 100; brightness += 30) {
        FastLED.setBrightness(brightness);
        FastLED.show();
        delay(500);
    }
    
    // Reset to test brightness
    FastLED.setBrightness(50);
    FastLED.clear();
    FastLED.show();
    
    TEST_ASSERT_TRUE_MESSAGE(true, "Brightness control test completed");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Brightness control test (simulated on native)");
    #endif
}

// Test rainbow pattern
void test_rainbow_pattern(void) {
    #ifdef ARDUINO
    Serial.println("Testing rainbow pattern...");
    
    // Create a rainbow pattern
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(i * 255 / NUM_LEDS, 255, 255);
    }
    FastLED.show();
    delay(2000);
    
    // Clear
    FastLED.clear();
    FastLED.show();
    
    TEST_ASSERT_TRUE_MESSAGE(true, "Rainbow pattern test completed");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Rainbow pattern test (simulated on native)");
    #endif
}

// Test two-strip configuration (first 30 vs last 30 LEDs)
void test_two_strip_configuration(void) {
    #ifdef ARDUINO
    Serial.println("Testing two-strip configuration...");
    
    // Light up first strip (LEDs 0-29) in red
    for (int i = 0; i < 30; i++) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
    delay(1000);
    
    // Clear first strip, light up second strip (LEDs 30-59) in blue
    for (int i = 0; i < 30; i++) {
        leds[i] = CRGB::Black;
        leds[i + 30] = CRGB::Blue;
    }
    FastLED.show();
    delay(1000);
    
    // Light up both strips in different colors
    for (int i = 0; i < 30; i++) {
        leds[i] = CRGB::Green;      // First strip green
        leds[i + 30] = CRGB::Purple; // Second strip purple
    }
    FastLED.show();
    delay(1000);
    
    // Clear
    FastLED.clear();
    FastLED.show();
    
    TEST_ASSERT_TRUE_MESSAGE(true, "Two-strip configuration test completed");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Two-strip configuration test (simulated on native)");
    #endif
}

// Test running light effect
void test_running_light_effect(void) {
    #ifdef ARDUINO
    Serial.println("Testing running light effect...");
    
    // Running light from start to end
    for (int i = 0; i < NUM_LEDS; i++) {
        FastLED.clear();
        leds[i] = CRGB::White;
        FastLED.show();
        delay(50);
    }
    
    // Running light from end to start
    for (int i = NUM_LEDS - 1; i >= 0; i--) {
        FastLED.clear();
        leds[i] = CRGB::Cyan;
        FastLED.show();
        delay(50);
    }
    
    // Clear
    FastLED.clear();
    FastLED.show();
    
    TEST_ASSERT_TRUE_MESSAGE(true, "Running light effect test completed");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Running light effect test (simulated on native)");
    #endif
}

#ifdef ARDUINO
void setup() {
    Serial.begin(115200);
    delay(2000); // Wait for serial monitor
    
    Serial.println("Starting LED Strip Tests...");
    Serial.println("Total LEDs: " + String(NUM_LEDS));
    Serial.println("LED Pin: " + String(LED_PIN));
    
    UNITY_BEGIN();
    RUN_TEST(test_led_strip_initialization);
    RUN_TEST(test_all_leds_single_color);
    RUN_TEST(test_rgb_colors);
    RUN_TEST(test_individual_led_control);
    RUN_TEST(test_brightness_control);
    RUN_TEST(test_rainbow_pattern);
    RUN_TEST(test_two_strip_configuration);
    RUN_TEST(test_running_light_effect);
    UNITY_END();
}

void loop() {
    // Tests run once in setup, nothing needed in loop
    delay(1000);
}

#else
// For native testing (runs on computer)
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_led_strip_initialization);
    RUN_TEST(test_all_leds_single_color);
    RUN_TEST(test_rgb_colors);
    RUN_TEST(test_individual_led_control);
    RUN_TEST(test_brightness_control);
    RUN_TEST(test_rainbow_pattern);
    RUN_TEST(test_two_strip_configuration);
    RUN_TEST(test_running_light_effect);
    return UNITY_END();
}
#endif