#include "heart.h"
#include "calibration.h"

// ---- Module-internal sample buffers (800 bytes static RAM) ----
static uint32_t g_irBuffer[HEART_BUFFER_SIZE];
static uint32_t g_redBuffer[HEART_BUFFER_SIZE];

// ---- Internal: acquire one red/IR sample with timeout ----
static bool getHeartSample(uint32_t& red, uint32_t& ir, unsigned long timeoutMs = 1000) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    max30102.check();
    if (max30102.available()) {
      red = max30102.getRed();
      ir = max30102.getIR();
      max30102.nextSample();
      return true;
    }
  }
  return false;
}

// ---- HEART command ----
void measureHeart() {
  if (!g_heartReady) {
    Serial.println(F("{\"type\":\"heart\",\"status\":\"sensor_error\"}"));
    return;
  }

  Serial.println(F("{\"type\":\"heart\",\"status\":\"waiting_for_finger\"}"));

  // Wait for finger
  unsigned long fingerWait = millis();
  while (true) {
    uint32_t ir = max30102.getIR();
    if (ir > FINGER_THRESHOLD) {
      break;
    }
    if (millis() - fingerWait > 15000) {
      Serial.println(F("{\"type\":\"heart\",\"status\":\"no_finger\"}"));
      return;
    }
    delay(50);
  }

  // Stabilize finger
  Serial.println(F("{\"type\":\"heart\",\"status\":\"stabilizing\"}"));
  unsigned long stabilizeTimer = millis();
  while (millis() - stabilizeTimer < 2000) {
    uint32_t ir = max30102.getIR();
    if (ir < FINGER_REMOVED_THRESHOLD) {
      Serial.println(F("{\"type\":\"heart\",\"status\":\"finger_removed\"}"));
      return;
    }
    delay(50);
  }

  max30102.clearFIFO();
  delay(100);

  Serial.println(F("{\"type\":\"heart\",\"status\":\"measuring\"}"));

  // Initial 100 samples
  for (int i = 0; i < HEART_BUFFER_SIZE; i++) {
    uint32_t red;
    uint32_t ir;
    if (!getHeartSample(red, ir)) {
      Serial.println(F("{\"type\":\"heart\",\"status\":\"read_error\"}"));
      return;
    }
    if (ir < FINGER_REMOVED_THRESHOLD) {
      Serial.println(F("{\"type\":\"heart\",\"status\":\"finger_removed\"}"));
      return;
    }
    g_redBuffer[i] = red;
    g_irBuffer[i] = ir;
  }

  int bpmResults[HEART_WINDOWS];
  int spo2Results[HEART_WINDOWS];
  byte validWindows = 0;

  // Five overlapping windows
  for (byte window = 0; window < HEART_WINDOWS; window++) {
    int32_t spo2;
    int8_t  validSPO2;
    int32_t heartRate;
    int8_t  validHeartRate;

    maxim_heart_rate_and_oxygen_saturation(
      g_irBuffer, HEART_BUFFER_SIZE, g_redBuffer,
      &spo2, &validSPO2, &heartRate, &validHeartRate);

    // Diagnostic output
    Serial.print(F("WINDOW "));
    Serial.print(window + 1);
    Serial.print(F(" -> BPM: "));
    Serial.print(heartRate);
    Serial.print(F(" | SpO2: "));
    Serial.print(spo2);
    Serial.print(F(" | HR Valid: "));
    Serial.print(validHeartRate);
    Serial.print(F(" | SpO2 Valid: "));
    Serial.println(validSPO2);

    // Store valid window
    if (validHeartRate && validSPO2 &&
        heartRate >= MIN_BPM && heartRate <= MAX_BPM &&
        spo2 >= MIN_SPO2 && spo2 <= MAX_SPO2) {
      bpmResults[validWindows] = (int)heartRate;
      spo2Results[validWindows] = (int)spo2;
      validWindows++;
    }

    // Shift 75 + collect 25 new
    if (window < HEART_WINDOWS - 1) {
      for (int i = 25; i < HEART_BUFFER_SIZE; i++) {
        g_redBuffer[i - 25] = g_redBuffer[i];
        g_irBuffer[i - 25] = g_irBuffer[i];
      }
      for (int i = 75; i < HEART_BUFFER_SIZE; i++) {
        uint32_t red;
        uint32_t ir;
        if (!getHeartSample(red, ir)) {
          Serial.println(F("{\"type\":\"heart\",\"status\":\"read_error\"}"));
          return;
        }
        if (ir < FINGER_REMOVED_THRESHOLD) {
          Serial.println(F("{\"type\":\"heart\",\"status\":\"finger_removed\"}"));
          return;
        }
        g_redBuffer[i] = red;
        g_irBuffer[i] = ir;
      }
    }
  }

  // Minimum valid windows
  if (validWindows < MIN_VALID_HEART_WINDOWS) {
    Serial.print(F("{\"type\":\"heart\",\"status\":\"insufficient_signal\",\"valid_windows\":"));
    Serial.print(validWindows);
    Serial.println(F("}"));
    return;
  }

  // Median before filtering
  int medianBPM = medianInt(bpmResults, validWindows);
  int medianSPO2 = medianInt(spo2Results, validWindows);

  // Outlier filter
  int filteredBPM[HEART_WINDOWS];
  int filteredSPO2[HEART_WINDOWS];
  byte filteredCount = 0;

  for (byte i = 0; i < validWindows; i++) {
    int bpmDifference = abs(bpmResults[i] - medianBPM);
    int spo2Difference = abs(spo2Results[i] - medianSPO2);
    if (bpmDifference <= BPM_OUTLIER_LIMIT && spo2Difference <= SPO2_OUTLIER_LIMIT) {
      filteredBPM[filteredCount] = bpmResults[i];
      filteredSPO2[filteredCount] = spo2Results[i];
      filteredCount++;
    }
  }

  // Require >= 3 after filtering
  if (filteredCount < MIN_VALID_HEART_WINDOWS) {
    Serial.print(F("{\"type\":\"heart\",\"status\":\"unstable\",\"valid_windows\":"));
    Serial.print(validWindows);
    Serial.print(F(",\"filtered_windows\":"));
    Serial.print(filteredCount);
    Serial.println(F("}"));
    return;
  }

  // Sort final values
  sortIntArray(filteredBPM, filteredCount);
  sortIntArray(filteredSPO2, filteredCount);

  int minBPM = filteredBPM[0];
  int maxBPM = filteredBPM[filteredCount - 1];
  int minSpO2 = filteredSPO2[0];
  int maxSpO2 = filteredSPO2[filteredCount - 1];

  int bpmSpread = maxBPM - minBPM;
  int spo2Spread = maxSpO2 - minSpO2;

  // Final stability
  if (bpmSpread > HEART_MAX_SPREAD_BPM || spo2Spread > SPO2_MAX_SPREAD) {
    Serial.print(F("{\"type\":\"heart\",\"status\":\"unstable\",\"valid_windows\":"));
    Serial.print(validWindows);
    Serial.print(F(",\"filtered_windows\":"));
    Serial.print(filteredCount);
    Serial.print(F(",\"bpm_spread\":"));
    Serial.print(bpmSpread);
    Serial.print(F(",\"spo2_spread\":"));
    Serial.print(spo2Spread);
    Serial.println(F("}"));
    return;
  }

  // Final result
  int finalBPM = medianInt(filteredBPM, filteredCount);
  int finalSPO2 = medianInt(filteredSPO2, filteredCount);

  // Apply heart calibration: calibrated = raw * scale + offset
  int calibratedBPM = static_cast<int>(finalBPM * g_heartCalScale + g_heartCalOffset);
  int calibratedSPO2 = static_cast<int>(finalSPO2 * g_heartCalScale + g_heartCalOffset);

  // Clamp to valid ranges
  if (calibratedBPM < MIN_BPM) calibratedBPM = MIN_BPM;
  if (calibratedBPM > MAX_BPM) calibratedBPM = MAX_BPM;
  if (calibratedSPO2 < MIN_SPO2) calibratedSPO2 = MIN_SPO2;
  if (calibratedSPO2 > MAX_SPO2) calibratedSPO2 = MAX_SPO2;

  finalBPM = calibratedBPM;
  finalSPO2 = calibratedSPO2;

  // Stability label
  const char* stability;
  if (bpmSpread <= 5 && spo2Spread <= 1) {
    stability = "excellent";
  } else if (bpmSpread <= 9 && spo2Spread <= 2) {
    stability = "good";
  } else {
    stability = "acceptable";
  }

  // BPM range
  const char* bpmRange;
  if (finalBPM < 60) {
    bpmRange = "below_typical_resting";
  } else if (finalBPM <= 100) {
    bpmRange = "typical_resting";
  } else {
    bpmRange = "above_typical_resting";
  }

  // SpO2 range
  const char* spo2Range;
  if (finalSPO2 >= 95) {
    spo2Range = "typical";
  } else if (finalSPO2 >= 90) {
    spo2Range = "below_typical";
  } else {
    spo2Range = "low";
  }

  // Heart success JSON
  Serial.print(F("{\"type\":\"heart\",\"status\":\"success\",\"bpm\":"));
  Serial.print(finalBPM);
  Serial.print(F(",\"spo2\":"));
  Serial.print(finalSPO2);
  Serial.print(F(",\"valid_windows\":"));
  Serial.print(validWindows);
  Serial.print(F(",\"accepted_windows\":"));
  Serial.print(filteredCount);
  Serial.print(F(",\"bpm_spread\":"));
  Serial.print(bpmSpread);
  Serial.print(F(",\"spo2_spread\":"));
  Serial.print(spo2Spread);
  Serial.print(F(",\"stability\":\""));
  Serial.print(stability);
  Serial.print(F("\",\"bpm_range\":\""));
  Serial.print(bpmRange);
  Serial.print(F("\",\"spo2_range\":\""));
  Serial.print(spo2Range);
  Serial.println(F("\"}"));
}
