#pragma once

#include <Adafruit_NeoPixel.h>

// Pattern 1 — Chase Fast
// 3 groups × 2 LEDs; sync short7↔long11 and short9↔long20; short 23 mirrors 22
void pattern1Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
void pattern1Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
