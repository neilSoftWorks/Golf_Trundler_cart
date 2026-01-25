#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>

#define ABS(a) (((a) < 0.0) ? -(a) : (a))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

// Debug macros
#define DEBUG(txt, val) {Serial.print(F(txt)); Serial.print(F(": ")); Serial.print(val);}
#define DEBUGT(txt, val) {Serial.print(F(txt)); Serial.print(F(": ")); Serial.print(val); Serial.print(F("\t"));}
#define DEBUGN(txt, val) {Serial.print(F(txt)); Serial.print(F(": ")); Serial.println(val);}

#endif // UTIL_H
