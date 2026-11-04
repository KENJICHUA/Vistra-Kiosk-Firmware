#ifndef VISTRA_CONFIG_H
#define VISTRA_CONFIG_H

#include <Arduino.h>

// Serial
static constexpr uint32_t BAUD_RATE       = 115200;
static constexpr unsigned long I2C_CLOCK_HZ = 100000; // 100 kHz shared bus

// I2C addresses
static constexpr uint8_t MLX90614_ADDR   = 0x5A;
static constexpr uint8_t MLX_AMBIENT_REG = 0x06;
static constexpr uint8_t MLX_OBJECT_REG  = 0x07;
static constexpr uint8_t VL53L1X_ADDR    = 0x29;

// Height settings
// Replace 215.0 with the ACTUAL distance from VL53L1X sensor face
// to the top of the standing platform.
static constexpr float  SENSOR_TO_FLOOR_CM     = 215.0f;
static constexpr byte   HEIGHT_SAMPLES         = 9;
static constexpr float  HEIGHT_MAX_SPREAD_CM   = 2.0f;
static constexpr float  MIN_PERSON_HEIGHT_CM   = 80.0f;
static constexpr float  MAX_PERSON_HEIGHT_CM   = 220.0f;
static constexpr unsigned long HEIGHT_WAIT_TIMEOUT = 10000;
static constexpr byte   HEIGHT_MAX_ATTEMPTS    = 3;

// Temperature settings
static constexpr byte   TEMP_SAMPLES       = 9;
static constexpr float  TEMP_MAX_SPREAD_C  = 0.60f;
static constexpr float  TEMP_CAL_SCALE     = 1.0f;
static constexpr float  TEMP_CAL_OFFSET    = 0.0f;
// Ambient/no hand ~30-31 C, hand ~35 C
static constexpr float  HAND_MIN_DELTA_C   = 1.5f;
static constexpr unsigned long HAND_WAIT_TIMEOUT   = 10000;
static constexpr unsigned long HAND_STABILIZE_MS   = 1500;

// Heart / SpO2 settings
static constexpr byte   HEART_BUFFER_SIZE = 100;
static constexpr uint32_t FINGER_THRESHOLD          = 50000;
static constexpr uint32_t FINGER_REMOVED_THRESHOLD  = 30000;
static constexpr byte   HEART_WINDOWS               = 5;
static constexpr int    MIN_BPM                     = 40;
static constexpr int    MAX_BPM                     = 200;
static constexpr int    MIN_SPO2                    = 80;
static constexpr int    MAX_SPO2                    = 100;
static constexpr int    HEART_MAX_SPREAD_BPM        = 12;
static constexpr int    SPO2_MAX_SPREAD             = 4;
// Outlier rejection
static constexpr int    BPM_OUTLIER_LIMIT           = 15;
static constexpr int    SPO2_OUTLIER_LIMIT          = 4;
static constexpr byte   MIN_VALID_HEART_WINDOWS     = 3;

// Command buffer
static constexpr size_t kCmdBufferSize = 32;

#endif // VISTRA_CONFIG_H
