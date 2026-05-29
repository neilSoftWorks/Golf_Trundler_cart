#ifndef TRUNDLER_DISPLAY_H
#define TRUNDLER_DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include "BigFont.h"
#include "communication.h"
#include "input.h"

class TrundlerDisplay {
public:
    TrundlerDisplay(int cs, int dc, int rst);
    void begin();
    void update(const CartData &stats, int16_t targetSpeed, int16_t currentSteer, float slaveComp, bool active, bool isParked, bool isCruise, bool devMode, bool hasRemote, uint8_t curLimit, uint8_t inertia);

private:
    Adafruit_ST7789 tft;
    unsigned long _lastUpdate = 0;

    // State for flicker reduction
    float _lastVolt = -1.0;
    int16_t _lastSpeedL = -999, _lastSpeedR = -999;
    int16_t _lastTarget = -999;
    int16_t _lastSteer = -999;
    float _lastComp = -1.0;
    uint8_t _lastLimit = 0, _lastInertia = 0;
    bool _lastActive = false; 
    bool _lastParked = false; 
    bool _lastCruise = false;
    bool _lastDevMode = false;
    bool _lastRemote = false;

    void drawHeader();
    void drawFooter(float voltage, bool hasRemote, bool isParked);
    void drawUserScreen(const CartData &stats, int16_t targetSpeed, int16_t currentSteer, bool active);
    void drawDevScreen(const CartData &stats, int16_t targetSpeed, int16_t currentSteer, float slaveComp, bool active, bool isParked, bool isCruise);
    void drawBattery(int x, int y, float voltage);
    void drawRemoteIcon(int x, int y, bool connected);
    void drawParkedIcon(int x, int y, bool parked);

    const uint16_t C_BLACK   = 0x0000;
    const uint16_t C_WHITE   = 0xFFFF;
    const uint16_t C_GREY    = 0x2104; 
    const uint16_t C_GREEN   = 0x07E0;
    const uint16_t C_RED     = 0xF800;
    const uint16_t C_YELLOW  = 0xFFE0;
    const uint16_t C_CYAN    = 0x07FF;
    const uint16_t C_ORANGE  = 0xFC00;
};

#endif