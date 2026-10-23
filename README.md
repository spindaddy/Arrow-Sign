# Arrow Sign

Arduino Nano firmware for a bent-arrow NeoPixel sign, plus a browser LED simulator traced from the CAD outline.

## Hardware

Target board matches the EasyEDA schematic (Arduino Nano + LM7805 + DIP SW1).

| Connector | Role | Nano pin | LEDs |
|-----------|------|----------|------|
| LED1 | Short strip | **D2** | 24 |
| LED2 | Long strip | **D3** | 35 |
| LED3 | Sign strip | **D4** | 40 |
| LED4 | Unused (reserved) | **D5** | — |

| Item | Detail |
|------|--------|
| Board | Arduino Nano (ATmega328P) |
| Color order | `NEO_RGB` (warm yellow-white ~ RGB 255, 210, 60) |
| DIP SW1 | **A0–A3**, other side GND → `INPUT_PULLUP` (ON = LOW). **Switch 1 (A0) ON** → all strips at **50%** brightness on every pattern; **Switch 4 (A3) ON** → skip / disable **Broken Sign**; switches 2–3 reserved |
| LED6 / LED7 | 12V strips, pot + MOSFET dimming — hardware only, no MCU pins |

XH headers for LED1–LED4: pin1 GND, pin2 +5V, pin3 data.

## Patterns

Patterns cycle every **60 seconds**. Boot starts on **Chase Fast** (pattern 1).

**LED3 (sign, D4):** solid warm white (`RGB 255, 210, 60`) during patterns 1–4; random whole-strip on/off flicker during **Broken Sign** (pattern 5).

| # | Name | Description |
|---|------|-------------|
| 1 | Chase Fast ★ boot | 3 groups × 2 LEDs; sync short7↔long11 and short9↔long20; short LED23 mirrors 22 |
| 2 | Chase Half | Same as Chase Fast at 50% speed |
| 3 | Chase Fine | Same landmarks; 10 groups × 1 LED; ~75% of Pattern 1 speed |
| 4 | Twinkle | Short + long soft random twinkle; sign solid warm white |
| 5 | Broken Sign | Short + long faults/dead groups; sign flickers randomly off/on |

Pattern code lives in `src/pattern1.cpp` … `src/pattern5.cpp`. Selection and cycling are in `src/main.cpp`.

## Build & flash

Requires [PlatformIO](https://platformio.org/).

```bash
pio run -t upload
pio device monitor   # optional, 115200 baud
```

Default env is `nanoatmega328` (classic Nano bootloader). For newer Optiboot Nanos, set `board = nanoatmega328new` in `platformio.ini`.

## LED simulator

Open `sim/arrow-leds.html` in a browser to preview patterns on the CAD outline (no board needed). Outline and hole positions are in `sim/outline.json` / `sim/outline-data.js`.

The simulator still shows the **short + long** arrow CAD only; the 40-LED sign strip is firmware-side and not laid out in the sim yet.

## Repo layout

```
src/           Firmware (main + patterns)
sim/           Browser LED simulator
platformio.ini Board / library config
```
