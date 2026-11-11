/*
 * BH1750 Diagnostic v3 - CANNOT FREEZE
 * Uses watchdog timer: if Wire hangs, Arduino auto-resets in 2 seconds.
 * Also enables internal pull-ups on I2C lines.
 */

#include <Wire.h>
#include <avr/wdt.h>

#define BH1750_ADDR 0x23

byte sensorAddr = 0;

void setup() {
  // Disable watchdog first (in case we just came from a WDT reset)
  wdt_disable();

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(2000);

  // Check if we rebooted from a watchdog reset
  if (MCUSR & (1 << WDRF)) {
    Serial.println(F("!! REBOOTED FROM WATCHDOG (I2C bus hung) !!"));
    Serial.println(F("!! Your I2C wiring is unreliable !!"));
    Serial.println(F("!! ADD 4.7k PULL-UP RESISTORS !!"));
    Serial.println();
    MCUSR = 0; // Clear the flag
  }

  Serial.println(F("============================="));
  Serial.println(F("  BH1750 DIAGNOSTIC v3"));
  Serial.println(F("  (Watchdog protected)"));
  Serial.println(F("============================="));
  Serial.println();

  // Enable internal pull-ups on SDA/SCL BEFORE Wire.begin()
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  delay(100); // Let lines settle

  // Check pin voltages after enabling pull-ups
  // Temporarily set back to input to read analog
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  int sdaVal = analogRead(A4);
  int sclVal = analogRead(A5);
  // Re-enable pull-ups
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);

  Serial.println(F("--- Pin Voltages (with internal pull-ups) ---"));
  Serial.print(F("  SDA (A4) = "));
  Serial.print(sdaVal);
  if (sdaVal > 800) Serial.println(F(" GOOD"));
  else if (sdaVal > 400) Serial.println(F(" WEAK (needs external pull-up)"));
  else Serial.println(F(" BAD (wiring problem!)"));

  Serial.print(F("  SCL (A5) = "));
  Serial.print(sclVal);
  if (sclVal > 800) Serial.println(F(" GOOD"));
  else if (sclVal > 400) Serial.println(F(" WEAK (needs external pull-up)"));
  else Serial.println(F(" BAD (wiring problem!)"));
  Serial.println();

  // NOW enable watchdog - if Wire.begin() or scan hangs, we auto-reset
  Serial.println(F("Enabling watchdog (2s timeout)..."));
  Serial.println(F("Starting Wire..."));
  Serial.flush(); // Make sure this prints before potential hang
  
  wdt_enable(WDTO_2S);
  Wire.begin();
  wdt_reset();
  Serial.println(F("Wire OK!"));
  Serial.println();

  // ---- I2C Bus Scan (watchdog protected) ----
  Serial.println(F("--- Scanning I2C bus ---"));
  Serial.flush();
  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    wdt_reset(); // Pet the watchdog each iteration
    Wire.beginTransmission(addr);
    byte err = Wire.endTransmission();
    if (err == 0) {
      Serial.print(F("  FOUND - 0x"));
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      if (addr == 0x23) Serial.print(F(" <- BH1750 (ADDR=GND)"));
      if (addr == 0x5C) Serial.print(F(" <- BH1750 (ADDR=VCC)"));
      Serial.println();
      found++;
    }
  }
  wdt_reset();
  
  if (found == 0) {
    Serial.println(F("  NO DEVICES FOUND!"));
  } else {
    Serial.print(F("  Total: "));
    Serial.println(found);
  }
  Serial.println();

  // ---- Try BH1750 ----
  Wire.beginTransmission(BH1750_ADDR);
  if (Wire.endTransmission() == 0) {
    sensorAddr = BH1750_ADDR;
  } else {
    Wire.beginTransmission(0x5C);
    if (Wire.endTransmission() == 0) sensorAddr = 0x5C;
  }
  wdt_reset();

  if (sensorAddr != 0) {
    Serial.print(F("Sensor at 0x"));
    Serial.println(sensorAddr, HEX);

    // Power on
    Wire.beginTransmission(sensorAddr);
    Wire.write(0x01);
    Wire.endTransmission();
    wdt_reset();
    delay(10);

    // Continuous High Res Mode 2
    Wire.beginTransmission(sensorAddr);
    Wire.write(0x11);
    Wire.endTransmission();
    wdt_reset();
    
    Serial.println(F("Waiting 200ms for first reading..."));
    delay(200);
    wdt_reset();

    Wire.requestFrom(sensorAddr, (byte)2);
    if (Wire.available() == 2) {
      uint16_t raw = Wire.read() << 8 | Wire.read();
      float lux = raw / 1.2;
      Serial.print(F("First read - "));
      Serial.print(lux, 2);
      Serial.println(F(" lux - WORKS!"));
    } else {
      Serial.println(F("ERROR: No data from sensor"));
    }
  } else {
    Serial.println(F("BH1750 NOT FOUND"));
  }
  wdt_reset();

  Serial.println();
  Serial.println(F("=== DIAGNOSTIC DONE ==="));
  Serial.println();

  if (sensorAddr != 0) {
    Serial.println(F("--- Continuous readings (watchdog protected) ---"));
  }
}

void loop() {
  wdt_reset(); // Pet watchdog every loop

  if (sensorAddr == 0) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);
    return;
  }

  Wire.requestFrom(sensorAddr, (byte)2);
  wdt_reset();
  
  if (Wire.available() == 2) {
    uint16_t raw = Wire.read() << 8 | Wire.read();
    float lux = raw / 1.2;
    Serial.print(F("Lux:"));
    Serial.println(lux, 2);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  } else {
    Serial.println(F("READ FAIL"));
  }

  delay(150);
}
