#ifndef VISTRA_SERIAL_H
#define VISTRA_SERIAL_H

#include <Arduino.h>
#include "config.h"
#include "commands.h"

// ---- Shared command buffer (defined in serial.cpp) ----
extern char g_cmdBuf[kCmdBufferSize + 1];
extern size_t g_cmdLen;

// ---- Serial input processing ----
void processSerial();

#endif // VISTRA_SERIAL_H
