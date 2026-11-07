#ifndef VISTRA_TEST_H
#define VISTRA_TEST_H

#include "config.h"

// ---- Test status enum ----
enum TestStatus {
  TEST_PASS,
  TEST_FAIL,
  TEST_SKIP
};

// ---- Test result struct ----
struct TestResult {
  TestStatus status;
  float      value;
  const char* message;
};

// ---- Pure validation helpers (hardware-independent, unit-testable) ----
static inline bool isValidTofDistance(float distance) {
  if (distance <= 0.0f) {
    return false;
  }
  if (distance < TEST_DISTANCE_MIN_CM || distance > TEST_DISTANCE_MAX_CM) {
    return false;
  }
  return true;
}

static inline bool isValidTempReading(float ambient, float objectTemp) {
  if (ambient == -999.0f || objectTemp == -999.0f) {
    return false;
  }
  if (ambient < TEST_AMBIENT_MIN_C || ambient > TEST_AMBIENT_MAX_C) {
    return false;
  }
  if (objectTemp < TEST_TEMP_MIN_C || objectTemp > TEST_TEMP_MAX_C) {
    return false;
  }
  return true;
}

static inline bool isValidWeightKg(float weightKg) {
  if (weightKg == -999.0f) {
    return false;
  }
  if (weightKg < MIN_WEIGHT_KG || weightKg > MAX_WEIGHT_KG) {
    return false;
  }
  return true;
}

// ---- Hardware-dependent declarations (defined in test.cpp) ----
// These require Arduino.h types (Serial, Wire, HX711, etc.) and are
// implemented in src/test.cpp which includes sensors.h, calibration.h, weight.h.
TestResult testTof();
TestResult testTemp();
TestResult testHeart();
TestResult testWeight();
TestResult testCal();
void handleTestCommand(const char* rest);
void printTestResult(const char* sensor, const TestResult& r);
void printTestSummary(uint8_t passed, uint8_t failed, uint8_t skipped);

#endif // VISTRA_TEST_H
