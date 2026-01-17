#ifndef LED_H
#define LED_H

#include <FastLED.h>

// Constants
#define LED_PIN     27
#define NUM_LEDS    60

// Functions
void setupLeds();
void controlLighting(const PhaseConfig& config);
void setLEDColor(CRGB color);

#endif 
