#include "height.h"

// ---- Internal: person-under-sensor detection ----
static bool personUnderHeightSensor() {
  float distance = readDistanceCM();
  if (distance <= 0) {
    return false;
  }
  float calculatedHeight = g_sensorToFloorCm - distance;
  return (calculatedHeight >= MIN_PERSON_HEIGHT_CM &&
          calculatedHeight <= MAX_PERSON_HEIGHT_CM);
}

// ---- HEIGHT command ----
void measureHeight() {
  if (!g_tofReady) {
    Serial.println(F("{\"type\":\"height\",\"status\":\"sensor_error\"}"));
    return;
  }

  Serial.println(F("{\"type\":\"height\",\"status\":\"waiting_for_person\"}"));

  unsigned long startWait = millis();
  while (!personUnderHeightSensor()) {
    if (millis() - startWait > HEIGHT_WAIT_TIMEOUT) {
      Serial.println(F("{\"type\":\"height\",\"status\":\"no_person\"}"));
      return;
    }
    delay(100);
  }

  Serial.println(F("{\"type\":\"height\",\"status\":\"measuring\"}"));

  for (byte attempt = 1; attempt <= HEIGHT_MAX_ATTEMPTS; attempt++) {
    float samples[HEIGHT_SAMPLES];
    bool failed = false;

    for (byte i = 0; i < HEIGHT_SAMPLES; i++) {
      float distance = readDistanceCM();
      if (distance <= 0) {
        failed = true;
        break;
      }
      samples[i] = distance;
      delay(70);
    }

    if (failed) {
      Serial.println(F("{\"type\":\"height\",\"status\":\"read_error\"}"));
      return;
    }

    float sortedSamples[HEIGHT_SAMPLES];
    for (byte i = 0; i < HEIGHT_SAMPLES; i++) {
      sortedSamples[i] = samples[i];
    }
    sortFloatArray(sortedSamples, HEIGHT_SAMPLES);

    float minDistance = sortedSamples[0];
    float maxDistance = sortedSamples[HEIGHT_SAMPLES - 1];
    float spread = maxDistance - minDistance;
    float medianDistance = medianFloat(samples, HEIGHT_SAMPLES);
    float height = g_sensorToFloorCm - medianDistance;

    // Height range check
    if (height < MIN_PERSON_HEIGHT_CM || height > MAX_PERSON_HEIGHT_CM) {
      Serial.println(F("{\"type\":\"height\",\"status\":\"invalid_position\"}"));
      return;
    }

    // Height stability
    if (spread > HEIGHT_MAX_SPREAD_CM) {
      if (attempt < HEIGHT_MAX_ATTEMPTS) {
        Serial.print(F("{\"type\":\"height\",\"status\":\"retrying\",\"attempt\":"));
        Serial.print(attempt);
        Serial.print(F(",\"spread_cm\":"));
        Serial.print(spread, 1);
        Serial.println(F("}"));
        delay(500);
        continue;
      }
      Serial.print(F("{\"type\":\"height\",\"status\":\"unstable\",\"spread_cm\":"));
      Serial.print(spread, 1);
      Serial.println(F("}"));
      return;
    }

    // Stability label
    const char* stability;
    if (spread <= 0.5f) {
      stability = "excellent";
    } else if (spread <= 1.0f) {
      stability = "good";
    } else {
      stability = "acceptable";
    }

    // Height success
    Serial.print(F("{\"type\":\"height\",\"status\":\"success\",\"distance_cm\":"));
    Serial.print(medianDistance, 1);
    Serial.print(F(",\"height_cm\":"));
    Serial.print(height, 1);
    Serial.print(F(",\"samples\":"));
    Serial.print(HEIGHT_SAMPLES);
    Serial.print(F(",\"spread_cm\":"));
    Serial.print(spread, 1);
    Serial.print(F(",\"stability\":\""));
    Serial.print(stability);
    Serial.println(F("\"}"));
    return;
  }
}
