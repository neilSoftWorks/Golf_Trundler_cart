#include "TrundlerDisplay.h"

TrundlerDisplay::TrundlerDisplay(int cs, int dc, int rst) 
    : tft(cs, dc, rst) {
}

void TrundlerDisplay::begin() {
    SPI.begin(TFT_SCK, -1, TFT_MOSI, -1);
    tft.initR(INITR_144GREENTAB); 
    tft.setRotation(1); // Landscape (USB port on left/right depending on mount)
    tft.fillScreen(C_BLACK);
}

void TrundlerDisplay::drawFooter(float voltage, bool hasRemote) {
    // Moved up for landscape mode (assuming 128 height, though might be 160 width)
    tft.fillRect(0, 105, 128, 23, C_GREY);
    
    // Draw battery without the text next to it
    drawBattery(5, 108, voltage);
    
    // Draw the signal icon if remote is connected
    drawRemoteIcon(100, 108, hasRemote);
}

void TrundlerDisplay::drawRemoteIcon(int x, int y, bool connected) {
    if (!connected) return;
    uint16_t color = C_CYAN;
    
    // Draw concentric circles to look like a radio signal
    tft.fillCircle(x+10, y+8, 2, color);
    tft.drawCircle(x+10, y+8, 5, color);
    tft.drawCircle(x+10, y+8, 9, color);
    
    // Mask the bottom half to make it look like a wifi/broadcast symbol
    tft.fillRect(x, y+8, 20, 10, C_GREY); 
}

void TrundlerDisplay::drawBattery(int x, int y, float voltage) {
    // Adjusted curve: 38.0V is 100%, 32.0V is 0%
    float pct = (voltage - 32.0) / (38.0 - 32.0);
    pct = constrain(pct, 0.0, 1.0);
    int w = 35, h = 15;
    tft.drawRect(x, y, w, h, C_WHITE);
    tft.fillRect(x + w, y + 4, 3, 7, C_WHITE);
    int fillW = (w - 4) * pct;
    uint16_t color = C_GREEN;
    if (pct < 0.4) color = C_YELLOW;
    if (pct < 0.15) color = C_RED;
    tft.fillRect(x + 2, y + 2, fillW, h - 4, color);
    tft.fillRect(x + 2 + fillW, y + 2, (w - 4) - fillW, h - 4, C_BLACK);
}

void TrundlerDisplay::update(const CartData &stats, int16_t targetSpeed, int16_t currentSteer, float slaveComp, bool active, bool isCruise, bool devMode, bool hasRemote, uint8_t curLimit, uint8_t inertia) {
    if (millis() - _lastUpdate < 100) return;
    _lastUpdate = millis();

    if (devMode != _lastDevMode) {
        tft.fillScreen(C_BLACK); 
        _lastDevMode = devMode;
        _lastVolt = -1.0; _lastTarget = -999; _lastSteer = -999; 
        _lastComp = -1.0; _lastLimit = 0; _lastActive = !active; _lastCruise = !isCruise;
        drawFooter(stats.voltage, hasRemote); 
    }

    if (stats.voltage != _lastVolt || hasRemote != _lastRemote) {
        drawFooter(stats.voltage, hasRemote);
        _lastVolt = stats.voltage;
        _lastRemote = hasRemote;
    }

    if (devMode) {
        drawDevScreen(stats, targetSpeed, currentSteer, slaveComp, active, isCruise);
    }
    else {
        // Row 1: Speed Big Number
        if (targetSpeed != _lastTarget) {
            tft.fillRect(0, 0, 128, 70, C_BLACK);
            tft.setFont(&FreeSans24pt7b); 
            int userSpeed = abs(targetSpeed); 
            String sSpeed = (targetSpeed < 0 ? "-" : "") + String(userSpeed);
            tft.setTextColor(targetSpeed < 0 ? C_ORANGE : C_WHITE); 
            
            int16_t x1, y1; uint16_t w, h;
            tft.getTextBounds(sSpeed, 0, 0, &x1, &y1, &w, &h);
            tft.setCursor((128 - w) / 2, 55);
            tft.print(sSpeed);
            _lastTarget = targetSpeed;
        }

        // Row 2: Status Text (Mixed Case, Small Font)
        if (active != _lastActive) {
            tft.fillRect(0, 75, 128, 15, C_BLACK);
            tft.setFont(NULL); // Small built-in font for space
            String sStatus = active ? "Driving" : "Stopped";
            uint16_t color = active ? C_GREEN : C_WHITE;
            tft.setTextColor(color);
            int16_t x1, y1; uint16_t w, h;
            tft.getTextBounds(sStatus, 0, 0, &x1, &y1, &w, &h);
            tft.setCursor((128 - w) / 2, 80);
            tft.print(sStatus);
        }

        // Row 3: Mode (Mixed Case)
        if (isCruise != _lastCruise || active != _lastActive) {
            tft.fillRect(0, 92, 128, 12, C_BLACK);
            if (active) {
                tft.setFont(NULL); 
                tft.setTextColor(isCruise ? C_CYAN : C_GREY);
                String sMode = isCruise ? "Cruise Control" : "Smooth PWM";
                int16_t x1, y1; uint16_t w, h;
                tft.getTextBounds(sMode, 0, 0, &x1, &y1, &w, &h);
                tft.setCursor((128 - w) / 2, 95);
                tft.print(sMode);
            }
        }
    }

    _lastActive = active;
    _lastCruise = isCruise;
}

void TrundlerDisplay::drawUserScreen(const CartData &stats, int16_t targetSpeed, int16_t currentSteer, bool active) {
}

void TrundlerDisplay::drawDevScreen(const CartData &stats, int16_t targetSpeed, int16_t currentSteer, float slaveComp, bool active, bool isCruise) {
    tft.setFont(NULL);
    if (targetSpeed != _lastTarget) {
        tft.fillRect(2, 45, 124, 10, C_BLACK);
        tft.setTextColor(C_CYAN);
        tft.setCursor(2, 45);
        tft.printf("TGT: %d", targetSpeed);
        _lastTarget = targetSpeed;
    }
    if (stats.speedL != _lastSpeedL || stats.speedR != _lastSpeedR) {
        tft.fillRect(2, 55, 124, 10, C_BLACK);
        tft.setTextColor(C_YELLOW);
        tft.setCursor(2, 55);
        tft.printf("L:%d R:%d", stats.speedL, stats.speedR);
        _lastSpeedL = stats.speedL; _lastSpeedR = stats.speedR;
    }
    if (slaveComp != _lastComp) {
        tft.fillRect(2, 65, 124, 10, C_BLACK);
        tft.setTextColor(C_ORANGE);
        tft.setCursor(2, 65);
        tft.printf("CMP: %.3f", slaveComp);
        _lastComp = slaveComp;
    }
}
