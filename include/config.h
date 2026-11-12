#ifndef VISTRA_CONFIG_H
#define VISTRA_CONFIG_H

#include <stdint.h>
#include <stddef.h>

// Serial
static constexpr uint32_t BAUD_RATE       = 115200;
static constexpr unsigned long I2C_CLOCK_HZ = 100000; // 100 kHz shared bus

// I2C addresses
static constexpr uint8_t MLX90614_ADDR   = 0x5A;
static constexpr uint8_t MLX_AMBIENT_REG = 0x06;
static constexpr uint8_t MLX_OBJECT_REG  = 0x07;
static constexpr uint8_t VL53L1X_ADDR    = 0x29;

// Height settings
// Factory default. Replace with calibrated value via CAL SET HEIGHT <cm>.
// The calibrated value persists in EEPROM and overrides this at runtime.
static constexpr float  DEFAULT_SENSOR_TO_FLOOR_CM = 215.0f;
static constexpr uint8_t HEIGHT_SAMPLES         = 9;
static constexpr float  HEIGHT_MAX_SPREAD_CM   = 2.0f;
static constexpr float  MIN_PERSON_HEIGHT_CM   = 80.0f;
static constexpr float  MAX_PERSON_HEIGHT_CM   = 220.0f;
static constexpr unsigned long HEIGHT_WAIT_TIMEOUT = 10000;
static constexpr uint8_t HEIGHT_MAX_ATTEMPTS    = 3;

// Temperature settings
static constexpr uint8_t TEMP_SAMPLES       = 9;
static constexpr float  TEMP_MAX_SPREAD_C  = 0.60f;
// Factory defaults. Overridden at runtime by EEPROM calibration.
static constexpr float  DEFAULT_TEMP_CAL_SCALE = 1.0f;
static constexpr float  DEFAULT_TEMP_CAL_OFFSET = 0.0f;
// Ambient/no hand ~30-31 C, hand ~35 C
static constexpr float  HAND_MIN_DELTA_C   = 0.5f;  // lower = more sensitive
static constexpr float  HAND_MAX_DELTA_C   = 15.0f; // above this = fire/extreme heat, not hand
static constexpr unsigned long HAND_WAIT_TIMEOUT   = 10000;
static constexpr unsigned long HAND_STABILIZE_MS   = 1500;

// Heart / SpO2 settings
static constexpr uint8_t HEART_BUFFER_SIZE = 100;
static constexpr uint32_t FINGER_THRESHOLD          = 50000;
static constexpr uint32_t FINGER_REMOVED_THRESHOLD  = 30000;
static constexpr uint8_t HEART_WINDOWS               = 5;
static constexpr int    MIN_BPM                     = 40;
static constexpr int    MAX_BPM                     = 200;
static constexpr int    MIN_SPO2                    = 80;
static constexpr int    MAX_SPO2                    = 100;
static constexpr int    HEART_MAX_SPREAD_BPM        = 12;
static constexpr int    SPO2_MAX_SPREAD             = 4;
// Outlier rejection
static constexpr int    BPM_OUTLIER_LIMIT           = 15;
static constexpr int    SPO2_OUTLIER_LIMIT          = 4;
static constexpr uint8_t MIN_VALID_HEART_WINDOWS     = 3;

// Command buffer
static constexpr size_t kCmdBufferSize = 32;

// ---- HX711 weight sensor settings ----
// HX711 uses two digital GPIO pins (not I2C).
static constexpr uint8_t HX711_DOUT_PIN = 48;
static constexpr uint8_t HX711_SCK_PIN  = 49;

// Weight calibration factory defaults. Persisted in EEPROM via CAL SET WEIGHT ...
static constexpr float  DEFAULT_WEIGHT_SCALE_FACTOR = 1.0f; // raw units per kg
static constexpr float  DEFAULT_WEIGHT_OFFSET       = 0.0f;  // tare raw value
static constexpr uint8_t WEIGHT_SAMPLES              = 10;    // samples to average
static constexpr float  MIN_WEIGHT_KG               = -5.0f;
static constexpr float  MAX_WEIGHT_KG               = 500.0f;

// Heart calibration factory defaults
static constexpr float  DEFAULT_HEART_CAL_SCALE     = 1.0f; // multiply measured BPM
static constexpr float  DEFAULT_HEART_CAL_OFFSET    = 0.0f; // add to measured BPM

// ---- TEST command timeouts ----
static constexpr unsigned long TEST_TOF_TIMEOUT_MS      = 1000;
static constexpr unsigned long TEST_TEMP_TIMEOUT_MS     = 2000;
static constexpr unsigned long TEST_HEART_TIMEOUT_MS    = 5000;
static constexpr unsigned long TEST_WEIGHT_TIMEOUT_MS   = 1000;

// ---- TEST validation ranges ----
static constexpr float  TEST_DISTANCE_MIN_CM  = 1.0f;
static constexpr float  TEST_DISTANCE_MAX_CM  = 500.0f;
static constexpr float  TEST_TEMP_MIN_C       = 20.0f;
static constexpr float  TEST_TEMP_MAX_C       = 45.0f;
static constexpr float  TEST_AMBIENT_MIN_C    = 10.0f;
static constexpr float  TEST_AMBIENT_MAX_C    = 40.0f;

// ---- RESCAN / hot-swap settings ----
static constexpr unsigned long RESCAN_INTERVAL_MS = 5000; // auto-rescan every 5s
static constexpr unsigned long RESCAN_I2C_TIMEOUT_MS = 500; // per-device I2C probe

#endif // VISTRA_CONFIG_H
