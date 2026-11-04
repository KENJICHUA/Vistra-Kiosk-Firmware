#ifndef VISTRA_CALIBRATION_H
#define VISTRA_CALIBRATION_H

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"

// ---- Runtime calibration values (loaded from EEPROM, defaults from config.h) ----
extern float g_sensorToFloorCm;
extern float g_tempCalScale;
extern float g_tempCalOffset;
extern float g_weightScaleFactor;
extern float g_weightOffset;

// ---- EEPROM layout ----
static constexpr int kEepromAddr = 0;
static constexpr uint16_t kCalMagic = 0x4B43;   // "KC"
static constexpr uint8_t kCalVersion = 2;        // v2 adds weight calibration

struct CalibrationRecord {
  uint16_t magic;
  uint8_t  version;
  float    sensorToFloorCm;
  float    tempCalScale;
  float    tempCalOffset;
  float    weightScaleFactor;
  float    weightOffset;
  uint16_t checksum;
};

// ---- Calibration API ----
void calibrationInit();      // load from EEPROM if valid, else defaults
bool calibrationSave();      // persist current values to EEPROM
void calibrationReset();     // reset RAM values to factory defaults
bool calibrationIsValid();   // check EEPROM contents integrity
void calibrationPrint();     // print current calibration as JSON

#endif // VISTRA_CALIBRATION_H
