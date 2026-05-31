#include "input.h"

void initInput() {
    pinMode(ENC_SW, INPUT_PULLUP);
}

InputState readInput() {
    InputState s;
    memset(&s, 0, sizeof(s));
    
    // Encoder button is handled via the library in main.cpp, 
    // but we can read it here if we want a raw state in the future.
    s.stop = !digitalRead(ENC_SW); 
    
    // Local controller has no steering buttons
    s.left = 0;
    s.right = 0;
    s.fwd = 0; 
    s.rev = 0;
    
    return s;
}
