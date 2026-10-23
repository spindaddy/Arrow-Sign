#pragma once

#include <Adafruit_NeoPixel.h>

// Shared warm white for LED3 (sign strip) — matches arrow pattern base color
constexpr uint8_t SIGN_WARM_R = 255;
constexpr uint8_t SIGN_WARM_G = 210;
constexpr uint8_t SIGN_WARM_B = 60;

inline void fillSignWarmWhite(Adafruit_NeoPixel &stripSign) {
  const uint32_t c = stripSign.Color(SIGN_WARM_R, SIGN_WARM_G, SIGN_WARM_B);
  const int n = static_cast<int>(stripSign.numPixels());
  for (int i = 0; i < n; i++) {
    stripSign.setPixelColor(i, c);
  }
  stripSign.show();
}

inline void clearSign(Adafruit_NeoPixel &stripSign) {
  stripSign.clear();
  stripSign.show();
}
