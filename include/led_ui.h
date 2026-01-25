#ifndef LED_UI_H
#define LED_UI_H

#include <Arduino.h>
#include "input.h"

// Pin Definitions
#define PIN_LED_GREEN   18
#define PIN_LED_YELLOW  19
#define PIN_LED_RED     5

void initLEDs();
void updateLEDs(bool isManualMode, bool isManualActive, bool isRemoteConnected, bool waitingForZero, float batteryVoltage);

#endif