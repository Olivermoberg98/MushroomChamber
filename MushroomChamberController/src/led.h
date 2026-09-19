#ifndef LED_H
#define LED_H

#include <FastLED.h>

// Constants
#define LED_PIN     27
#define NUM_LEDS    60

// FastLED scales brightness to hold the strip under this; raise only if the
// strip has its own supply rather than sharing the 5 V rail
#define LED_MAX_MILLIAMPS 1200

// Functions
void setupLeds();
void controlLighting(const PhaseConfig& config);
void setLEDColor(CRGB color);

#endif 
