#pragma once

#include <Adafruit_NeoPixel.h>

// Pattern 5 — Broken Sign
// Short+long: mostly on, chronic flickers, dead groups
// LED3 (sign): whole-strip random on/off flicker (4600K white when on)
void pattern5Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong,
                   Adafruit_NeoPixel &stripSign);
void pattern5Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong,
                  Adafruit_NeoPixel &stripSign);

// Whole-strip sign lamp: true while the 200 LEDs are 4600K white.
bool pattern5SignIsOn();
