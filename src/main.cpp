#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "pattern1.h"
#include "pattern2.h"
#include "pattern3.h"
#include "pattern4.h"
#include "pattern5.h"

static constexpr int STRIP1_PIN = 4;
static constexpr int STRIP1_COUNT = 24;  // shorter

static constexpr int STRIP2_PIN = 2;
static constexpr int STRIP2_COUNT = 35;  // longer

static constexpr uint8_t BRIGHTNESS = 255;

// ---------------------------------------------------------------------------
// Pattern catalog (cycled every PATTERN_MS)
//   1  Chase Fast      — 3 groups × 2 LEDs, sync landmarks, full speed
//   2  Chase Half      — same as 1, 50% speed
//   3  Chase Fine      — 10 groups × 1 LED, same landmarks, 75% of pattern 1
//   4  Twinkle         — all LEDs on, soft random twinkle
//   5  Broken Sign     — mostly on, chronic flickers, dead groups
// ---------------------------------------------------------------------------
enum PatternId : int {
  PATTERN_CHASE_FAST = 1,
  PATTERN_CHASE_HALF = 2,
  PATTERN_CHASE_FINE = 3,
  PATTERN_TWINKLE = 4,
  PATTERN_BROKEN_SIGN = 5,
};

static constexpr int PATTERN_COUNT = 5;
static constexpr uint32_t PATTERN_MS = 60000;  // 1 minute each
static constexpr PatternId BOOT_PATTERN = PATTERN_BROKEN_SIGN;

Adafruit_NeoPixel strip1(STRIP1_COUNT, STRIP1_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip2(STRIP2_COUNT, STRIP2_PIN, NEO_RGB + NEO_KHZ800);

static int activePattern = PATTERN_CHASE_FAST;
static uint32_t patternStartedMs = 0;

const char *patternName(int pattern) {
  switch (pattern) {
    case PATTERN_CHASE_FAST:
      return "1 Chase Fast";
    case PATTERN_CHASE_HALF:
      return "2 Chase Half";
    case PATTERN_CHASE_FINE:
      return "3 Chase Fine";
    case PATTERN_TWINKLE:
      return "4 Twinkle";
    case PATTERN_BROKEN_SIGN:
      return "5 Broken Sign";
    default:
      return "Unknown";
  }
}

void startPattern(int pattern) {
  activePattern = pattern;
  patternStartedMs = millis();

  strip1.clear();
  strip2.clear();
  strip1.show();
  strip2.show();

  Serial.print("Starting pattern ");
  Serial.println(patternName(pattern));

  if (pattern == PATTERN_CHASE_FAST) {
    pattern1Setup(strip1, strip2);
  } else if (pattern == PATTERN_CHASE_HALF) {
    pattern2Setup(strip1, strip2);
  } else if (pattern == PATTERN_CHASE_FINE) {
    pattern3Setup(strip1, strip2);
  } else if (pattern == PATTERN_TWINKLE) {
    pattern4Setup(strip1, strip2);
  } else {
    pattern5Setup(strip1, strip2);
  }
}

void setup() {
  Serial.begin(115200);

  strip1.begin();
  strip1.setBrightness(BRIGHTNESS);
  strip1.clear();
  strip1.show();

  strip2.begin();
  strip2.setBrightness(BRIGHTNESS);
  strip2.clear();
  strip2.show();

  delay(300);

  startPattern(BOOT_PATTERN);
}

void loop() {
  if (millis() - patternStartedMs >= PATTERN_MS) {
    const int next = (activePattern % PATTERN_COUNT) + 1;
    startPattern(next);
  }

  if (activePattern == PATTERN_CHASE_FAST) {
    pattern1Loop(strip1, strip2);
  } else if (activePattern == PATTERN_CHASE_HALF) {
    pattern2Loop(strip1, strip2);
  } else if (activePattern == PATTERN_CHASE_FINE) {
    pattern3Loop(strip1, strip2);
  } else if (activePattern == PATTERN_TWINKLE) {
    pattern4Loop(strip1, strip2);
  } else {
    pattern5Loop(strip1, strip2);
  }
}
