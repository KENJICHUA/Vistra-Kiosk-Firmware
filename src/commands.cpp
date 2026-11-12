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
  Serial.print(F(",\"weight\":"));
  Serial.print(g_weightReady ? F("true") : F("false"));
  Serial.println(F("}"));
}

// ---- Internal: CAL subcommand handlers ----
static void calShow() {
  calibrationPrint();
}

static void calSetHeight(const char* valueStr) {
  if (!valueStr || *valueStr == '\0') {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"missing value\"}"));
    return;
  }
  float val = atof(valueStr);
  if (val < 0.0f || val > 500.0f) {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"value out of range\"}"));
    return;
  }
  g_sensorToFloorCm = val;
  Serial.print(F("{\"type\":\"calibration\",\"status\":\"set\",\"sensor_to_floor_cm\":"));
  Serial.print(g_sensorToFloorCm, 2);
  Serial.println(F("}"));
}

static void calSetTempScale(const char* valueStr) {
  if (!valueStr || *valueStr == '\0') {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"missing value\"}"));
    return;
  }
  float val = atof(valueStr);
  if (val < 0.1f || val > 10.0f) {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"value out of range\"}"));
    return;
  }
  g_tempCalScale = val;
  Serial.print(F("{\"type\":\"calibration\",\"status\":\"set\",\"temp_cal_scale\":"));
  Serial.print(g_tempCalScale, 3);
  Serial.println(F("}"));
}

static void calSetTempOffset(const char* valueStr) {
  if (!valueStr || *valueStr == '\0') {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"missing value\"}"));
    return;
  }
  float val = atof(valueStr);
  if (val < -50.0f || val > 50.0f) {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"value out of range\"}"));
    return;
  }
  g_tempCalOffset = val;
  Serial.print(F("{\"type\":\"calibration\",\"status\":\"set\",\"temp_cal_offset\":"));
  Serial.print(g_tempCalOffset, 2);
  Serial.println(F("}"));
}

static void calSetWeightScale(const char* valueStr) {
  if (!valueStr || *valueStr == '\0') {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"missing value\"}"));
    return;
  }
  float val = atof(valueStr);
  if (val <= 0.0f || val > 100000.0f) {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"value out of range\"}"));
    return;
  }
  g_weightScaleFactor = val;
  weightSensor.set_scale(g_weightScaleFactor);
  Serial.print(F("{\"type\":\"calibration\",\"status\":\"set\",\"weight_scale_factor\":"));
  Serial.print(g_weightScaleFactor, 4);
  Serial.println(F("}"));
}

static void calSetWeightOffset(const char* valueStr) {
  if (!valueStr || *valueStr == '\0') {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"missing value\"}"));
    return;
  }
  float val = atof(valueStr);
  if (val < -1000000.0f || val > 1000000.0f) {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"value out of range\"}"));
    return;
  }
  g_weightOffset = val;
  weightSensor.set_offset(static_cast<long>(g_weightOffset));
  Serial.print(F("{\"type\":\"calibration\",\"status\":\"set\",\"weight_offset\":"));
  Serial.print(g_weightOffset, 2);
  Serial.println(F("}"));
}

static void calSetHeartScale(const char* valueStr) {
  if (!valueStr || *valueStr == '\0') {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"missing value\"}"));
    return;
  }
  float val = atof(valueStr);
  if (val < 0.1f || val > 10.0f) {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"value out of range\"}"));
    return;
  }
  g_heartCalScale = val;
  Serial.print(F("{\"type\":\"calibration\",\"status\":\"set\",\"heart_cal_scale\":"));
  Serial.print(g_heartCalScale, 3);
  Serial.println(F("}"));
}

static void calSetHeartOffset(const char* valueStr) {
  if (!valueStr || *valueStr == '\0') {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"missing value\"}"));
    return;
  }
  float val = atof(valueStr);
  if (val < -50.0f || val > 50.0f) {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"value out of range\"}"));
    return;
  }
  g_heartCalOffset = val;
  Serial.print(F("{\"type\":\"calibration\",\"status\":\"set\",\"heart_cal_offset\":"));
  Serial.print(g_heartCalOffset, 1);
  Serial.println(F("}"));
}

static void calSave() {
  if (calibrationSave()) {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"saved\"}"));
  } else {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"save failed\"}"));
  }
}

static void calLoad() {
  calibrationInit();
}

static void calReset() {
  calibrationReset();
  Serial.println(F("{\"type\":\"calibration\",\"status\":\"reset\"}"));
}

// ---- Internal: parse CAL subcommand ----
// `rest` points to everything after "CAL " (already uppercased by serial layer)
static void handleCalCommand(const char* rest) {
  if (!rest || *rest == '\0') {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"missing subcommand\"}"));
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

  if (strcmp(sub, "SHOW") == 0 || strcmp(sub, "GET") == 0) {
    calShow();
  } else if (strcmp(sub, "SAVE") == 0) {
    calSave();
  } else if (strcmp(sub, "LOAD") == 0) {
    calLoad();
  } else if (strcmp(sub, "RESET") == 0) {
    calReset();
  } else if (strcmp(sub, "SET") == 0) {
    // Expect: SET HEIGHT <value>  or  SET TEMP SCALE <value>  or  SET TEMP OFFSET <value>
    static char target[16];
    i = 0;
    while (*rest && *rest != ' ' && i < 15) {
      target[i++] = static_cast<char>(*rest++);
    }
    target[i] = '\0';
    while (*rest == ' ') {
      rest++;
    }

    if (strcmp(target, "HEIGHT") == 0) {
      calSetHeight(rest);
    } else if (strcmp(target, "TEMP") == 0) {
      static char kind[16];
      i = 0;
      while (*rest && *rest != ' ' && i < 15) {
        kind[i++] = static_cast<char>(*rest++);
      }
      kind[i] = '\0';
      while (*rest == ' ') {
        rest++;
      }
      if (strcmp(kind, "SCALE") == 0) {
        calSetTempScale(rest);
      } else if (strcmp(kind, "OFFSET") == 0) {
        calSetTempOffset(rest);
      } else {
        Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"unknown temp param\"}"));
      }
    } else if (strcmp(target, "WEIGHT") == 0) {
      static char kind[16];
      i = 0;
      while (*rest && *rest != ' ' && i < 15) {
        kind[i++] = static_cast<char>(*rest++);
      }
      kind[i] = '\0';
      while (*rest == ' ') {
        rest++;
      }
      if (strcmp(kind, "SCALE") == 0) {
        calSetWeightScale(rest);
      } else if (strcmp(kind, "OFFSET") == 0) {
        calSetWeightOffset(rest);
      } else {
        Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"unknown weight param\"}"));
      }
    } else if (strcmp(target, "HEART") == 0) {
      static char kind[16];
      i = 0;
      while (*rest && *rest != ' ' && i < 15) {
        kind[i++] = static_cast<char>(*rest++);
      }
      kind[i] = '\0';
      while (*rest == ' ') {
        rest++;
      }
      if (strcmp(kind, "SCALE") == 0) {
        calSetHeartScale(rest);
      } else if (strcmp(kind, "OFFSET") == 0) {
        calSetHeartOffset(rest);
      } else {
        Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"unknown heart param\"}"));
      }
    } else {
      Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"unknown set target\"}"));
    }
  } else {
    Serial.print(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"unknown subcommand: '"));
    Serial.print(sub);
    Serial.println(F("'}"));
  }
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
  } else if (strcmp(cmd, "WEIGHT") == 0) {
    measureWeight();
  } else if (strcmp(cmd, "TEST") == 0) {
    Serial.println(F("{\"type\":\"test\",\"status\":\"error\",\"message\":\"missing subcommand\"}"));
  } else if (strncmp(cmd, "TEST ", 5) == 0) {
    handleTestCommand(cmd + 5);
  } else if (strcmp(cmd, "CAL") == 0) {
    Serial.println(F("{\"type\":\"calibration\",\"status\":\"error\",\"message\":\"missing subcommand\"}"));
  } else if (strncmp(cmd, "CAL ", 4) == 0) {
    handleCalCommand(cmd + 4);
  } else if (strcmp(cmd, "RESCAN") == 0) {
    bool changed = rescanSensors();
    Serial.print(F("{\"type\":\"rescan\",\"status\":\"success\",\"changed\":"));
    Serial.print(changed ? F("true") : F("false"));
    Serial.println(F("}"));
  } else {
    Serial.println(F("{\"type\":\"error\",\"status\":\"unknown_command\"}"));
  }
}
