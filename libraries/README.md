# Bundled libraries

Neither library below is in the Arduino Library Manager, so both are vendored here to make the
project build from a clean checkout. They are unmodified copies.

**To use them:** either copy each folder into your Arduino sketchbook `libraries/` directory
(`~/Arduino/libraries/` on macOS/Linux, `%USERPROFILE%\Documents\Arduino\libraries\` on
Windows) and restart the IDE, or build with
`arduino-cli compile --libraries ./libraries ...` as shown in the top-level README.

---

## LiquidCrystal_I2C_GY

- **Purpose:** drives an HD44780 character LCD through a *GY-IICLCD*–style PCF8574 I²C backpack,
  whose expander-bit-to-LCD-pin wiring differs from the mass-market backpack (RS→P6, RW→P5,
  E→P4, backlight→P7 active-low, data→P0–P3).
- **Upstream:** <https://github.com/t3chguy/LiquidCrystal_I2C_GY> (the `-master` snapshot,
  library version "V2.0" per the source header).
- **License:** MIT — see `LiquidCrystal_I2C_GY/LICENSE` (Copyright (c) 2015 Michael Telatynski).
- **Changes:** none. The upstream `info/` folder (datasheet PDFs and photos) was omitted to keep
  the repo small.

## cactus_io_BME280_I2C

- **Purpose:** reads the Bosch BME280, applying the factory calibration compensation, and
  returns temperature (°C/°F), relative humidity (%) and pressure (Pa / hPa / mbar).
- **Upstream:** cactus.io — search "cactus.io BME280 library". Constructor `BME280_I2C(0x76)`
  selects the alternate I²C address.
- **License:** none stated. The source carries only `No warranty is given`. Included here
  unmodified for convenience; if redistribution licensing matters for your use, download it
  from cactus.io yourself.
- **Changes:** none, except removing a stray `.DS_Store`.
