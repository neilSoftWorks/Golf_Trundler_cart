#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

// Note: Pin definitions (BTN_FWD, BTN_REV, etc.) are managed in platformio.ini

// Input State
struct InputState {
    bool fwd;
    bool rev;
    bool left;
    bool right;
    bool stop; // stop/mode button
};

void initInput();
InputState readInput();

#endif
