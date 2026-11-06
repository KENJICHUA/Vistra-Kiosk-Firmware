#include "calibration.h"

// ---- Runtime calibration values (actual storage) ----
float g_sensorToFloorCm  = DEFAULT_SENSOR_TO_FLOOR_CM;
float g_tempCalScale     = DEFAULT_TEMP_CAL_SCALE;
float g_tempCalOffset    = DEFAULT_TEMP_CAL_OFFSET;
float g_weightScaleFactor = DEFAULT_WEIGHT_SCALE_FACTOR;
float g_weightOffset     = DEFAULT_WEIGHT_OFFSET;

// ---- Checksum: sum of all bytes except the checksum field itself ----
// (defined as static inline in calibration.h for testability)

bool calibrationIsValid() {
  CalibrationRecord rec;
  EEPROM.get(kEepromAddr, rec);
  if (rec.magic != kCalMagic || rec.version != kCalVersion) {
    return false;
  }
  return rec.checksum == computeChecksum(rec);
}

void calibrationInit() {
  if (calibrationIsValid()) {
    CalibrationRecord rec;
    EEPROM.get(kEepromAddr, rec);
    g_sensorToFloorCm  = rec.sensorToFloorCm;
    g_tempCalScale     = rec.tempCalScale;
    g_tempCalOffset    = rec.tempCalOffset;
    g_weightScaleFactor = rec.weightScaleFactor;
    g_weightOffset     = rec.weightOffset;
    Serial.println(F("Calibration loaded from EEPROM"));
  } else {
    calibrationReset();
    Serial.println(F("Calibration: using factory defaults"));
  }
}

bool calibrationSave() {
  CalibrationRecord rec;
  rec.magic          = kCalMagic;
  rec.version        = kCalVersion;
  rec.sensorToFloorCm = g_sensorToFloorCm;
  rec.tempCalScale    = g_tempCalScale;
  rec.tempCalOffset   = g_tempCalOffset;
  rec.weightScaleFactor = g_weightScaleFactor;
  rec.weightOffset    = g_weightOffset;
  rec.checksum        = computeChecksum(rec);
  EEPROM.put(kEepromAddr, rec);
  return true;
}

void calibrationReset() {
  g_sensorToFloorCm  = DEFAULT_SENSOR_TO_FLOOR_CM;
  g_tempCalScale     = DEFAULT_TEMP_CAL_SCALE;
  g_tempCalOffset    = DEFAULT_TEMP_CAL_OFFSET;
  g_weightScaleFactor = DEFAULT_WEIGHT_SCALE_FACTOR;
  g_weightOffset     = DEFAULT_WEIGHT_OFFSET;
}

void calibrationPrint() {
  Serial.print(F("{\"type\":\"calibration\","));
  Serial.print(F("\"sensor_to_floor_cm\":"));
  Serial.print(g_sensorToFloorCm, 2);
  Serial.print(F(",\"temp_cal_scale\":"));
  Serial.print(g_tempCalScale, 3);
  Serial.print(F(",\"temp_cal_offset\":"));
  Serial.print(g_tempCalOffset, 2);
  Serial.print(F(",\"weight_scale_factor\":"));
  Serial.print(g_weightScaleFactor, 4);
  Serial.print(F(",\"weight_offset\":"));
  Serial.print(g_weightOffset, 2);
  Serial.print(F(",\"eeprom_valid\":"));
  Serial.print(calibrationIsValid() ? F("true") : F("false"));
  Serial.println(F("}"));
}
