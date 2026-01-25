#include "led_ui.h"

// Blink State Variables
unsigned long lastBlink = 0;
const int slowInterval = 500; // 1Hz

void initLEDs() {
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT); // Back to standard Output
    pinMode(PIN_LED_RED, OUTPUT);
    
    // Boot Sequence
    digitalWrite(PIN_LED_RED, HIGH); delay(200);
    digitalWrite(PIN_LED_YELLOW, HIGH); delay(200);
    digitalWrite(PIN_LED_GREEN, HIGH); delay(200);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
}

void updateLEDs(bool isManualMode, bool isManualActive, bool isRemoteConnected, bool waitingForZero, float batteryVoltage) {
    unsigned long now = millis();
    
    // --- 1. GREEN LED (Connection Status) ---
    if (isRemoteConnected) {
        // Connected: Solid
        digitalWrite(PIN_LED_GREEN, HIGH);
    } else {
        // No Connection: Off (Less distraction)
        digitalWrite(PIN_LED_GREEN, LOW);
    }

    // --- 2. YELLOW LED (Manual Mode) ---
    if (isManualMode) {
        if (isManualActive) {
            // Driving: Solid (Steady light while walking)
            digitalWrite(PIN_LED_YELLOW, HIGH);
        } else {
            // Paused: Slow Blink (Standby)
            int phase = (now / 500) % 2;
            digitalWrite(PIN_LED_YELLOW, phase);
        }
    } else {
        // Remote Mode: Off
        digitalWrite(PIN_LED_YELLOW, LOW);
    }

    // --- 3. RED LED (Battery Monitor) ---
    if (batteryVoltage > 10.0 && batteryVoltage < 33.0) {
        // Low Battery: Slow Blink
        int phase = (now / 1000) % 2;
        digitalWrite(PIN_LED_RED, phase);
    } else if (batteryVoltage > 10.0 && batteryVoltage < 31.0) {
        // Critical: Solid
        digitalWrite(PIN_LED_RED, HIGH);
    } else {
        digitalWrite(PIN_LED_RED, LOW);
    }
}
