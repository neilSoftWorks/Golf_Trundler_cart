#include "TrundlerDisplay.h"

TrundlerDisplay::TrundlerDisplay(int cs, int dc, int rst) 
    : tft(cs, dc, rst) {
}

void TrundlerDisplay::begin() {
    SPI.begin(TFT_SCK, -1, TFT_MOSI, -1);
    tft.init(240, 320);
    tft.setRotation(2); 
    tft.fillScreen(C_BLACK);
    drawHeader();
}

void TrundlerDisplay::drawHeader() {
    tft.fillRect(0, 0, 240, 35, C_BLACK);
    tft.drawFastHLine(0, 32, 240, C_GREY); 
    tft.setFont(&FreeSans12pt7b);
    tft.setTextColor(C_WHITE);
    tft.setCursor(45, 25);
    tft.print("TRUNDLER V2");
}

void TrundlerDisplay::drawFooter(float voltage, bool hasRemote) {
    tft.fillRect(0, 285, 240, 35, C_GREY);
    drawBattery(10, 292, voltage);
    
    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(C_WHITE);
    tft.setCursor(75, 308);
    tft.printf("%.1fV", voltage);

    drawRemoteIcon(180, 292, hasRemote);
    drawParkedIcon(140, 292, _lastParked);
}

void TrundlerDisplay::drawRemoteIcon(int x, int y, bool connected) {
    if (!connected) return;
    uint16_t color = C_CYAN;
    tft.fillCircle(x+20, y+15, 2, color);
    tft.drawCircle(x+20, y+15, 6, color);
    tft.drawCircle(x+20, y+15, 12, color);
    tft.fillRect(x, y+15, 40, 20, C_GREY); 
}

void TrundlerDisplay::drawParkedIcon(int x, int y, bool parked) {
    if (!parked) return;
    tft.drawCircle(x+10, y+10, 9, C_WHITE);
    tft.setTextColor(C_WHITE);
    tft.setFont(&FreeSans9pt7b);
    tft.setCursor(x+6, y+15);
    tft.print("P");
}

void TrundlerDisplay::drawBattery(int x, int y, float voltage) {
    float pct = (voltage - 32.0) / (41.5 - 32.0);
    pct = constrain(pct, 0.0, 1.0);
    int w = 50, h = 20;
    tft.drawRect(x, y, w, h, C_WHITE);
    tft.fillRect(x + w, y + 5, 4, 10, C_WHITE);
    int fillW = (w - 4) * pct;
    uint16_t color = C_GREEN;
    if (pct < 0.4) color = C_YELLOW;
    if (pct < 0.15) color = C_RED;
    tft.fillRect(x + 2, y + 2, fillW, h - 4, color);
    tft.fillRect(x + 2 + fillW, y + 2, (w - 4) - fillW, h - 4, C_BLACK);
}

void TrundlerDisplay::update(const CartData &stats, int16_t targetSpeed, int16_t currentSteer, float slaveComp, bool active, bool isParked, bool devMode, bool hasRemote) {
    if (millis() - _lastUpdate < 100) return;
    _lastUpdate = millis();

    if (devMode != _lastDevMode) {
        tft.fillRect(0, 35, 240, 250, C_BLACK);
        _lastVolt = -1.0; _lastTarget = -999; _lastSteer = -999;
        _lastComp = -1.0; _lastDevMode = devMode; _lastActive = !active; _lastParked = !isParked;
        _lastRemote = !hasRemote;
    }

    if (stats.voltage != _lastVolt || hasRemote != _lastRemote || isParked != _lastParked) {
        _lastActive = active; 
        _lastParked = isParked;
        drawFooter(stats.voltage, hasRemote);
        _lastVolt = stats.voltage;
        _lastRemote = hasRemote;
    }

    if (devMode) drawDevScreen(stats, targetSpeed, currentSteer, slaveComp, active);
    else drawUserScreen(stats, targetSpeed, currentSteer, active);
}

void TrundlerDisplay::drawUserScreen(const CartData &stats, int16_t targetSpeed, int16_t currentSteer, bool active) {
    if (targetSpeed != _lastTarget) {
        tft.fillRect(0, 70, 240, 100, C_BLACK);
        tft.setFont(&BigFont);
        int userSpeed = abs(targetSpeed) / 17.5; 
        if (abs(targetSpeed) > 0 && userSpeed == 0) userSpeed = 1; 
        String sSpeed = (targetSpeed < 0 ? "-" : "") + String(userSpeed);
        tft.setTextColor(targetSpeed < 0 ? C_ORANGE : C_WHITE); 
        int16_t x1, y1; uint16_t w, h;
        tft.getTextBounds(sSpeed, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((240 - w) / 2, 160);
        tft.print(sSpeed);
        _lastTarget = targetSpeed;
    }

    if (active != _lastActive || _lastParked != _lastParked) {
        tft.fillRect(0, 180, 240, 50, C_BLACK);
        tft.setFont(&FreeSans12pt7b);
        String sStatus = "Stopped";
        uint16_t color = C_WHITE;
        if (active) { sStatus = "Driving"; color = C_GREEN; }
        else if (_lastParked) { sStatus = "Braked"; color = C_RED; }
        tft.setTextColor(color);
        int16_t x1, y1; uint16_t w, h;
        tft.getTextBounds(sStatus, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((240 - w) / 2, 210);
        tft.print(sStatus);
        _lastActive = active;
    }

    if (currentSteer != _lastSteer) {
        tft.fillRect(0, 235, 240, 40, C_BLACK);
        if (currentSteer != 0) {
            tft.setFont(&FreeSans12pt7b);
            tft.setTextColor(C_ORANGE);
            String sSteer = (currentSteer < 0) ? "<<< Left" : "Right >>>";
            int16_t x1, y1; uint16_t w, h;
            tft.getTextBounds(sSteer, 0, 0, &x1, &y1, &w, &h);
            tft.setCursor((240 - w) / 2, 260);
            tft.print(sSteer);
        }
        _lastSteer = currentSteer;
    }
}

void TrundlerDisplay::drawDevScreen(const CartData &stats, int16_t targetSpeed, int16_t currentSteer, float slaveComp, bool active) {
    tft.setFont(&FreeSans9pt7b);
    if (active != _lastActive) {
        tft.fillRect(5, 45, 230, 30, C_BLACK);
        tft.setCursor(5, 65);
        tft.setTextColor(active ? C_GREEN : C_WHITE);
        tft.printf("STATUS: %s", active ? "Driving" : "Parked");
    }
    if (targetSpeed != _lastTarget) {
        tft.fillRect(5, 80, 230, 30, C_BLACK);
        tft.setTextColor(C_CYAN);
        tft.setCursor(5, 100);
        tft.printf("TARGET: %d", targetSpeed);
        _lastTarget = targetSpeed;
    }
    if (stats.speedL != _lastSpeedL || stats.speedR != _lastSpeedR) {
        tft.fillRect(5, 115, 230, 30, C_BLACK);
        tft.setTextColor(C_YELLOW);
        tft.setCursor(5, 135);
        tft.printf("L:%3d  R:%3d rpm", stats.speedL, stats.speedR);
        _lastSpeedL = stats.speedL; _lastSpeedR = stats.speedR;
    }
    if (slaveComp != _lastComp) {
        tft.fillRect(5, 150, 230, 30, C_BLACK);
        tft.setTextColor(C_ORANGE);
        tft.setCursor(5, 170);
        tft.printf("COMP: %.3f", slaveComp);
        _lastComp = slaveComp;
    }
    if (currentSteer != _lastSteer) {
        tft.fillRect(5, 185, 230, 30, C_BLACK);
        tft.setTextColor(C_GREEN);
        tft.setCursor(5, 205);
        tft.printf("STEER: %d", currentSteer);
        _lastSteer = currentSteer;
    }
}
