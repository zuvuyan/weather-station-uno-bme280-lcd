// WeatherLCD - read a BME280 (I2C 0x76) and show temperature, humidity and
// pressure on a 16x2 HD44780 LCD driven by a PCF8574 "GY" I2C backpack (I2C 0x20).
//
// Bus: Uno SDA = A4, SCL = A5. Both devices share the same I2C bus.
//
// Libraries (already installed):
//   LiquidCrystal_I2C_GY   - LCD via the GY-series PCF8574 backpack
//                            (RS=P6, RW=P5, E=P4, BL=P7 active-low, D4..D7=P0..P3)
//   cactus_io_BME280_I2C   - BME280 sensor

#include <Wire.h>
#include <LiquidCrystal_I2C_GY.h>
#include <cactus_io_BME280_I2C.h>

const uint8_t LCD_ADDR = 0x20;
const uint8_t BME_ADDR = 0x76;
const uint8_t LCD_COLS = 16;
const uint8_t LCD_ROWS = 2;

const unsigned long REFRESH_MS = 2000;

// ---- user-adjustable calibration -------------------------------------------
// Temperature offset in degrees C, added to the raw reading. Negative trims
// down (e.g. to cancel sensor self-heating).
#define TEMP_CAL_C   -1.4

// Station altitude above sea level, in metres. Used to reduce the measured
// (station) pressure to its sea-level equivalent.
// 123 m = postcode BL6 7PH, Horwich (Copernicus DEM / OS-derived).
#define ALTITUDE_M    123.0
// ---------------------------------------------------------------------------

LiquidCrystal_I2C_GY lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
BME280_I2C bme(BME_ADDR);

// Write a C-string to one LCD row, blank-padded to the full width so stale
// characters from a previous (longer) reading are cleared.
void printRow(uint8_t row, const char *s) {
  lcd.setCursor(0, row);
  uint8_t i = 0;
  for (; s[i] && i < LCD_COLS; i++) lcd.write((uint8_t)s[i]);
  for (; i < LCD_COLS; i++) lcd.write(' ');
}

// Reduce a station-pressure reading to its sea-level equivalent (QNH-style),
// using the current temperature and a fixed station altitude.
float toSeaLevel(float stationHPa, float tempC, float altM) {
  return stationHPa *
         pow(1.0 - (0.0065 * altM) / (tempC + 0.0065 * altM + 273.15), -5.257);
}

void halt(const char *msg) {
  Serial.print(F("FATAL: "));
  Serial.println(msg);
  lcd.clear();
  printRow(0, "SENSOR ERROR");
  printRow(1, msg);
  while (true) {}
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  printRow(0, "Weather station");
  printRow(1, "starting...");

  if (!bme.begin()) {
    halt("BME280 0x76 fail");
  }
  bme.setTempCal(TEMP_CAL_C);

  delay(500);
  lcd.clear();
}

void loop() {
  static unsigned long last = 0;
  if (last != 0 && millis() - last < REFRESH_MS) return;
  last = millis();

  bme.readSensor();
  float tC     = bme.getTemperature_C();            // already includes TEMP_CAL_C
  float rh     = bme.getHumidity();
  float staHPa = bme.getPressure_MB();              // station pressure, hPa
  float mslHPa = toSeaLevel(staHPa, tC, ALTITUDE_M); // reduced to sea level

  // Full-precision copy to the serial monitor.
  Serial.print(tC, 2);     Serial.print(F(" C  "));
  Serial.print(rh, 2);     Serial.print(F(" %RH  station "));
  Serial.print(staHPa, 2); Serial.print(F(" hPa  MSL "));
  Serial.print(mslHPa, 2); Serial.println(F(" hPa"));

  char num[10];
  char line[24];

  // Row 0:  "23.4<deg>C  45.6%RH"
  line[0] = '\0';
  dtostrf(tC, 1, 1, num);           strcat(line, num);
  {
    size_t k = strlen(line);
    line[k++] = (char)0xDF;         // HD44780 degree symbol
    line[k++] = 'C';
    line[k]   = '\0';
  }
  strcat(line, "  ");
  dtostrf(rh, 1, 1, num);           strcat(line, num);
  strcat(line, "%RH");
  printRow(0, line);

  // Row 1:  "1013.2 hPa" (sea-level corrected)
  line[0] = '\0';
  dtostrf(mslHPa, 1, 1, num);       strcat(line, num);
  strcat(line, " hPa");
  printRow(1, line);
}
