#include "test.h"
#include "sensors.h"
#include "calibration.h"
#include "weight.h"

// ---- JSON printing helpers ----

void printTestResult(const char* sensor, const TestResult& r) {
  Serial.print(F("{\"type\":\"test\",\"sensor\":\""));
  Serial.print(sensor);
  Serial.print(F("\",\"status\":\""));

  switch (r.status) {
    case TEST_PASS:  Serial.print(F("pass")); break;
    case TEST_FAIL:  Serial.print(F("fail")); break;
    case TEST_SKIP:  Serial.print(F("skip")); break;
  }

  Serial.print(F("\",\"value\":"));

  if (r.status == TEST_SKIP && r.value == -1.0f) {
    Serial.print(F("-1"));
  } else if (r.status == TEST_FAIL && r.value == -999.0f) {
    Serial.print(F("-999"));
  } else {
    Serial.print(r.value, 2);
  }

  Serial.print(F(",\"message\":\""));
  Serial.print(r.message);
  Serial.println(F("\"}"));
}

void printTestSummary(uint8_t passed, uint8_t failed, uint8_t skipped) {
  Serial.print(F("{\"type\":\"test\",\"sensor\":\"all\",\"status\":\""));

  if (failed > 0) {
    Serial.print(F("fail"));
  } else if (skipped > 0) {
    Serial.print(F("partial"));
  } else {
    Serial.print(F("pass"));
  }

  Serial.print(F("\",\"value\":"));
  Serial.print((int)passed);
  Serial.print(F(",\"message\":\""));
  Serial.print((int)passed);
  Serial.print(F(" passed, "));
  Serial.print((int)failed);
  Serial.print(F(" failed, "));
  Serial.print((int)skipped);
  Serial.print(F(" skipped"));
  Serial.println(F("\"}"));
}

// ---- JSON printing helpers ----

TestResult testTof() {
  if (!g_tofReady) {
    return {TEST_SKIP, -1.0f, "tof sensor not ready"};
  }

  float distance = readDistanceCM();  // bounded 1000ms internally

  if (!isValidTofDistance(distance)) {
    return {TEST_FAIL, distance, "tof read failed or out of range"};
  }

  char msg[64];
  snprintf(msg, sizeof(msg), "tof distance %.1f cm", distance);
  // snprintf writes into stack buffer; message pointer must remain valid.
  // Use a static buffer for the message string.
  static char msgBuf[64];
  snprintf(msgBuf, sizeof(msgBuf), "tof distance %.1f cm", distance);
  return {TEST_PASS, distance, msgBuf};
}

TestResult testTemp() {
  if (!g_mlxReady) {
    return {TEST_SKIP, -999.0f, "temp sensor not ready"};
  }

  float ambient = readMLXTemperature(MLX_AMBIENT_REG);
  float objectTemp = readMLXTemperature(MLX_OBJECT_REG);

  if (ambient == -999.0f || objectTemp == -999.0f) {
    return {TEST_FAIL, -999.0f, "temp read failed"};
  }

  if (!isValidTempReading(ambient, objectTemp)) {
    return {TEST_FAIL, objectTemp, "temp out of valid range"};
  }

  static char msgBuf[64];
  snprintf(msgBuf, sizeof(msgBuf), "temp ambient %.1f C object %.1f C", ambient, objectTemp);
  return {TEST_PASS, objectTemp, msgBuf};
}

TestResult testHeart() {
  if (!g_heartReady) {
    return {TEST_SKIP, 0.0f, "heart sensor not ready"};
  }

  // Bounded finger detection: max TEST_HEART_TIMEOUT_MS
  unsigned long start = millis();
  uint32_t ir = 0;

  while (millis() - start < TEST_HEART_TIMEOUT_MS) {
    max30102.check();
    if (max30102.available()) {
      ir = max30102.getIR();
      max30102.nextSample();
    }
    if (ir > FINGER_THRESHOLD) {
      break;
    }
    delay(10);
  }

  if (ir <= FINGER_THRESHOLD) {
    return {TEST_SKIP, (float)ir, "heart no finger within 5000ms"};
  }

  static char msgBuf[48];
  snprintf(msgBuf, sizeof(msgBuf), "heart finger detected IR=%lu", (unsigned long)ir);
  return {TEST_PASS, (float)ir, msgBuf};
}

TestResult testWeight() {
  if (!g_weightReady) {
    return {TEST_SKIP, -999.0f, "weight sensor not ready"};
  }

  long raw = weightSensor.read();

  if (raw <= 0) {
    return {TEST_FAIL, (float)raw, "weight sensor not connected"};
  }

  float weightKg = readWeightKg(WEIGHT_SAMPLES);

  if (!isValidWeightKg(weightKg)) {
    return {TEST_FAIL, (float)raw, "weight out of range"};
  }

  static char msgBuf[48];
  snprintf(msgBuf, sizeof(msgBuf), "weight %.2f kg raw %ld", weightKg, raw);
  return {TEST_PASS, weightKg, msgBuf};
}

TestResult testCal() {
  if (!calibrationIsValid()) {
    return {TEST_FAIL, 0.0f, "cal EEPROM calibration invalid"};
  }

  static char msgBuf[80];
  snprintf(msgBuf, sizeof(msgBuf),
           "cal EEPROM valid v2 floor=%.2f temp_scale=%.3f weight_scale=%.4f",
           g_sensorToFloorCm, g_tempCalScale, g_weightScaleFactor);
  return {TEST_PASS, (float)kCalVersion, msgBuf};
}

// ---- Command dispatcher ----

void handleTestCommand(const char* rest) {
  if (!rest || *rest == '\0') {
    Serial.println(F("{\"type\":\"test\",\"status\":\"error\",\"message\":\"missing subcommand\"}"));
    return;
  }

  // Skip leading spaces
  while (*rest == ' ') {
    rest++;
  }

  // Extract subcommand (first word)
  static char sub[16];
  int i = 0;
  while (*rest && *rest != ' ' && i < 15) {
    sub[i++] = static_cast<char>(*rest++);
  }
  sub[i] = '\0';

  // Skip to arguments
  while (*rest == ' ') {
    rest++;
  }

  if (strcmp(sub, "ALL") == 0) {
    // Run all five tests sequentially, print each result, then summary
    uint8_t passed = 0, failed = 0, skipped = 0;

    TestResult r = testTof();
    printTestResult("tof", r);
    if (r.status == TEST_PASS) passed++;
    else if (r.status == TEST_FAIL) failed++;
    else skipped++;

    r = testTemp();
    printTestResult("temp", r);
    if (r.status == TEST_PASS) passed++;
    else if (r.status == TEST_FAIL) failed++;
    else skipped++;

    r = testHeart();
    printTestResult("heart", r);
    if (r.status == TEST_PASS) passed++;
    else if (r.status == TEST_FAIL) failed++;
    else skipped++;

    r = testWeight();
    printTestResult("weight", r);
    if (r.status == TEST_PASS) passed++;
    else if (r.status == TEST_FAIL) failed++;
    else skipped++;

    r = testCal();
    printTestResult("cal", r);
    if (r.status == TEST_PASS) passed++;
    else if (r.status == TEST_FAIL) failed++;
    else skipped++;

    printTestSummary(passed, failed, skipped);

  } else if (strcmp(sub, "TOF") == 0) {
    printTestResult("tof", testTof());
  } else if (strcmp(sub, "TEMP") == 0) {
    printTestResult("temp", testTemp());
  } else if (strcmp(sub, "HEART") == 0) {
    printTestResult("heart", testHeart());
  } else if (strcmp(sub, "WEIGHT") == 0) {
    printTestResult("weight", testWeight());
  } else if (strcmp(sub, "CAL") == 0) {
    printTestResult("cal", testCal());
  } else {
    Serial.print(F("{\"type\":\"test\",\"status\":\"error\",\"message\":\"unknown subcommand: '"));
    Serial.print(sub);
    Serial.println(F("'}"));
  }
}
