#pragma once

#include <Adafruit_NeoPixel.h>

// Pattern 4 — Twinkle
// All LEDs on warm white with soft random brightness twinkle
void pattern4Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
void pattern4Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
