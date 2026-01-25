#include "input.h"

void initInput() {
    pinMode(BTN_FWD, INPUT_PULLUP);
    pinMode(BTN_REV, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_STOP, INPUT_PULLUP);
}

InputState readInput() {
    InputState s;
    // Low = Pressed (because Pull-up)
    s.fwd   = !digitalRead(BTN_FWD);
    s.rev   = !digitalRead(BTN_REV);
    s.left  = !digitalRead(BTN_LEFT);
    s.right = !digitalRead(BTN_RIGHT);
    s.stop  = !digitalRead(BTN_STOP);
    return s;
}
