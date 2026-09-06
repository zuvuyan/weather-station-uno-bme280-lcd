# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A single Arduino sketch (`WeatherLCD/WeatherLCD.ino`) for an Arduino Uno: it reads a Bosch
BME280 over I²C and displays temperature, humidity and sea-level–corrected pressure on a 16×2
HD44780 LCD driven by a PCF8574 I²C backpack. There is no application framework, no test suite,
and only one build target.

## Commands

Compile (this is exactly what CI runs):

```sh
arduino-cli compile --fqbn arduino:avr:uno --libraries ./libraries --warnings all ./WeatherLCD
```

`--libraries ./libraries` is **required** — the two dependencies are vendored in `libraries/`
and are not in the Arduino Library Manager (see Architecture). A clean build is ~40% flash /
~28% RAM.

Flash to a connected board (ask the user before doing this — it writes to hardware):

```sh
arduino-cli board list                                              # find the port
arduino-cli upload --fqbn arduino:avr:uno -p <PORT> ./WeatherLCD
```

On the current Windows dev machine `arduino-cli` is **not on PATH** in fresh tool shells; use
the copy bundled with Arduino IDE:
`C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe`.

There is no `arduino-cli monitor` workflow set up here. To read the 9600-baud serial output
from PowerShell, open a `System.IO.Ports.SerialPort` at 9600 and toggle `DtrEnable` to reset
the board and capture output from boot.

Verification path: compile → flash → read serial. CI (`.github/workflows/build.yml`) covers the
compile on any change to `WeatherLCD/**`, `libraries/**`, or the workflow file.

## Architecture

**Everything is on one I²C bus** (Uno A4 = SDA, A5 = SCL). No other Arduino pins are used.
Addresses are hardcoded: `LCD_ADDR 0x20`, `BME_ADDR 0x76` (BME280 with SDO tied low; use `0x77`
otherwise).

**The LCD backpack is non-standard — this is the main gotcha.** The PCF8574 "GY" backpack wires
the expander bits to the HD44780 differently from the mass-market backpack:

| PCF8574 bit | This "GY" board | Mass-market board |
|---|---|---|
| P0–P3 | data D4–D7 | RS, RW, E, backlight |
| P4 | E | data D4 |
| P5 | RW | data D5 |
| P6 | RS | data D6 |
| P7 | backlight (active-low) | data D7 |

`LiquidCrystal_I2C_GY` encodes this mapping. **`LiquidCrystal_I2C` / `LiquidCrystal_PCF8574`
will not work** — they leave the display showing uninitialised speckle on line 1. If you ever
swap the LCD library, preserve this mapping.

**Vendored libraries** (`libraries/`, unmodified upstream copies — do not edit them; behaviour
changes belong in the sketch):
- `LiquidCrystal_I2C_GY` — MIT, from github.com/t3chguy/LiquidCrystal_I2C_GY
- `cactus_io_BME280_I2C` — from cactus.io, no stated license. Quirk: `getPressure_MB()` returns
  hPa (~1000); `getPressure_HP()` is mislabeled and returns pascals. The sketch uses `_MB()`.

**Sketch structure** (`WeatherLCD/WeatherLCD.ino`):
- Two user knobs as `#define` at the top: `TEMP_CAL_C` (°C offset added to every reading) and
  `ALTITUDE_M` (station altitude, metres — drives the sea-level reduction).
- `toSeaLevel()` — hypsometric reduction of station pressure to MSL using the current
  temperature and `ALTITUDE_M`. The LCD's bottom row shows the MSL value; serial prints both
  station and MSL.
- `printRow()` — writes a string to one LCD row, blank-padded to `LCD_COLS` so a shorter new
  reading clears the previous one. The degree glyph is the raw HD44780 byte `0xDF`.
- `halt()` — on sensor init failure, prints an error to the LCD and serial and spins forever
  (never returns).
- `loop()` refreshes every `REFRESH_MS` (2 s) via a `millis()` check, not `delay()`.

## Conventions

- `.gitattributes` normalises line endings to LF in the repo.
- `README.md` is the user-facing build/wiring/troubleshooting guide and `docs/wiring.svg` the
  wiring diagram — keep both in sync when hardware assumptions or the pin table change.
