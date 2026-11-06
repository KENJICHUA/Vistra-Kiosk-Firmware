#ifndef VISTRA_TEMPERATURE_H
#define VISTRA_TEMPERATURE_H

#include "sensors.h"
#include "calibration.h"
#include "config.h"
#include "array_helpers.h"

// ---- Test hooks (pure functions, no hardware) ----
bool detectHand(float& ambient, float& objectTemp);

void measureTemperature();

#endif // VISTRA_TEMPERATURE_H
