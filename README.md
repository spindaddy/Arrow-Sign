# Arrow Sign

ESP32-C3 firmware for a bent-arrow NeoPixel sign, plus a browser LED simulator traced from the CAD outline.

## Hardware

| Item | Detail |
|------|--------|
| Board | ESP32-C3 Super Mini |
| Short strip | GPIO **4**, **24** LEDs |
| Long strip | GPIO **2**, **35** LEDs |
| Color order | `NEO_RGB` (warm yellow-white ~ RGB 255, 210, 60) |

## Patterns

Patterns cycle every **60 seconds**. Boot starts on **Broken Sign**.

| # | Name | Description |
|---|------|-------------|
| 1 | Chase Fast | 3 groups × 2 LEDs; sync short7↔long11 and short9↔long20; short LED23 mirrors 22 |
| 2 | Chase Half | Same as Chase Fast at 50% speed |
| 3 | Chase Fine | Same landmarks; 10 groups × 1 LED; ~75% of Pattern 1 speed |
| 4 | Twinkle | All LEDs on with soft random brightness |
| 5 | Broken Sign | Mostly on; a few chronic flickers; chunks go dead for a while |

Pattern code lives in `src/pattern1.cpp` … `src/pattern5.cpp`. Selection and cycling are in `src/main.cpp`.

## Build & flash

Requires [PlatformIO](https://platformio.org/).

```bash
pio run -t upload
pio device monitor   # optional, 115200 baud
```

USB CDC is enabled for the Super Mini’s native USB port.

## LED simulator

Open `sim/arrow-leds.html` in a browser to preview patterns on the CAD outline (no board needed). Outline and hole positions are in `sim/outline.json` / `sim/outline-data.js`.

## Repo layout

```
src/           Firmware (main + patterns)
sim/           Browser LED simulator
platformio.ini Board / library config
```
