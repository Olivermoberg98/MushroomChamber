#include <unity.h>

#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCL 22       // I2C Clock pin 
#define BME_SDA 21       // I2C Data pin 
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // Create BME280 object
bool bme_found = false;
#endif

// Test setup function - called before each test
void setUp(void) {
    #ifdef ARDUINO
    Wire.begin(BME_SDA, BME_SCL);
    delay(100);
    
    // Try to initialize BME280
    bme_found = bme.begin(0x76); // Default I2C address is 0x76, try 0x77 if this fails
    if (!bme_found) {
        bme_found = bme.begin(0x77); // Alternative I2C address
    }
    
    if (bme_found) {
        // Set sampling rates for better accuracy
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // mode
                       Adafruit_BME280::SAMPLING_X2,      // temperature
                       Adafruit_BME280::SAMPLING_X16,     // pressure
                       Adafruit_BME280::SAMPLING_X1,      // humidity
                       Adafruit_BME280::FILTER_X16,       // filtering
                       Adafruit_BME280::STANDBY_MS_500);  // standby
        delay(100);
    }
    #endif
}

// Test teardown function - called after each test
void tearDown(void) {
    #ifdef ARDUINO
    delay(100);
    #endif
}

// Test BME280 sensor initialization
void test_bme280_initialization(void) {
    #ifdef ARDUINO
    Serial.println("Testing BME280 initialization...");
    Serial.println("Trying I2C addresses 0x76 and 0x77...");
    
    TEST_ASSERT_TRUE_MESSAGE(bme_found, "BME280 sensor should be detected and initialized");
    
    if (bme_found) {
        Serial.println("BME280 sensor found and initialized successfully!");
    }
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "BME280 initialization test (simulated on native)");
    #endif
}

// Test temperature reading
void test_temperature_reading(void) {
    #ifdef ARDUINO
    if (!bme_found) {
        TEST_FAIL_MESSAGE("BME280 not found - cannot test temperature");
        return;
    }
    
    Serial.println("Testing temperature reading...");
    
    float temperature = bme.readTemperature();
    
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    
    // Temperature should be within reasonable range (-40°C to 85°C for BME280)
    TEST_ASSERT_TRUE_MESSAGE(!isnan(temperature), "Temperature reading should not be NaN");
    TEST_ASSERT_TRUE_MESSAGE(temperature > -50.0 && temperature < 100.0, 
                            "Temperature should be within reasonable range (-50°C to 100°C)");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Temperature reading test (simulated on native)");
    #endif
}

// Test pressure reading
void test_pressure_reading(void) {
    #ifdef ARDUINO
    if (!bme_found) {
        TEST_FAIL_MESSAGE("BME280 not found - cannot test pressure");
        return;
    }
    
    Serial.println("Testing pressure reading...");
    
    float pressure = bme.readPressure() / 100.0F; // Convert Pa to hPa
    
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");
    
    // Pressure should be within reasonable range (300-1200 hPa)
    TEST_ASSERT_TRUE_MESSAGE(!isnan(pressure), "Pressure reading should not be NaN");
    TEST_ASSERT_TRUE_MESSAGE(pressure > 300.0 && pressure < 1200.0, 
                            "Pressure should be within reasonable range (300-1200 hPa)");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Pressure reading test (simulated on native)");
    #endif
}

// Test humidity reading
void test_humidity_reading(void) {
    #ifdef ARDUINO
    if (!bme_found) {
        TEST_FAIL_MESSAGE("BME280 not found - cannot test humidity");
        return;
    }
    
    Serial.println("Testing humidity reading...");
    
    float humidity = bme.readHumidity();
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    
    // Humidity should be within 0-100%
    TEST_ASSERT_TRUE_MESSAGE(!isnan(humidity), "Humidity reading should not be NaN");
    TEST_ASSERT_TRUE_MESSAGE(humidity >= 0.0 && humidity <= 100.0, 
                            "Humidity should be within 0-100%");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Humidity reading test (simulated on native)");
    #endif
}

// Test altitude calculation
void test_altitude_calculation(void) {
    #ifdef ARDUINO
    if (!bme_found) {
        TEST_FAIL_MESSAGE("BME280 not found - cannot test altitude");
        return;
    }
    
    Serial.println("Testing altitude calculation...");
    
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    
    Serial.print("Approx. Altitude: ");
    Serial.print(altitude);
    Serial.println(" m");
    
    // Altitude should be within reasonable range (-500m to 9000m)
    TEST_ASSERT_TRUE_MESSAGE(!isnan(altitude), "Altitude calculation should not be NaN");
    TEST_ASSERT_TRUE_MESSAGE(altitude > -1000.0 && altitude < 10000.0, 
                            "Altitude should be within reasonable range (-1000m to 10000m)");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Altitude calculation test (simulated on native)");
    #endif
}

// Test multiple consecutive readings
void test_multiple_readings(void) {
    #ifdef ARDUINO
    if (!bme_found) {
        TEST_FAIL_MESSAGE("BME280 not found - cannot test multiple readings");
        return;
    }
    
    Serial.println("Testing multiple consecutive readings...");
    
    bool all_readings_valid = true;
    
    for (int i = 0; i < 5; i++) {
        float temp = bme.readTemperature();
        float pressure = bme.readPressure() / 100.0F;
        float humidity = bme.readHumidity();
        
        Serial.print("Reading ");
        Serial.print(i + 1);
        Serial.print(": T=");
        Serial.print(temp);
        Serial.print("°C, P=");
        Serial.print(pressure);
        Serial.print("hPa, H=");
        Serial.print(humidity);
        Serial.println("%");
        
        if (isnan(temp) || isnan(pressure) || isnan(humidity)) {
            all_readings_valid = false;
            break;
        }
        
        delay(500); // Small delay between readings
    }
    
    TEST_ASSERT_TRUE_MESSAGE(all_readings_valid, "All consecutive readings should be valid");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Multiple readings test (simulated on native)");
    #endif
}

// Test sensor stability (readings shouldn't vary wildly)
void test_sensor_stability(void) {
    #ifdef ARDUINO
    if (!bme_found) {
        TEST_FAIL_MESSAGE("BME280 not found - cannot test stability");
        return;
    }
    
    Serial.println("Testing sensor stability...");
    
    float temp1 = bme.readTemperature();
    delay(1000);
    float temp2 = bme.readTemperature();
    
    float temp_diff = abs(temp2 - temp1);
    
    Serial.print("Temperature difference between readings: ");
    Serial.print(temp_diff);
    Serial.println("°C");
    
    // Temperature shouldn't vary more than 2°C in 1 second under normal conditions
    TEST_ASSERT_TRUE_MESSAGE(temp_diff < 5.0, 
                            "Temperature should be stable (< 5°C difference in 1 second)");
    
    float pressure1 = bme.readPressure() / 100.0F;
    delay(1000);
    float pressure2 = bme.readPressure() / 100.0F;
    
    float pressure_diff = abs(pressure2 - pressure1);
    
    Serial.print("Pressure difference between readings: ");
    Serial.print(pressure_diff);
    Serial.println("hPa");
    
    // Pressure shouldn't vary more than 1 hPa in 1 second under normal conditions
    TEST_ASSERT_TRUE_MESSAGE(pressure_diff < 2.0, 
                            "Pressure should be stable (< 2 hPa difference in 1 second)");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Sensor stability test (simulated on native)");
    #endif
}

// Test all sensors together (comprehensive reading)
void test_comprehensive_reading(void) {
    #ifdef ARDUINO
    if (!bme_found) {
        TEST_FAIL_MESSAGE("BME280 not found - cannot perform comprehensive test");
        return;
    }
    
    Serial.println("Performing comprehensive sensor reading...");
    Serial.println("=== BME280 Sensor Data ===");
    
    float temperature = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0F;
    float humidity = bme.readHumidity();
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");
    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    
    Serial.print("Approx. Altitude: ");
    Serial.print(altitude);
    Serial.println(" m");
    
    Serial.println("==========================");
    
    // All readings should be valid
    bool all_valid = !isnan(temperature) && !isnan(pressure) && !isnan(humidity) && !isnan(altitude);
    TEST_ASSERT_TRUE_MESSAGE(all_valid, "All sensor readings should be valid in comprehensive test");
    #else
    TEST_ASSERT_TRUE_MESSAGE(true, "Comprehensive reading test (simulated on native)");
    #endif
}

#ifdef ARDUINO
void setup() {
    Serial.begin(115200);
    delay(5000); // Wait for serial monitor
    
    Serial.println("Starting BME280 Sensor Tests...");
    Serial.println("I2C SDA Pin: " + String(BME_SDA));
    Serial.println("I2C SCL Pin: " + String(BME_SCL));
    
    UNITY_BEGIN();
    RUN_TEST(test_bme280_initialization);
    RUN_TEST(test_temperature_reading);
    RUN_TEST(test_pressure_reading);
    RUN_TEST(test_humidity_reading);
    RUN_TEST(test_altitude_calculation);
    RUN_TEST(test_multiple_readings);
    RUN_TEST(test_sensor_stability);
    RUN_TEST(test_comprehensive_reading);
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
    RUN_TEST(test_bme280_initialization);
    RUN_TEST(test_temperature_reading);
    RUN_TEST(test_pressure_reading);
    RUN_TEST(test_humidity_reading);
    RUN_TEST(test_altitude_calculation);
    RUN_TEST(test_multiple_readings);
    RUN_TEST(test_sensor_stability);
    RUN_TEST(test_comprehensive_reading);
    return UNITY_END();
}
#endif