/*
 * Vistra Kiosk Firmware — Arduino Mega 2560 (V5 port)
 *
 * I2C: SDA -> Pin 20, SCL -> Pin 21
 *
 * Sensors:
 *   VL53L1X  -> 0x29  (height / distance)
 *   MAX30102 -> 0x57  (heart rate + SpO2)
 *   MLX90614 -> 0x5A  (object + ambient temperature)
 *
 * Serial: 115200 baud, newline-terminated commands
 *   STATUS  — per-sensor health (JSON)
 *   HEIGHT  — person detection + height (JSON)
 *   TEMP    — hand detection + skin temperature (JSON)
 *   HEART   — finger detection + BPM/SpO2 (JSON)
 *
 * This file is the thin orchestrator: setup() + loop() only.
 * All command logic lives in feature modules under src/.
 */

#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "sensors.h"
#include "serial.h"

// ============================================================
// SETUP / LOOP
// ============================================================

void setup() {
  Serial.begin(BAUD_RATE);
  delay(1000);

  Serial.println();
  Serial.println(F("VISTRA SENSOR SYSTEM V5"));
  Serial.println(F("-----------------------"));

  Wire.begin();
  Wire.setClock(I2C_CLOCK_HZ); // 100 kHz for shared I2C bus
  delay(100);

  initSensors();

  Serial.println();
  Serial.println(F("ARDUINO READY"));
  Serial.println(F("Commands: STATUS HEIGHT TEMP HEART"));
}

void loop() {
  processSerial();
}
