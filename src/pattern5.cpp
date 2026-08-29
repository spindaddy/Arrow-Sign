#include "pattern5.h"

#include <Arduino.h>

namespace {

// Warm yellow-white
constexpr uint8_t BASE_R = 255;
constexpr uint8_t BASE_G = 210;
constexpr uint8_t BASE_B = 60;

constexpr int MAX_LEDS = 40;
constexpr uint16_t FRAME_MS = 20;

constexpr int CHRONIC_PER_STRIP = 2;
constexpr int MAX_DEAD_GROUPS = 3;

struct FaultLed {
  bool on;
  bool chronic;
  uint32_t nextMs;
  uint8_t level;
};

struct DeadGroup {
  int start;
  int count;
  bool dead;
  uint32_t nextMs;
};

FaultLed shortFaults[MAX_LEDS];
FaultLed longFaults[MAX_LEDS];
DeadGroup shortGroups[MAX_DEAD_GROUPS];
DeadGroup longGroups[MAX_DEAD_GROUPS];
int shortGroupCount = 0;
int longGroupCount = 0;

uint32_t randomDuration(bool turningOn, bool chronic) {
  if (chronic) {
    // Slower, uneven flicker
    if (turningOn) {
      return random(200, 700);
    }
    return random(150, 500);
  }

  // Mostly on: long lit stretches, short off blips
  if (turningOn) {
    const int roll = random(100);
    if (roll < 55) {
      return random(2000, 6000);
    }
    if (roll < 85) {
      return random(6000, 14000);
    }
    return random(800, 2000);
  }

  const int roll = random(100);
  if (roll < 70) {
    return random(40, 200);
  }
  if (roll < 92) {
    return random(200, 600);
  }
  return random(600, 1500);
}

uint8_t randomLevel(bool chronic) {
  if (chronic) {
    return static_cast<uint8_t>(random(40, 180));
  }
  const int roll = random(100);
  if (roll < 20) {
    return static_cast<uint8_t>(random(40, 80));
  }
  if (roll < 70) {
    return static_cast<uint8_t>(random(80, 140));
  }
  return static_cast<uint8_t>(random(140, 200));
}

void scheduleNext(FaultLed &led, uint32_t now) {
  led.on = !led.on;
  if (led.on) {
    led.level = randomLevel(led.chronic);
  }
  led.nextMs = now + randomDuration(led.on, led.chronic);
}

void pickChronic(FaultLed *faults, int n) {
  for (int i = 0; i < n; i++) {
    faults[i].chronic = false;
  }
  const int count = (n < CHRONIC_PER_STRIP) ? n : CHRONIC_PER_STRIP;
  for (int c = 0; c < count; c++) {
    int idx;
    do {
      idx = random(n);
    } while (faults[idx].chronic);
    faults[idx].chronic = true;
  }
}

uint32_t groupDuration(bool becomingDead) {
  if (becomingDead) {
    // Chunk of the sign goes dark for a while
    const int roll = random(100);
    if (roll < 50) {
      return random(1500, 4000);
    }
    if (roll < 85) {
      return random(4000, 9000);
    }
    return random(9000, 16000);
  }
  // Alive stretch before next outage
  return random(3000, 12000);
}

void initGroups(DeadGroup *groups, int &groupCount, int n, uint32_t now) {
  // 1–2 larger dead zones per strip
  groupCount = 1 + random(2);
  if (groupCount > MAX_DEAD_GROUPS) {
    groupCount = MAX_DEAD_GROUPS;
  }

  for (int g = 0; g < groupCount; g++) {
    const int size = 3 + random(5);  // 3–7 LEDs
    int start = random(n);
    if (start + size > n) {
      start = n - size;
    }
    if (start < 0) {
      start = 0;
    }
    groups[g].start = start;
    groups[g].count = (size > n) ? n : size;
    groups[g].dead = false;
    groups[g].nextMs = now + random(2000, 8000);
  }
}

void updateGroups(DeadGroup *groups, int groupCount, uint32_t now) {
  for (int g = 0; g < groupCount; g++) {
    while (static_cast<int32_t>(now - groups[g].nextMs) >= 0) {
      groups[g].dead = !groups[g].dead;
      groups[g].nextMs = now + groupDuration(groups[g].dead);
    }
  }
}

bool inDeadGroup(const DeadGroup *groups, int groupCount, int i) {
  for (int g = 0; g < groupCount; g++) {
    if (!groups[g].dead) {
      continue;
    }
    if (i >= groups[g].start && i < groups[g].start + groups[g].count) {
      return true;
    }
  }
  return false;
}

void initStrip(FaultLed *faults, int n, uint32_t now) {
  pickChronic(faults, n);
  for (int i = 0; i < n; i++) {
    faults[i].on = faults[i].chronic ? (random(2) == 0) : true;
    faults[i].level = randomLevel(faults[i].chronic);
    faults[i].nextMs =
        now + (faults[i].chronic ? random(200, 800) : random(500, 4000));
  }
}

void updateStrip(Adafruit_NeoPixel &strip, FaultLed *faults, DeadGroup *groups,
                 int groupCount) {
  const int n = static_cast<int>(strip.numPixels());
  const uint32_t now = millis();

  updateGroups(groups, groupCount, now);

  for (int i = 0; i < n && i < MAX_LEDS; i++) {
    while (static_cast<int32_t>(now - faults[i].nextMs) >= 0) {
      scheduleNext(faults[i], now);
    }

    if (inDeadGroup(groups, groupCount, i) || !faults[i].on) {
      strip.setPixelColor(i, 0);
    } else {
      const uint8_t level = faults[i].level;
      const uint8_t r = static_cast<uint8_t>((BASE_R * level) / 255);
      const uint8_t g = static_cast<uint8_t>((BASE_G * level) / 255);
      const uint8_t b = static_cast<uint8_t>((BASE_B * level) / 255);
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
  }
  strip.show();
}

}  // namespace

void pattern5Setup(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong) {
  randomSeed(esp_random());
  const uint32_t now = millis();
  const int nShort = static_cast<int>(stripShort.numPixels());
  const int nLong = static_cast<int>(stripLong.numPixels());

  initStrip(shortFaults, nShort, now);
  initStrip(longFaults, nLong, now);
  initGroups(shortGroups, shortGroupCount, nShort, now);
  initGroups(longGroups, longGroupCount, nLong, now);

  updateStrip(stripShort, shortFaults, shortGroups, shortGroupCount);
  updateStrip(stripLong, longFaults, longGroups, longGroupCount);
  Serial.println("  Broken Sign: slow flickers + dead groups");
}

void pattern5Loop(Adafruit_NeoPixel &stripShort, Adafruit_NeoPixel &stripLong) {
  updateStrip(stripShort, shortFaults, shortGroups, shortGroupCount);
  updateStrip(stripLong, longFaults, longGroups, longGroupCount);
  delay(FRAME_MS);
}
