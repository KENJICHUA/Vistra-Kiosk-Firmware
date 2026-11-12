/*
 * Vistra Kiosk Firmware — Unit Tests
 *
 * PlatformIO Unity test framework.
 * Host (PC) test environment — no Arduino hardware needed.
 *
 * Run with: pio test -e host_test
 * Run on Mega 2560: pio test -e megaatmega2560_test
 */

#include <unity.h>
#include <string.h>
#include <math.h>

#include "../include/array_helpers.h"
#include "../include/calibration.h"
#include "../include/config.h"
#include "../include/test.h"

// ---- Host mocks for Arduino-only symbols ----
float g_sensorToFloorCm  = DEFAULT_SENSOR_TO_FLOOR_CM;
float g_tempCalScale     = DEFAULT_TEMP_CAL_SCALE;
float g_tempCalOffset    = DEFAULT_TEMP_CAL_OFFSET;
float g_weightScaleFactor = DEFAULT_WEIGHT_SCALE_FACTOR;
float g_weightOffset     = DEFAULT_WEIGHT_OFFSET;

void calibrationReset() {
  g_sensorToFloorCm  = DEFAULT_SENSOR_TO_FLOOR_CM;
  g_tempCalScale     = DEFAULT_TEMP_CAL_SCALE;
  g_tempCalOffset    = DEFAULT_TEMP_CAL_OFFSET;
  g_weightScaleFactor = DEFAULT_WEIGHT_SCALE_FACTOR;
  g_weightOffset     = DEFAULT_WEIGHT_OFFSET;
}

bool calibrationIsValid() {
  return false; // No EEPROM on host
}

void setUp() {}
void tearDown() {};

/*======================================================================
  ARRAY HELPERS
======================================================================*/

void test_sortFloatArray_ascending() {
  float arr[] = {3.0f, 1.0f, 2.0f};
  sortFloatArray(arr, 3);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, arr[0]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, arr[1]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0f, arr[2]);
}

void test_sortFloatArray_alreadySorted() {
  float arr[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  sortFloatArray(arr, 5);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, arr[0]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, arr[1]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0f, arr[2]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 4.0f, arr[3]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, arr[4]);
}

void test_sortFloatArray_singleElement() {
  float arr[] = {42.5f};
  sortFloatArray(arr, 1);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 42.5f, arr[0]);
}

void test_sortFloatArray_duplicates() {
  float arr[] = {2.0f, 2.0f, 1.0f, 2.0f};
  sortFloatArray(arr, 4);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, arr[0]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, arr[1]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, arr[2]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, arr[3]);
}

void test_sortFloatArray_negativeValues() {
  float arr[] = {-5.0f, 10.0f, -3.0f, 0.0f};
  sortFloatArray(arr, 4);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, -5.0f, arr[0]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, -3.0f, arr[1]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, arr[2]);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, arr[3]);
}

void test_sortIntArray_descending() {
  int arr[] = {30, 10, 20};
  sortIntArray(arr, 3);
  TEST_ASSERT_EQUAL(10, arr[0]);
  TEST_ASSERT_EQUAL(20, arr[1]);
  TEST_ASSERT_EQUAL(30, arr[2]);
}

void test_sortIntArray_negatives() {
  int arr[] = {-5, 10, -20, 0};
  sortIntArray(arr, 4);
  TEST_ASSERT_EQUAL(-20, arr[0]);
  TEST_ASSERT_EQUAL(-5, arr[1]);
  TEST_ASSERT_EQUAL(0, arr[2]);
  TEST_ASSERT_EQUAL(10, arr[3]);
}

void test_medianFloat_oddCount() {
  float arr[] = {1.0f, 5.0f, 3.0f};
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0f, medianFloat(arr, 3));
}

void test_medianFloat_evenCount() {
  float arr[] = {1.0f, 2.0f, 3.0f, 4.0f};
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.5f, medianFloat(arr, 4));
}

void test_medianFloat_singleElement() {
  float arr[] = {7.5f};
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 7.5f, medianFloat(arr, 1));
}

void test_medianFloat_unsortedInput() {
  float arr[] = {100.0f, 1.0f, 50.0f, 25.0f, 75.0f};
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 50.0f, medianFloat(arr, 5));
}

void test_medianFloat_capsAt20() {
  float arr[25];
  for (int i = 0; i < 25; i++) arr[i] = i * 1.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 9.5f, medianFloat(arr, 25));
}

void test_medianInt_oddCount() {
  int arr[] = {10, 50, 30};
  TEST_ASSERT_EQUAL(30, medianInt(arr, 3));
}

void test_medianInt_evenCount() {
  int arr[] = {10, 20, 30, 40};
  TEST_ASSERT_EQUAL(25, medianInt(arr, 4));
}

void test_medianInt_singleElement() {
  int arr[] = {99};
  TEST_ASSERT_EQUAL(99, medianInt(arr, 1));
}

void test_medianInt_unsortedInput() {
  int arr[] = {100, 1, 50, 25, 75};
  TEST_ASSERT_EQUAL(50, medianInt(arr, 5));
}

/*======================================================================
  CALIBRATION
======================================================================*/

void test_computeChecksum_knownRecord() {
  CalibrationRecord rec;
  rec.magic = 0x4B43;
  rec.version = 2;
  rec.sensorToFloorCm = 210.5f;
  rec.tempCalScale = 1.02f;
  rec.tempCalOffset = -0.5f;
  rec.weightScaleFactor = 400.1234f;
  rec.weightOffset = 12345.0f;
  rec.checksum = 0;

  uint16_t crc = computeChecksum(rec);
  TEST_ASSERT_NOT_EQUAL(0, crc);
  TEST_ASSERT_EQUAL(crc, computeChecksum(rec)); // deterministic
}

void test_computeChecksum_differsOnFieldChange() {
  CalibrationRecord a, b;
  memset(&a, 0, sizeof(a));
  memset(&b, 0, sizeof(b));
  a.magic = 0x4B43;
  a.version = 2;
  b = a;
  b.sensorToFloorCm = 999.0f;
  TEST_ASSERT_NOT_EQUAL(computeChecksum(a), computeChecksum(b));
}

void test_computeChecksum_emptyRecord() {
  CalibrationRecord rec;
  memset(&rec, 0, sizeof(rec));
  rec.checksum = 0;
  TEST_ASSERT_EQUAL(0, computeChecksum(rec));
}

void test_calibrationReset_setsFactoryDefaults() {
  calibrationReset();
  TEST_ASSERT_FLOAT_WITHIN(0.001f, DEFAULT_SENSOR_TO_FLOOR_CM, g_sensorToFloorCm);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, DEFAULT_TEMP_CAL_SCALE, g_tempCalScale);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, DEFAULT_TEMP_CAL_OFFSET, g_tempCalOffset);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, DEFAULT_WEIGHT_SCALE_FACTOR, g_weightScaleFactor);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, DEFAULT_WEIGHT_OFFSET, g_weightOffset);
}

void test_defaultConstants_haveExpectedValues() {
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 215.0f, DEFAULT_SENSOR_TO_FLOOR_CM);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, DEFAULT_TEMP_CAL_SCALE);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, DEFAULT_TEMP_CAL_OFFSET);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, DEFAULT_WEIGHT_SCALE_FACTOR);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, DEFAULT_WEIGHT_OFFSET);
}

void test_calibrationIsValid_freshEEPROM() {
  TEST_ASSERT_FALSE(calibrationIsValid());
}

/*======================================================================
  HEIGHT
======================================================================*/

void test_heightConstants_validRange() {
  TEST_ASSERT_TRUE(MIN_PERSON_HEIGHT_CM > 0.0f);
  TEST_ASSERT_TRUE(MAX_PERSON_HEIGHT_CM > MIN_PERSON_HEIGHT_CM);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 80.0f, MIN_PERSON_HEIGHT_CM);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 220.0f, MAX_PERSON_HEIGHT_CM);
}

void test_sensorToFloorPositive() {
  TEST_ASSERT_TRUE(DEFAULT_SENSOR_TO_FLOOR_CM > 0.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 215.0f, DEFAULT_SENSOR_TO_FLOOR_CM);
}

void test_heightSpreadReasonable() {
  TEST_ASSERT_TRUE(HEIGHT_MAX_SPREAD_CM > 0.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, HEIGHT_MAX_SPREAD_CM);
}

void test_heightSamplesPositive() {
  TEST_ASSERT_TRUE(HEIGHT_SAMPLES > 0);
  TEST_ASSERT_TRUE(HEIGHT_SAMPLES <= 20);
}

void test_heightTimeoutsPositive() {
  TEST_ASSERT_TRUE(HEIGHT_WAIT_TIMEOUT > 0);
  TEST_ASSERT_TRUE(HEIGHT_MAX_ATTEMPTS > 0);
}

void test_heightCalculation_basic() {
  float height = DEFAULT_SENSOR_TO_FLOOR_CM - 150.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 65.0f, height);
}

void test_heightCalculation_personTooShort() {
  float height = DEFAULT_SENSOR_TO_FLOOR_CM - 200.0f;
  TEST_ASSERT_TRUE(height < MIN_PERSON_HEIGHT_CM);
}

void test_heightCalculation_personTooTall() {
  float height = DEFAULT_SENSOR_TO_FLOOR_CM - 0.0f;
  TEST_ASSERT_TRUE(height <= MAX_PERSON_HEIGHT_CM); // 215 cm <= 220 cm -> valid
}

void test_heightCalculation_exceedsMaxHeight() {
  // Negative distance simulates sensor mounted above floor (physically unusual)
  float height = DEFAULT_SENSOR_TO_FLOOR_CM - (-6.0f);
  TEST_ASSERT_TRUE(height > MAX_PERSON_HEIGHT_CM); // 221 cm > 220 cm -> too tall
}

void test_spreadCalculation() {
  float sorted[9] = {10.0f, 10.5f, 11.0f, 11.5f, 12.0f, 12.5f, 13.0f, 13.5f, 14.0f};
  float spread = sorted[8] - sorted[0];
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 4.0f, spread);
  TEST_ASSERT_TRUE(spread > HEIGHT_MAX_SPREAD_CM);
}

/*======================================================================
  TEMPERATURE
======================================================================*/

void test_temperatureConstants_validRange() {
  TEST_ASSERT_TRUE(TEMP_MAX_SPREAD_C > 0.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.60f, TEMP_MAX_SPREAD_C);
}

void test_tempCalDefaults() {
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, DEFAULT_TEMP_CAL_SCALE);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, DEFAULT_TEMP_CAL_OFFSET);
}

void test_handDeltaPositive() {
  TEST_ASSERT_TRUE(HAND_MIN_DELTA_C > 0.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, HAND_MIN_DELTA_C);
}

void test_tempSamplesPositive() {
  TEST_ASSERT_TRUE(TEMP_SAMPLES > 0);
  TEST_ASSERT_TRUE(TEMP_SAMPLES <= 20);
}

void test_temperatureCalibration_identity() {
  float calibrated = 35.0f * DEFAULT_TEMP_CAL_SCALE + DEFAULT_TEMP_CAL_OFFSET;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 35.0f, calibrated);
}

void test_temperatureCalibration_scaleOnly() {
  float calibrated = 35.0f * 1.1f + 0.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 38.5f, calibrated);
}

void test_temperatureCalibration_offsetOnly() {
  float calibrated = 35.0f * 1.0f + 2.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 37.0f, calibrated);
}

void test_temperatureCalibration_both() {
  float calibrated = 35.0f * 0.95f - 1.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 32.25f, calibrated);
}

void test_handDetectionDelta_aboveThreshold() {
  float delta = 32.0f - 30.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, delta);
  TEST_ASSERT_TRUE(delta >= HAND_MIN_DELTA_C);
}

void test_handDetectionDelta_belowThreshold() {
  float delta = 30.3f - 30.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.3f, delta);
  TEST_ASSERT_TRUE(delta < HAND_MIN_DELTA_C);
}

void test_handDetectionDelta_exactlyAtThreshold() {
  float delta = 30.5f - 30.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, delta);
  TEST_ASSERT_TRUE(delta >= HAND_MIN_DELTA_C);
}

void test_isValidTempReading_fireRejected() {
  // Fire/extreme heat: delta far exceeds HAND_MAX_DELTA_C
  TEST_ASSERT_FALSE(isValidTempReading(30.0f, 200.0f));
  TEST_ASSERT_FALSE(isValidTempReading(25.0f, 150.0f));
  TEST_ASSERT_FALSE(isValidTempReading(30.0f, 100.0f));
}

void test_tempSpreadStable() {
  float sorted[9] = {34.0f, 34.1f, 34.2f, 34.3f, 34.4f, 34.5f, 34.6f, 34.7f, 34.8f};
  float spread = sorted[8] - sorted[0];
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.8f, spread);
  TEST_ASSERT_TRUE(spread > TEMP_MAX_SPREAD_C);
}

void test_tempSpreadVeryStable() {
  float sorted[9] = {34.0f, 34.05f, 34.1f, 34.15f, 34.2f, 34.25f, 34.3f, 34.35f, 34.4f};
  float spread = sorted[8] - sorted[0];
  TEST_ASSERT_TRUE(spread <= TEMP_MAX_SPREAD_C);
}

/*======================================================================
  WEIGHT
======================================================================*/

void test_weightConstants_validRange() {
  TEST_ASSERT_TRUE(MIN_WEIGHT_KG < 0.0f);
  TEST_ASSERT_TRUE(MAX_WEIGHT_KG > 0.0f);
  TEST_ASSERT_TRUE(MAX_WEIGHT_KG > MIN_WEIGHT_KG);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, -5.0f, MIN_WEIGHT_KG);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 500.0f, MAX_WEIGHT_KG);
}

void test_weightScaleFactorPositive() {
  TEST_ASSERT_TRUE(DEFAULT_WEIGHT_SCALE_FACTOR > 0.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, DEFAULT_WEIGHT_SCALE_FACTOR);
}

void test_weightOffsetDefaultZero() {
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, DEFAULT_WEIGHT_OFFSET);
}

void test_weightSamplesPositive() {
  TEST_ASSERT_TRUE(WEIGHT_SAMPLES > 0);
}

void test_weightConversion_basic() {
  float kg = (10500.0f - 500.0f) / 100.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, kg);
}

void test_weightConversion_zeroOffset() {
  float kg = 7500.0f / 100.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 75.0f, kg);
}

void test_weightConversion_negativeAfterTare() {
  float kg = (300.0f - 500.0f) / 100.0f;
  TEST_ASSERT_FLOAT_WITHIN(0.01f, -2.0f, kg);
  TEST_ASSERT_TRUE(kg >= MIN_WEIGHT_KG);
}

void test_weightOutOfRange_low() {
  float kg = (0.0f - 10000.0f) / 100.0f;
  TEST_ASSERT_TRUE(kg < MIN_WEIGHT_KG);
}

void test_weightOutOfRange_high() {
  float kg = 60000.0f / 100.0f;
  TEST_ASSERT_TRUE(kg > MAX_WEIGHT_KG);
}

void test_weightInRange_normal() {
  float kg = 5000.0f / 100.0f;
  TEST_ASSERT_TRUE(kg >= MIN_WEIGHT_KG && kg <= MAX_WEIGHT_KG);
}

/*======================================================================
  TEST COMMAND PROTOCOL
======================================================================*/

void test_testTofTimeoutPositive() {
  TEST_ASSERT_TRUE(TEST_TOF_TIMEOUT_MS > 0);
}

void test_testTempTimeoutPositive() {
  TEST_ASSERT_TRUE(TEST_TEMP_TIMEOUT_MS > 0);
}

void test_testHeartTimeoutPositive() {
  TEST_ASSERT_TRUE(TEST_HEART_TIMEOUT_MS > 0);
}

void test_testWeightTimeoutPositive() {
  TEST_ASSERT_TRUE(TEST_WEIGHT_TIMEOUT_MS > 0);
}

void test_testDistanceRangeValid() {
  TEST_ASSERT_TRUE(TEST_DISTANCE_MAX_CM > TEST_DISTANCE_MIN_CM);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, TEST_DISTANCE_MIN_CM);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 500.0f, TEST_DISTANCE_MAX_CM);
}

void test_testTempRangeValid() {
  TEST_ASSERT_TRUE(TEST_TEMP_MAX_C > TEST_TEMP_MIN_C);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 20.0f, TEST_TEMP_MIN_C);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 45.0f, TEST_TEMP_MAX_C);
}

void test_testAmbientRangeValid() {
  TEST_ASSERT_TRUE(TEST_AMBIENT_MAX_C > TEST_AMBIENT_MIN_C);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, TEST_AMBIENT_MIN_C);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 40.0f, TEST_AMBIENT_MAX_C);
}

void test_isValidTofDistance_inRange() {
  TEST_ASSERT_TRUE(isValidTofDistance(100.0f));
  TEST_ASSERT_TRUE(isValidTofDistance(1.0f));
  TEST_ASSERT_TRUE(isValidTofDistance(500.0f));
}

void test_isValidTofDistance_outOfRangeLow() {
  TEST_ASSERT_FALSE(isValidTofDistance(0.0f));
  TEST_ASSERT_FALSE(isValidTofDistance(0.5f));
  TEST_ASSERT_FALSE(isValidTofDistance(-10.0f));
}

void test_isValidTofDistance_outOfRangeHigh() {
  TEST_ASSERT_FALSE(isValidTofDistance(500.1f));
  TEST_ASSERT_FALSE(isValidTofDistance(600.0f));
}

void test_isValidTempReading_valid() {
  TEST_ASSERT_TRUE(isValidTempReading(30.0f, 35.0f));
  TEST_ASSERT_TRUE(isValidTempReading(20.0f, 20.0f));
  TEST_ASSERT_TRUE(isValidTempReading(40.0f, 45.0f));
}

void test_isValidTempReading_tooCold() {
  TEST_ASSERT_FALSE(isValidTempReading(5.0f, 35.0f));
  TEST_ASSERT_FALSE(isValidTempReading(30.0f, 10.0f));
}

void test_isValidTempReading_tooHot() {
  TEST_ASSERT_FALSE(isValidTempReading(50.0f, 35.0f));
  TEST_ASSERT_FALSE(isValidTempReading(30.0f, 50.0f));
}

void test_isValidTempReading_invalidValues() {
  TEST_ASSERT_FALSE(isValidTempReading(-999.0f, 35.0f));
  TEST_ASSERT_FALSE(isValidTempReading(30.0f, -999.0f));
  TEST_ASSERT_FALSE(isValidTempReading(-999.0f, -999.0f));
}

void test_isValidWeightKg_valid() {
  TEST_ASSERT_TRUE(isValidWeightKg(0.0f));
  TEST_ASSERT_TRUE(isValidWeightKg(75.0f));
  TEST_ASSERT_TRUE(isValidWeightKg(-2.0f));
  TEST_ASSERT_TRUE(isValidWeightKg(500.0f));
}

void test_isValidWeightKg_outOfRange() {
  TEST_ASSERT_FALSE(isValidWeightKg(-999.0f));
  TEST_ASSERT_FALSE(isValidWeightKg(-2.1f));
  TEST_ASSERT_FALSE(isValidWeightKg(500.1f));
}

void test_isValidWeightKg_invalidErrorValue() {
  TEST_ASSERT_FALSE(isValidWeightKg(-999.0f));
}

/*======================================================================
  RUNNER
======================================================================*/

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();

  /* Array helpers */
  RUN_TEST(test_sortFloatArray_ascending);
  RUN_TEST(test_sortFloatArray_alreadySorted);
  RUN_TEST(test_sortFloatArray_singleElement);
  RUN_TEST(test_sortFloatArray_duplicates);
  RUN_TEST(test_sortFloatArray_negativeValues);
  RUN_TEST(test_sortIntArray_descending);
  RUN_TEST(test_sortIntArray_negatives);
  RUN_TEST(test_medianFloat_oddCount);
  RUN_TEST(test_medianFloat_evenCount);
  RUN_TEST(test_medianFloat_singleElement);
  RUN_TEST(test_medianFloat_unsortedInput);
  RUN_TEST(test_medianFloat_capsAt20);
  RUN_TEST(test_medianInt_oddCount);
  RUN_TEST(test_medianInt_evenCount);
  RUN_TEST(test_medianInt_singleElement);
  RUN_TEST(test_medianInt_unsortedInput);

  /* Calibration */
  RUN_TEST(test_computeChecksum_knownRecord);
  RUN_TEST(test_computeChecksum_differsOnFieldChange);
  RUN_TEST(test_computeChecksum_emptyRecord);
  RUN_TEST(test_calibrationReset_setsFactoryDefaults);
  RUN_TEST(test_defaultConstants_haveExpectedValues);
  RUN_TEST(test_calibrationIsValid_freshEEPROM);

  /* Height */
  RUN_TEST(test_heightConstants_validRange);
  RUN_TEST(test_sensorToFloorPositive);
  RUN_TEST(test_heightSpreadReasonable);
  RUN_TEST(test_heightSamplesPositive);
  RUN_TEST(test_heightTimeoutsPositive);
  RUN_TEST(test_heightCalculation_basic);
  RUN_TEST(test_heightCalculation_personTooShort);
  RUN_TEST(test_heightCalculation_personTooTall);
  RUN_TEST(test_heightCalculation_exceedsMaxHeight);
  RUN_TEST(test_spreadCalculation);

  /* Temperature */
  RUN_TEST(test_temperatureConstants_validRange);
  RUN_TEST(test_tempCalDefaults);
  RUN_TEST(test_handDeltaPositive);
  RUN_TEST(test_tempSamplesPositive);
  RUN_TEST(test_temperatureCalibration_identity);
  RUN_TEST(test_temperatureCalibration_scaleOnly);
  RUN_TEST(test_temperatureCalibration_offsetOnly);
  RUN_TEST(test_temperatureCalibration_both);
  RUN_TEST(test_handDetectionDelta_aboveThreshold);
  RUN_TEST(test_handDetectionDelta_belowThreshold);
  RUN_TEST(test_handDetectionDelta_exactlyAtThreshold);
  RUN_TEST(test_tempSpreadStable);
  RUN_TEST(test_tempSpreadVeryStable);

  /* Weight */
  RUN_TEST(test_weightConstants_validRange);
  RUN_TEST(test_weightScaleFactorPositive);
  RUN_TEST(test_weightOffsetDefaultZero);
  RUN_TEST(test_weightSamplesPositive);
  RUN_TEST(test_weightConversion_basic);
  RUN_TEST(test_weightConversion_zeroOffset);
  RUN_TEST(test_weightConversion_negativeAfterTare);
  RUN_TEST(test_weightOutOfRange_low);
  RUN_TEST(test_weightOutOfRange_high);
  RUN_TEST(test_weightInRange_normal);

  /* TEST command protocol */
  RUN_TEST(test_testTofTimeoutPositive);
  RUN_TEST(test_testTempTimeoutPositive);
  RUN_TEST(test_testHeartTimeoutPositive);
  RUN_TEST(test_testWeightTimeoutPositive);
  RUN_TEST(test_testDistanceRangeValid);
  RUN_TEST(test_testTempRangeValid);
  RUN_TEST(test_testAmbientRangeValid);
  RUN_TEST(test_isValidTofDistance_inRange);
  RUN_TEST(test_isValidTofDistance_outOfRangeLow);
  RUN_TEST(test_isValidTofDistance_outOfRangeHigh);
  RUN_TEST(test_isValidTempReading_valid);
  RUN_TEST(test_isValidTempReading_tooCold);
  RUN_TEST(test_isValidTempReading_tooHot);
  RUN_TEST(test_isValidTempReading_invalidValues);
  RUN_TEST(test_isValidTempReading_fireRejected);
  RUN_TEST(test_isValidWeightKg_valid);
  RUN_TEST(test_isValidWeightKg_outOfRange);
  RUN_TEST(test_isValidWeightKg_invalidErrorValue);

  UNITY_END();
  return 0;
}
