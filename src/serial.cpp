#include "serial.h"

// ---- Shared command buffer (actual storage) ----
char g_cmdBuf[kCmdBufferSize + 1];
size_t g_cmdLen = 0;

// ---- Internal helpers ----
static void trimNewline(char* buf) {
  size_t len = strlen(buf);
  while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
    buf[--len] = '\0';
  }
}

static void toUpperInPlace(char* buf) {
  for (char* p = buf; *p; ++p) {
    if (*p >= 'a' && *p <= 'z') {
      *p = static_cast<char>(*p - 'a' + 'A');
    }
  }
}

// ---- Serial input processing ----
void processSerial() {
  while (Serial.available() > 0) {
    char c = static_cast<char>(Serial.read());

    // Newline terminates a command
    if (c == '\n' || c == '\r') {
      if (g_cmdLen > 0) {
        g_cmdBuf[g_cmdLen] = '\0';
        trimNewline(g_cmdBuf);
        toUpperInPlace(g_cmdBuf);

        // Echo-less dispatch; ignore blank lines
        if (g_cmdBuf[0] != '\0') {
          Serial.println();
          Serial.print(F("RECEIVED: "));
          Serial.println(g_cmdBuf);
          dispatchCommand(g_cmdBuf);
        }
        g_cmdLen = 0;
      }
      continue;
    }

    // Ignore other control characters
    if (c < 0x20) {
      continue;
    }

    if (g_cmdLen < kCmdBufferSize) {
      g_cmdBuf[g_cmdLen++] = c;
    } else {
      // Buffer overflow: drop the line and signal the user
      g_cmdLen = 0;
      Serial.println(F("ERR: command too long"));
    }
  }
}
