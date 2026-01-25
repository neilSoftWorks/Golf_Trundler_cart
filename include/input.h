#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

// Button Pins
#define BTN_FWD   12
#define BTN_REV   13
#define BTN_LEFT  14
#define BTN_RIGHT 27
#define BTN_STOP  26

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
