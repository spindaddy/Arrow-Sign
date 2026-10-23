#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "pattern1.h"
#include "pattern2.h"
#include "pattern3.h"
#include "pattern4.h"
#include "pattern5.h"

// Addressable strip data pins (Jason / schematic: LED1–LED4 → D2–D5)
static constexpr int LED1_PIN = 2;  // short strip
static constexpr int LED2_PIN = 3;  // long strip
static constexpr int LED3_PIN = 4;  // sign strip
static constexpr int LED4_PIN = 5;  // unused for now

static constexpr int STRIP_SHORT_COUNT = 24;
static constexpr int STRIP_LONG_COUNT = 35;
static constexpr int STRIP_SIGN_COUNT = 40;

// DIP SW1 (BD04): A0–A3 to switch, other side GND → INPUT_PULLUP (ON = LOW)
static constexpr int DIP_PIN_0 = A0;  // switch 1
static constexpr int DIP_PIN_1 = A1;  // switch 2
static constexpr int DIP_PIN_2 = A2;  // switch 3
static constexpr int DIP_PIN_3 = A3;  // switch 4

static constexpr uint8_t BRIGHTNESS_FULL = 255;
static constexpr uint8_t BRIGHTNESS_DIM = 128;  // 50% when DIP switch 1 is ON

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

Adafruit_NeoPixel stripShort(STRIP_SHORT_COUNT, LED1_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel stripLong(STRIP_LONG_COUNT, LED2_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel stripSign(STRIP_SIGN_COUNT, LED3_PIN, NEO_RGB + NEO_KHZ800);

static int activePattern = PATTERN_CHASE_FAST;
static uint32_t patternStartedMs = 0;
static uint8_t appliedBrightness = 0;  // force first apply

void setupDipPins() {
  pinMode(DIP_PIN_0, INPUT_PULLUP);
  pinMode(DIP_PIN_1, INPUT_PULLUP);
  pinMode(DIP_PIN_2, INPUT_PULLUP);
  pinMode(DIP_PIN_3, INPUT_PULLUP);
  // LED4 reserved; leave as input so the pin is not driven
  pinMode(LED4_PIN, INPUT);
}

// Returns bitmask of DIP positions that are ON (switch closed → pin LOW)
uint8_t readDipMask() {
  uint8_t mask = 0;
  if (digitalRead(DIP_PIN_0) == LOW) {
    mask |= 0x01;
  }
  if (digitalRead(DIP_PIN_1) == LOW) {
    mask |= 0x02;
  }
  if (digitalRead(DIP_PIN_2) == LOW) {
    mask |= 0x04;
  }
  if (digitalRead(DIP_PIN_3) == LOW) {
    mask |= 0x08;
  }
  return mask;
}

bool dipSwitch1On() { return digitalRead(DIP_PIN_0) == LOW; }
bool dipSwitch4On() { return digitalRead(DIP_PIN_3) == LOW; }

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

bool patternAllowed(int pattern) {
  if (pattern == PATTERN_BROKEN_SIGN && dipSwitch4On()) {
    return false;
  }
  return pattern >= 1 && pattern <= PATTERN_COUNT;
}

// Next pattern in the cycle, skipping any disabled by DIP
int nextAllowedPattern(int from) {
  int candidate = from;
  for (int i = 0; i < PATTERN_COUNT; i++) {
    candidate = (candidate % PATTERN_COUNT) + 1;
    if (patternAllowed(candidate)) {
      return candidate;
    }
  }
  return PATTERN_CHASE_FAST;
}

int resolveBootPattern() {
  if (patternAllowed(BOOT_PATTERN)) {
    return BOOT_PATTERN;
  }
  return nextAllowedPattern(BOOT_PATTERN);
}

void beginStrip(Adafruit_NeoPixel &strip) {
  strip.begin();
  strip.setBrightness(BRIGHTNESS_FULL);
  strip.clear();
  strip.show();
}

// DIP switch 1 (A0): ON → all addressable strips at 50% for every pattern
void applyGlobalBrightness() {
  const uint8_t bri = dipSwitch1On() ? BRIGHTNESS_DIM : BRIGHTNESS_FULL;
  // Always set on all strips so every pattern's show() uses the current level
  stripShort.setBrightness(bri);
  stripLong.setBrightness(bri);
  stripSign.setBrightness(bri);
  if (bri != appliedBrightness) {
    appliedBrightness = bri;
    Serial.print("Brightness ");
    Serial.print(bri == BRIGHTNESS_DIM ? "50%" : "100%");
    Serial.println(bri == BRIGHTNESS_DIM ? " (DIP1 ON)" : " (DIP1 OFF)");
  }
}

void startPattern(int pattern) {
  if (!patternAllowed(pattern)) {
    pattern = nextAllowedPattern(pattern);
  }

  activePattern = pattern;
  patternStartedMs = millis();
  applyGlobalBrightness();

  stripShort.clear();
  stripLong.clear();
  stripSign.clear();
  stripShort.show();
  stripLong.show();
  stripSign.show();

  Serial.print("Starting pattern ");
  Serial.println(patternName(pattern));

  if (pattern == PATTERN_CHASE_FAST) {
    pattern1Setup(stripShort, stripLong, stripSign);
  } else if (pattern == PATTERN_CHASE_HALF) {
    pattern2Setup(stripShort, stripLong, stripSign);
  } else if (pattern == PATTERN_CHASE_FINE) {
    pattern3Setup(stripShort, stripLong, stripSign);
  } else if (pattern == PATTERN_TWINKLE) {
    pattern4Setup(stripShort, stripLong, stripSign);
  } else {
    pattern5Setup(stripShort, stripLong, stripSign);
  }
}

void setup() {
  Serial.begin(115200);
  setupDipPins();

  beginStrip(stripShort);
  beginStrip(stripLong);
  beginStrip(stripSign);

  Serial.print("DIP SW1 (ON=1): ");
  Serial.println(readDipMask(), BIN);
  if (dipSwitch4On()) {
    Serial.println("DIP4 ON: Broken Sign disabled");
  }
  applyGlobalBrightness();

  delay(300);

  startPattern(resolveBootPattern());
}

void loop() {
  // Applies to every pattern on every frame
  applyGlobalBrightness();

  // DIP4 turned on while Broken Sign is active → leave immediately
  if (!patternAllowed(activePattern)) {
    startPattern(nextAllowedPattern(activePattern));
  }

  if (millis() - patternStartedMs >= PATTERN_MS) {
    startPattern(nextAllowedPattern(activePattern));
  }

  if (activePattern == PATTERN_CHASE_FAST) {
    pattern1Loop(stripShort, stripLong, stripSign);
  } else if (activePattern == PATTERN_CHASE_HALF) {
    pattern2Loop(stripShort, stripLong, stripSign);
  } else if (activePattern == PATTERN_CHASE_FINE) {
    pattern3Loop(stripShort, stripLong, stripSign);
  } else if (activePattern == PATTERN_TWINKLE) {
    pattern4Loop(stripShort, stripLong, stripSign);
  } else {
    pattern5Loop(stripShort, stripLong, stripSign);
  }
}
