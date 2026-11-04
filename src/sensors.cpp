#include "sensors.h"
#include "weight.h"

// ---- Shared sensor objects (actual storage) ----
Adafruit_VL53L1X vl53l1x;
MAX30105         max30102;

// ---- Shared sensor health flags (actual storage) ----
bool g_tofReady  = false;
bool g_heartReady = false;
bool g_mlxReady  = false;

// ---- Internal helper ----
static bool i2cDeviceExists(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

// ---- Sensor initialisation ----
bool initSensors() {
  // VL53L1X
  Serial.println(F("Initializing VL53L1X..."));
  if (vl53l1x.begin(VL53L1X_ADDR, &Wire)) {
    if (vl53l1x.startRanging()) {
      vl53l1x.setTimingBudget(100);
      g_tofReady = true;
      Serial.println(F("VL53L1X READY"));
    } else {
      Serial.println(F("VL53L1X RANGING FAILED"));
    }
  } else {
    Serial.println(F("VL53L1X NOT FOUND"));
  }

  // MLX90614
  Serial.println(F("Checking MLX90614..."));
  g_mlxReady = i2cDeviceExists(MLX90614_ADDR);
  Serial.println(g_mlxReady ? F("MLX90614 READY") : F("MLX90614 NOT FOUND"));

  // MAX30102
  Serial.println(F("Initializing MAX30102..."));
  if (max30102.begin(Wire, I2C_SPEED_STANDARD)) {
    max30102.setup(0x2A, 4, 2, 100, 411, 4096);
    max30102.setPulseAmplitudeRed(0x2A);
    max30102.setPulseAmplitudeIR(0x2A);
    max30102.setPulseAmplitudeGreen(0);
    g_heartReady = true;
    Serial.println(F("MAX30102 READY"));
  } else {
    Serial.println(F("MAX30102 NOT FOUND"));
  }

  // HX711 weight sensor
  initWeightSensor();

  return g_tofReady && g_heartReady && g_mlxReady && g_weightReady;
}

// ---- MLX90614 read (direct I2C with PEC byte) ----
float readMLXTemperature(uint8_t reg) {
  Wire.beginTransmission(MLX90614_ADDR);
  Wire.write(reg);
  byte error = Wire.endTransmission(false);
  if (error != 0) {
    return -999.0f;
  }

  Wire.requestFrom((uint8_t)MLX90614_ADDR, (uint8_t)3);
  if (Wire.available() < 3) {
    return -999.0f;
  }

  byte lowByte  = Wire.read();
  byte highByte = Wire.read();
  Wire.read(); // PEC byte

  uint16_t raw = ((uint16_t)highByte << 8) | lowByte;
  return (raw * 0.02f) - 273.15f;
}

// ---- VL53L1X single distance (cm) ----
float readDistanceCM() {
  unsigned long timeout = millis();
  while (!vl53l1x.dataReady()) {
    if (millis() - timeout > 1000) {
      return -1.0f;
    }
    delay(5);
  }

  int16_t distanceMM = vl53l1x.distance();
  vl53l1x.clearInterrupt();

  if (distanceMM <= 0) {
    return -1.0f;
  }
  return distanceMM / 10.0f;
}
