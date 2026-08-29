#pragma once

#include <Adafruit_NeoPixel.h>

// Pattern 2 — Chase Half
// Same chase as Pattern 1 (3 groups × 2 LEDs), at 50% speed
void pattern2Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
void pattern2Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
