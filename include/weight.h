#ifndef VISTRA_WEIGHT_H
#define VISTRA_WEIGHT_H

#include "sensors.h"
#include "calibration.h"
#include "config.h"

#include <HX711.h>

// ---- Shared weight sensor object (defined in weight.cpp) ----
extern HX711 weightSensor;

// ---- Shared weight sensor health flag (defined in weight.cpp) ----
extern bool g_weightReady;

// ---- Weight sensor API ----
bool initWeightSensor();        // initialize HX711, apply calibration
float readWeightKg(byte samples = WEIGHT_SAMPLES);  // returns weight in kg, or -999 on error
bool tareWeight(byte samples = WEIGHT_SAMPLES);     // tare / zero the scale

#endif // VISTRA_WEIGHT_H
