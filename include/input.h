#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

// Note: Pin definitions (BTN_FWD, BTN_REV, etc.) are managed in platformio.ini

// Input State (Explicitly sized for protocol matching)
struct __attribute__((packed)) InputState {
    uint8_t fwd;
    uint8_t rev;
    uint8_t left;
    uint8_t right;
    uint8_t stop; 
};

void initInput();
InputState readInput();

#endif
