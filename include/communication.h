#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>

// Data sent from Remote to Cart
typedef struct {
    int16_t speed;
    int16_t steer;
    bool stop;
    bool fwd;
    bool rev;
    bool left;
    bool right;
} RemoteData;

// Data sent from Cart back to Remote (Telemetry)
typedef struct {
    float voltage;
    float ampL;
    float ampR;
    int16_t speedL;
    int16_t speedR;
    int32_t odom;
    bool isManual; // NEW: Feedback for Remote
    int16_t cmdSpeed; // Target Speed (Manual Setpoint or Remote Command)
} CartData;

#endif
