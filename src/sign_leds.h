#pragma once

#include <Adafruit_NeoPixel.h>

// LED3 is one solid color, or off. The strand is long enough that a per-pixel
// buffer does not fit in the Nano's 2KB RAM, so the color is stored once and
// clocked out SIGN_LED_COUNT times.
constexpr int SIGN_LED_COUNT = 200;

// DIP2 OFF = 4600K. DIP2 ON = 2700K warm white.
constexpr uint8_t SIGN_4600_R = 255;
constexpr uint8_t SIGN_4600_G = 220;
constexpr uint8_t SIGN_4600_B = 191;
constexpr uint8_t SIGN_WARM_R = 255;
constexpr uint8_t SIGN_WARM_G = 167;
constexpr uint8_t SIGN_WARM_B = 87;

extern bool signWarmWhite;

void showSignSolid(Adafruit_NeoPixel &stripSign, uint16_t count, uint8_t r,
                   uint8_t g, uint8_t b);

inline void fillSignWarmWhite(Adafruit_NeoPixel &stripSign) {
  if (signWarmWhite) {
    showSignSolid(stripSign, SIGN_LED_COUNT, SIGN_WARM_R, SIGN_WARM_G, SIGN_WARM_B);
  } else {
    showSignSolid(stripSign, SIGN_LED_COUNT, SIGN_4600_R, SIGN_4600_G, SIGN_4600_B);
  }
}

inline void clearSign(Adafruit_NeoPixel &stripSign) {
  showSignSolid(stripSign, SIGN_LED_COUNT, 0, 0, 0);
}
