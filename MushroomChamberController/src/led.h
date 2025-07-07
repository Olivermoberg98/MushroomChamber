#ifndef LED_H
#define LED_H

#include <FastLED.h>

// Constants
#define LED_PIN     5
#define NUM_LEDS    60

// Functions
void setupLeds();
void controlLighting();
void setLEDColor(CRGB color);

#endif 
