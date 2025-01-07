#include <Arduino.h>
#include "config.h"

#if ENABLE_DEBUG

uint32_t showTimer(uint32_t &t) {
    uint32_t dt = millis() - t;
    Serial.printf("Timer: %dms\n", dt);
    return dt;
}

void printDebug(const char *s) {
    Serial.printf("Debug: %s\n", s);
}
void printDebug(const String &s) {
    Serial.print("Debug: ");
    Serial.println(s);
}
void printDebug(float f) {
    Serial.printf("Debug: %f\n", f);
}

#endif