#ifndef VISTRA_HEIGHT_H
#define VISTRA_HEIGHT_H

#include "sensors.h"
#include "calibration.h"
#include "config.h"
#include "array_helpers.h"

// ---- Test hooks (pure functions, no hardware) ----
bool personUnderHeightSensor();

void measureHeight();

#endif // VISTRA_HEIGHT_H
