#include "sign_leds.h"

#include <Arduino.h>

bool signWarmWhite = false;

namespace {

// 16 MHz, 800 kHz, same bit timing as Adafruit_NeoPixel::show().
// One color is repeated `count` times. Bytes are sent in the order given.
// LED3 pixels are GRB, so showSignSolid passes green, red, blue. The three
// byte-loops stay inside one asm block so the line
// never goes idle long enough to latch between pixels.
void sendRgbRepeat(volatile uint8_t *port, uint8_t pinMask, uint16_t count,
                   uint8_t r, uint8_t g, uint8_t b) {
  uint8_t byte = r;
  uint8_t bit = 8;
  uint8_t hi = *port | pinMask;
  uint8_t lo = *port & ~pinMask;
  uint8_t next = lo;

  noInterrupts();
  asm volatile(
      "headR%=:"
      "\n\t"
      "st   %a[port], %[hi]"
      "\n\t"
      "sbrc %[byte], 7"
      "\n\t"
      "mov  %[next], %[hi]"
      "\n\t"
      "dec  %[bit]"
      "\n\t"
      "st   %a[port], %[next]"
      "\n\t"
      "mov  %[next], %[lo]"
      "\n\t"
      "breq nextR%="
      "\n\t"
      "rol  %[byte]"
      "\n\t"
      "rjmp .+0"
      "\n\t"
      "nop"
      "\n\t"
      "st   %a[port], %[lo]"
      "\n\t"
      "nop"
      "\n\t"
      "rjmp .+0"
      "\n\t"
      "rjmp headR%="
      "\n\t"
      "nextR%=:"
      "\n\t"
      "ldi  %[bit], 8"
      "\n\t"
      "mov  %[byte], %[gval]"
      "\n\t"
      "nop"
      "\n\t"
      "st   %a[port], %[lo]"
      "\n\t"
      "nop"
      "\n\t"
      "rjmp .+0"
      "\n\t"
      "rjmp headG%="
      "\n\t"

      "headG%=:"
      "\n\t"
      "st   %a[port], %[hi]"
      "\n\t"
      "sbrc %[byte], 7"
      "\n\t"
      "mov  %[next], %[hi]"
      "\n\t"
      "dec  %[bit]"
      "\n\t"
      "st   %a[port], %[next]"
      "\n\t"
      "mov  %[next], %[lo]"
      "\n\t"
      "breq nextG%="
      "\n\t"
      "rol  %[byte]"
      "\n\t"
      "rjmp .+0"
      "\n\t"
      "nop"
      "\n\t"
      "st   %a[port], %[lo]"
      "\n\t"
      "nop"
      "\n\t"
      "rjmp .+0"
      "\n\t"
      "rjmp headG%="
      "\n\t"
      "nextG%=:"
      "\n\t"
      "ldi  %[bit], 8"
      "\n\t"
      "mov  %[byte], %[bval]"
      "\n\t"
      "nop"
      "\n\t"
      "st   %a[port], %[lo]"
      "\n\t"
      "nop"
      "\n\t"
      "rjmp .+0"
      "\n\t"
      "rjmp headB%="
      "\n\t"

      "headB%=:"
      "\n\t"
      "st   %a[port], %[hi]"
      "\n\t"
      "sbrc %[byte], 7"
      "\n\t"
      "mov  %[next], %[hi]"
      "\n\t"
      "dec  %[bit]"
      "\n\t"
      "st   %a[port], %[next]"
      "\n\t"
      "mov  %[next], %[lo]"
      "\n\t"
      "breq nextB%="
      "\n\t"
      "rol  %[byte]"
      "\n\t"
      "rjmp .+0"
      "\n\t"
      "nop"
      "\n\t"
      "st   %a[port], %[lo]"
      "\n\t"
      "nop"
      "\n\t"
      "rjmp .+0"
      "\n\t"
      "rjmp headB%="
      "\n\t"
      "nextB%=:"
      "\n\t"
      "ldi  %[bit], 8"
      "\n\t"
      "mov  %[byte], %[rval]"
      "\n\t"
      "nop"
      "\n\t"
      "st   %a[port], %[lo]"
      "\n\t"
      "nop"
      "\n\t"
      "sbiw %[pixels], 1"
      "\n\t"
      "brne headR%="
      "\n"
      : [port] "+e"(port), [byte] "+&r"(byte), [bit] "+&d"(bit),
        [next] "+&r"(next), [pixels] "+w"(count)
      : [hi] "r"(hi), [lo] "r"(lo), [rval] "r"(r), [gval] "r"(g), [bval] "r"(b));
  interrupts();
}

}  // namespace

void showSignSolid(Adafruit_NeoPixel &stripSign, uint16_t count, uint8_t r,
                   uint8_t g, uint8_t b) {
  if (count == 0 || stripSign.getPin() < 0) {
    return;
  }

  // Match Adafruit setPixelColor(): full brightness (255) is unscaled.
  const uint8_t level = stripSign.getBrightness();
  if (level != 255) {
    const uint8_t stored = static_cast<uint8_t>(level + 1);
    r = static_cast<uint8_t>((r * stored) >> 8);
    g = static_cast<uint8_t>((g * stored) >> 8);
    b = static_cast<uint8_t>((b * stored) >> 8);
  }

  static uint32_t endTime = 0;
  uint32_t now = micros();
  if (endTime > now) {
    endTime = now;
  }
  while ((micros() - endTime) < 300L) {
  }

  const int16_t pin = stripSign.getPin();
  // This strand shows a red value as green, so the first byte is green (GRB).
  sendRgbRepeat(portOutputRegister(digitalPinToPort(pin)),
                digitalPinToBitMask(pin), count, g, r, b);
  endTime = micros();
}
