#pragma once

#include <Adafruit_NeoPixel.h>

// Pattern 3 — Chase Fine
// Same sync landmarks as Pattern 1; 10 groups × 1 LED; 75% of Pattern 1 speed
void pattern3Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
void pattern3Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
