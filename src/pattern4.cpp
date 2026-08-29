#include "pattern4.h"

#include <Arduino.h>

namespace {

// Warm yellow-white base color
constexpr uint8_t BASE_R = 255;
constexpr uint8_t BASE_G = 210;
constexpr uint8_t BASE_B = 60;

constexpr uint8_t MIN_LEVEL = 15;   // dimmest twinkle
constexpr uint8_t MAX_LEVEL = 110;  // brightest (overall dimmer)

constexpr uint16_t FRAME_MS = 40;

void twinkleStrip(Adafruit_NeoPixel &strip) {
  const int n = static_cast<int>(strip.numPixels());
  for (int i = 0; i < n; i++) {
    // Mostly stay lit; occasionally dip for a twinkle
    uint8_t level;
    if (random(100) < 18) {
      level = static_cast<uint8_t>(random(MIN_LEVEL, 55));
    } else {
      level = static_cast<uint8_t>(random(70, MAX_LEVEL + 1));
    }

    const uint8_t r = static_cast<uint8_t>((BASE_R * level) / 255);
    const uint8_t g = static_cast<uint8_t>((BASE_G * level) / 255);
    const uint8_t b = static_cast<uint8_t>((BASE_B * level) / 255);
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

}  // namespace

void pattern4Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong) {
  randomSeed(esp_random());
  twinkleStrip(stripShort);
  twinkleStrip(stripLong);
  Serial.println("  Twinkle: all on, soft random twinkle");
}

void pattern4Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong) {
  twinkleStrip(stripShort);
  twinkleStrip(stripLong);
  delay(FRAME_MS);
}
