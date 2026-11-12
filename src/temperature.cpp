#include "temperature.h"

// ---- Internal: hand detection ----
bool detectHand(float& ambient, float& objectTemp) {
  ambient = readMLXTemperature(MLX_AMBIENT_REG);
  objectTemp = readMLXTemperature(MLX_OBJECT_REG);
  if (ambient == -999.0f || objectTemp == -999.0f) {
    return false;
  }
  float difference = objectTemp - ambient;
  // Reject fire/extreme heat (delta too large = not a hand)
  if (difference > HAND_MAX_DELTA_C) {
    return false;
  }
  return difference >= HAND_MIN_DELTA_C;
}

// ---- TEMPERATURE command ----
void measureTemperature() {
  if (!g_mlxReady) {
    Serial.println(F("{\"type\":\"temperature\",\"status\":\"sensor_error\"}"));
    return;
  }

  Serial.println(F("{\"type\":\"temperature\",\"status\":\"waiting_for_hand\"}"));

  unsigned long waitStart = millis();
  float ambient = 0.0f;
  float handTemperature = 0.0f;

  // Wait for hand
  while (!detectHand(ambient, handTemperature)) {
    if (millis() - waitStart > HAND_WAIT_TIMEOUT) {
      Serial.println(F("{\"type\":\"temperature\",\"status\":\"no_hand\"}"));
      return;
    }
    delay(100);
  }

  // Hand stabilization
  Serial.println(F("{\"type\":\"temperature\",\"status\":\"stabilizing\"}"));
  unsigned long stabilizeStart = millis();
  while (millis() - stabilizeStart < HAND_STABILIZE_MS) {
    float currentAmbient;
    float currentObject;
    if (!detectHand(currentAmbient, currentObject)) {
      Serial.println(F("{\"type\":\"temperature\",\"status\":\"hand_removed\"}"));
      return;
    }
    delay(100);
  }

  Serial.println(F("{\"type\":\"temperature\",\"status\":\"measuring\"}"));

  // Collect temperature samples
  float temperatureSamples[TEMP_SAMPLES];
  float ambientSamples[TEMP_SAMPLES];

  for (byte i = 0; i < TEMP_SAMPLES; i++) {
    float currentAmbient = readMLXTemperature(MLX_AMBIENT_REG);
    float currentObject = readMLXTemperature(MLX_OBJECT_REG);
    if (currentAmbient == -999.0f || currentObject == -999.0f) {
      Serial.println(F("{\"type\":\"temperature\",\"status\":\"read_error\"}"));
      return;
    }
    float difference = currentObject - currentAmbient;
    if (difference < HAND_MIN_DELTA_C || difference > HAND_MAX_DELTA_C) {
      Serial.println(F("{\"type\":\"temperature\",\"status\":\"hand_removed\"}"));
      return;
    }
    ambientSamples[i] = currentAmbient;
    temperatureSamples[i] = currentObject;
    delay(100);
  }

  // Temperature medians
  float rawTemperature = medianFloat(temperatureSamples, TEMP_SAMPLES);
  float medianAmbient = medianFloat(ambientSamples, TEMP_SAMPLES);

  // Temperature spread
  float sortedTemps[TEMP_SAMPLES];
  for (byte i = 0; i < TEMP_SAMPLES; i++) {
    sortedTemps[i] = temperatureSamples[i];
  }
  sortFloatArray(sortedTemps, TEMP_SAMPLES);
  float spread = sortedTemps[TEMP_SAMPLES - 1] - sortedTemps[0];

  // Temperature stability check
  if (spread > TEMP_MAX_SPREAD_C) {
    Serial.print(F("{\"type\":\"temperature\",\"status\":\"unstable\",\"spread\":"));
    Serial.print(spread, 2);
    Serial.println(F("}"));
    return;
  }

  // Temperature calibration
  float calibratedTemperature = (rawTemperature * g_tempCalScale) + g_tempCalOffset;

  // Stability label
  const char* stability;
  if (spread <= 0.15f) {
    stability = "excellent";
  } else if (spread <= 0.40f) {
    stability = "good";
  } else {
    stability = "acceptable";
  }

  // Temperature success
  Serial.print(F("{\"type\":\"temperature\",\"status\":\"success\",\"measurement\":\"hand_skin\",\"ambient\":"));
  Serial.print(medianAmbient, 2);
  Serial.print(F(",\"raw_temperature\":"));
  Serial.print(rawTemperature, 2);
  Serial.print(F(",\"temperature\":"));
  Serial.print(calibratedTemperature, 2);
  Serial.print(F(",\"samples\":"));
  Serial.print(TEMP_SAMPLES);
  Serial.print(F(",\"spread\":"));
  Serial.print(spread, 2);
  Serial.print(F(",\"stability\":\""));
  Serial.print(stability);
  Serial.println(F("\"}"));
}
