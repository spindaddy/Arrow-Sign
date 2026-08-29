#pragma once

#include <Adafruit_NeoPixel.h>

// Pattern 5 — Broken Sign
// Mostly on; a couple chronic slow flickers; larger groups go dead for a while
void pattern5Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
void pattern5Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong);
