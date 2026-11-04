#include "weight.h"

// ---- Shared weight sensor object (actual storage) ----
HX711 weightSensor;

// ---- Shared weight sensor health flag (actual storage) ----
bool g_weightReady = false;

// ---- Weight sensor initialisation ----
bool initWeightSensor() {
  Serial.println(F("Initializing HX711 weight sensor..."));

  weightSensor.begin(HX711_DOUT_PIN, HX711_SCK_PIN);

  // Apply persisted calibration
  weightSensor.set_scale(g_weightScaleFactor);
  weightSensor.set_offset(static_cast<long>(g_weightOffset));

  // Quick sanity read: if the sensor responds, mark ready
  long raw = weightSensor.read();
  if (raw == 0) {
    Serial.println(F("HX711 NOT FOUND"));
    g_weightReady = false;
    return false;
  }

  g_weightReady = true;
  Serial.println(F("HX711 READY"));
  return true;
}

// ---- Read weight in kg ----
float readWeightKg(byte samples) {
  if (!g_weightReady) {
    return -999.0f;
  }

  float weight = weightSensor.get_units(samples);

  // Sanity check
  if (weight < MIN_WEIGHT_KG || weight > MAX_WEIGHT_KG) {
    return -999.0f;
  }

  return weight;
}

// ---- Tare / zero the scale ----
bool tareWeight(byte samples) {
  if (!g_weightReady) {
    return false;
  }

  weightSensor.tare(samples);

  // Persist the new offset
  g_weightOffset = static_cast<float>(weightSensor.get_offset());
  g_weightScaleFactor = weightSensor.get_scale();

  return true;
}
