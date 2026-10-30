#pragma once

#include <Adafruit_NeoPixel.h>

// Pattern 4 — Twinkle
// Short + long warm twinkle. LED3 stays solid 4600K white.
void pattern4Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong,
                   Adafruit_NeoPixel &stripSign);
void pattern4Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong,
                  Adafruit_NeoPixel &stripSign);
