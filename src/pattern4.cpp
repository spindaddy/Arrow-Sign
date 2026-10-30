#include "pattern4.h"
#include "sign_leds.h"

#include <Arduino.h>

namespace {

constexpr uint8_t MIN_LEVEL = 15;   // dimmest twinkle
constexpr uint8_t MAX_LEVEL = 110;  // brightest (overall dimmer)

constexpr uint16_t FRAME_MS = 40;

void twinkleStrip(Adafruit_NeoPixel &strip, uint8_t baseR, uint8_t baseG,
                    uint8_t baseB) {
  const int n = static_cast<int>(strip.numPixels());
  for (int i = 0; i < n; i++) {
    // Mostly stay lit; occasionally dip for a twinkle
    uint8_t level;
    if (random(100) < 18) {
      level = static_cast<uint8_t>(random(MIN_LEVEL, 55));
    } else {
      level = static_cast<uint8_t>(random(70, MAX_LEVEL + 1));
    }

    const uint8_t r = static_cast<uint8_t>((baseR * level) / 255);
    const uint8_t g = static_cast<uint8_t>((baseG * level) / 255);
    const uint8_t b = static_cast<uint8_t>((baseB * level) / 255);
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

}  // namespace

void pattern4Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong,
                   Adafruit_NeoPixel &stripSign) {
  randomSeed(analogRead(A6) ^ millis());
  twinkleStrip(stripShort, 255, 210, 60);
  twinkleStrip(stripLong, 255, 210, 60);
  fillSignWarmWhite(stripSign);  // LED3: 4600K, or warm white if DIP2 is on
  Serial.println("  Twinkle: short+long twinkle; sign solid 4600K white");
}

void pattern4Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong,
                  Adafruit_NeoPixel &stripSign) {
  (void)stripSign;  // LED3 stays solid 4600K white (set in setup)
  twinkleStrip(stripShort, 255, 210, 60);
  twinkleStrip(stripLong, 255, 210, 60);
  delay(FRAME_MS);
}
