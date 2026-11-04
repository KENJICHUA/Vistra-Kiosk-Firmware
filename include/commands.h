#ifndef VISTRA_COMMANDS_H
#define VISTRA_COMMANDS_H

#include <Arduino.h>
#include "config.h"
#include "height.h"
#include "temperature.h"
#include "heart.h"

void dispatchCommand(const char* cmd);

#endif // VISTRA_COMMANDS_H
