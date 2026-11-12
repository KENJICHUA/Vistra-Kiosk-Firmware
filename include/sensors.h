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

// ---- Hot-swap / rescan ----
bool rescanSensors();           // re-detect all sensors, update health flags
bool reinitTof();               // re-init VL53L1X only
bool reinitMlx();               // re-init MLX90614 only
bool reinitHeart();             // re-init MAX30102 only
bool reinitWeight();            // re-init HX711 only
bool i2cDeviceExists(uint8_t address);  // I2C device probe (public for rescan)

#endif // VISTRA_SENSORS_H
