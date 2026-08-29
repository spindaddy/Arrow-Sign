#include "pattern3.h"

#include <Arduino.h>
#include <math.h>

namespace {

constexpr int HEAD_LEDS = 1;     // 1 LED per group
constexpr int NUM_GROUPS = 10;   // 10 chasing groups

// Landmark pairs (1-based → 0-based): short7↔long11, short9↔long20
constexpr int SHORT_SYNC_A = 7 - 1;
constexpr int LONG_SYNC_A = 11 - 1;
constexpr int SHORT_SYNC_B = 9 - 1;
constexpr int LONG_SYNC_B = 20 - 1;

constexpr int SHORT_LED_22 = 22;
constexpr int SHORT_LED_23 = 23;

// 75% of Pattern 1 speed → longer sync time
constexpr uint32_t SYNC_B_MS = 889;
constexpr float P_A =
    static_cast<float>(SHORT_SYNC_A) / static_cast<float>(24 - 1);
constexpr float P_B =
    static_cast<float>(SHORT_SYNC_B) / static_cast<float>(24 - 1);
constexpr uint32_t CYCLE_MS =
    static_cast<uint32_t>(static_cast<float>(SYNC_B_MS) / P_B + 0.5f);

uint32_t chaseStartMs = 0;

float wrap01(float p) {
  if (p > 1.0f) {
    p = fmodf(p, 1.0f);
    if (p <= 0.0f) {
      return 1.0f;
    }
  }
  while (p < 0.0f) {
    p += 1.0f;
  }
  if (p <= 0.0f) {
    return 0.0f;
  }
  return p;
}

float pToHead(float p, int syncA, int syncB, int last) {
  p = wrap01(p);

  if (p >= 1.0f) {
    return static_cast<float>(last);
  }
  if (p <= P_A) {
    if (P_A <= 0.0f) {
      return 0.0f;
    }
    return (p / P_A) * static_cast<float>(syncA);
  }
  if (p <= P_B) {
    if (P_B <= P_A) {
      return static_cast<float>(syncB);
    }
    return static_cast<float>(syncA) +
           ((p - P_A) / (P_B - P_A)) * static_cast<float>(syncB - syncA);
  }
  if (last <= syncB) {
    return static_cast<float>(syncB);
  }
  return static_cast<float>(syncB) +
         ((p - P_B) / (1.0f - P_B)) * static_cast<float>(last - syncB);
}

void patternStep(Adafruit_NeoPixel &strip, float p, int syncA, int syncB,
                 int last, bool mirror22to23) {
  const int n = static_cast<int>(strip.numPixels());
  strip.clear();

  for (int g = 0; g < NUM_GROUPS; g++) {
    const float gp = p + static_cast<float>(g) / static_cast<float>(NUM_GROUPS);
    float head = pToHead(gp, syncA, syncB, last);
    if (head > static_cast<float>(last)) {
      head = static_cast<float>(last);
    }

    for (int i = 0; i <= last && i < n; i++) {
      const float behind = head - static_cast<float>(i);
      if (behind >= 0.0f && behind < static_cast<float>(HEAD_LEDS)) {
        strip.setPixelColor(i, strip.Color(255, 210, 60));
      }
    }
  }

  if (mirror22to23 && SHORT_LED_22 < n && SHORT_LED_23 < n) {
    const uint32_t c22 = strip.getPixelColor(SHORT_LED_22);
    if (c22 != 0) {
      strip.setPixelColor(SHORT_LED_23, c22);
    }
  }

  strip.show();
}

}  // namespace

void pattern3Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong) {
  (void)stripShort;
  (void)stripLong;
  chaseStartMs = millis();
  Serial.println("  Chase Fine: 10×1 LED; sync 7/11 + 9/20; 75% speed");
}

void pattern3Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong) {
  const uint32_t t = (millis() - chaseStartMs) % CYCLE_MS;
  const float p = (CYCLE_MS <= 1)
                      ? 0.0f
                      : static_cast<float>(t) / static_cast<float>(CYCLE_MS - 1);

  const int shortLast = static_cast<int>(stripShort.numPixels()) - 1;
  const int longLast = static_cast<int>(stripLong.numPixels()) - 1;

  patternStep(stripShort, p, SHORT_SYNC_A, SHORT_SYNC_B, shortLast, true);
  patternStep(stripLong, p, LONG_SYNC_A, LONG_SYNC_B, longLast, false);

  delay(5);
}
