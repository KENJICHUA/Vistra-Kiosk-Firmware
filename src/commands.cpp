#include "commands.h"

// ---- Internal: STATUS handler ----
static void printStatus() {
  Serial.print(F("{\"type\":\"status\","));
  Serial.print(F("\"tof\":"));
  Serial.print(g_tofReady ? F("true") : F("false"));
  Serial.print(F(",\"heart\":"));
  Serial.print(g_heartReady ? F("true") : F("false"));
  Serial.print(F(",\"temperature\":"));
  Serial.print(g_mlxReady ? F("true") : F("false"));
  Serial.println(F("}"));
}

// ---- Command dispatch ----
void dispatchCommand(const char* cmd) {
  if (strcmp(cmd, "STATUS") == 0) {
    printStatus();
  } else if (strcmp(cmd, "HEIGHT") == 0) {
    measureHeight();
  } else if (strcmp(cmd, "TEMP") == 0) {
    measureTemperature();
  } else if (strcmp(cmd, "HEART") == 0) {
    measureHeart();
  } else {
    Serial.println(F("{\"type\":\"error\",\"status\":\"unknown_command\"}"));
  }
}
