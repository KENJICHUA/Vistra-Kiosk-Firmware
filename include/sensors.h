#ifndef VISTRA_SENSORS_H
#define VISTRA_SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L1X.h>
#include <MAX30105.h>

#include "config.h"

// ---- Shared sensor objects (defined in sensors.cpp) ----
extern Adafruit_VL53L1X vl53l1x;
extern MAX30105         max30102;

// ---- Shared sensor health flags (defined in sensors.cpp) ----
extern bool g_tofReady;
extern bool g_heartReady;
extern bool g_mlxReady;

// ---- Cross-module sensor operations ----
bool initSensors();
float readMLXTemperature(uint8_t reg);
float readDistanceCM();

#endif // VISTRA_SENSORS_H
