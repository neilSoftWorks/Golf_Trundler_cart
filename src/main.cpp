#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "util.h"
#include "hoverserial.h"
#include "input.h" 
#include "TrundlerDisplay.h"
#include "communication.h"

// --- Config ---
#define HOVER_BAUD 19200
#define SEND_INTERVAL_MS 20  

HardwareSerial HoverSerial(1); 
SerialHover2Server feedback;

InputState remoteInputs; 
CartData cartStatus;

bool hasRemote = false;
unsigned long lastRecvTime = 0;

// System States
bool manualActive = false;   
bool isParked = false;       
int16_t manualTargetSpeed = 0; 
int16_t manualCurrentSpeed = 0;
float slaveComp = 1.0; 
bool showDevScreen = false; 

// Tuning Parameters (Now Dynamic!)
uint8_t curLimit = 15;  // Default 15 Amps
uint8_t inertia = 10;   // Default FILTER_SHIFT 10

// UI Object (CS=7, DC=2, RST=3)
TrundlerDisplay ui(TFT_CS, TFT_DC, TFT_RST);

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(InputState)) {
    memcpy(&remoteInputs, incomingData, sizeof(InputState));
    lastRecvTime = millis();
    hasRemote = true;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- GOLF CART TRUNDLER (UNIVERSAL CONTROL) ---");
  Serial.print("MY MAC: "); Serial.println(WiFi.macAddress());
  
  initInput();
  ui.begin();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
  }
  esp_now_register_recv_cb(onDataRecv);

  HoverSerial.begin(HOVER_BAUD, SERIAL_8N1, HOVER_SERIAL_RX, HOVER_SERIAL_TX); 
  
  delay(1000); 
  Serial.println("System Ready.");
}

unsigned long lastSend = 0;
unsigned long lastSerialDebug = 0;
uint32_t packetCounter = 0;

void loop() {
  // --- 1. Combined Button Logic ---
  InputState localInputs = readInput();
  
  if (hasRemote && (millis() - lastRecvTime > 1000)) {
      memset(&remoteInputs, 0, sizeof(remoteInputs));
      hasRemote = false;
  }

  InputState currentInputs;
  currentInputs.fwd   = localInputs.fwd   || remoteInputs.fwd;
  currentInputs.rev   = localInputs.rev   || remoteInputs.rev;
  currentInputs.left  = localInputs.left  || remoteInputs.left;
  currentInputs.right = localInputs.right || remoteInputs.right;
  currentInputs.stop  = localInputs.stop  || remoteInputs.stop;

  static InputState lastInputs;
  static unsigned long lastFwdRevAction = 0;
  
  if (manualActive && (millis() - lastFwdRevAction > 200)) { 
      if (currentInputs.fwd) {
          manualTargetSpeed = CLAMP(manualTargetSpeed + 25, -175, 350); 
          lastFwdRevAction = millis();
      }
      if (currentInputs.rev) {
          manualTargetSpeed = CLAMP(manualTargetSpeed - 25, -175, 350); 
          lastFwdRevAction = millis();
      }
  }

  // --- 2. STOP Button Multi-Action Logic ---
  static unsigned long stopPressTime = 0;
  static unsigned long lastStopReleaseTime = 0;
  static int clickCount = 0;
  static bool stopHandledLong = false;
  static bool waitingForDoubleClick = false;

  if (currentInputs.stop && !lastInputs.stop) {
      stopPressTime = millis();
      stopHandledLong = false;
  }
  
  if (currentInputs.stop && !stopHandledLong && (millis() - stopPressTime > 1200)) {
      isParked = !isParked;
      if (isParked) manualActive = false; 
      stopHandledLong = true;
      waitingForDoubleClick = false;
  }

  if (!currentInputs.stop && lastInputs.stop) {
      if (!stopHandledLong) {
          clickCount++;
          lastStopReleaseTime = millis();
          waitingForDoubleClick = true;
      }
  }

  if (waitingForDoubleClick && (millis() - lastStopReleaseTime > 350)) {
      if (clickCount >= 2) {
          showDevScreen = !showDevScreen;
      } else {
          manualActive = !manualActive;
          if (manualActive) isParked = false; 
      }
      clickCount = 0;
      waitingForDoubleClick = false;
  }
  
  lastInputs = currentInputs;

  // --- 3. Internal Ramping ---
  static unsigned long lastMoveRamp = 0;
  if (millis() - lastMoveRamp > 50) {
      lastMoveRamp = millis();
      int16_t target = (manualActive && !isParked) ? manualTargetSpeed : 0;
      if (manualCurrentSpeed < target) manualCurrentSpeed += 5;
      else if (manualCurrentSpeed > target) manualCurrentSpeed -= 5;
  }
  
  // --- 4. Arbitration & Tank Mix ---
  int16_t driveSpeed = manualCurrentSpeed;
  int16_t turnSteer = 0; 
  if (currentInputs.left) turnSteer = -100;
  else if (currentInputs.right) turnSteer = 100;

  static unsigned long lastCompAdjust = 0;
  if (manualActive && abs(driveSpeed) > 100 && turnSteer == 0 && (millis() - lastCompAdjust > 200)) {
      lastCompAdjust = millis();
      int16_t absL = abs(cartStatus.speedL);
      int16_t absR = abs(cartStatus.speedR);
      if (absR > absL + 20) slaveComp -= 0.002; 
      else if (absR < absL - 20) slaveComp += 0.002; 
      slaveComp = constrain(slaveComp, 0.7, 1.3); 
  }
  
  int16_t speedL = -(driveSpeed + turnSteer);
  int16_t speedR = (driveSpeed - turnSteer) * slaveComp; 

  // Mode Selection: 1 (PID) for Brake, 0 (PWM) for Drive
  uint8_t hoverMode = (isParked && !manualActive) ? 1 : 0;
  uint8_t controlState = (manualActive || isParked) ? STATE_BATT_ONLY : STATE_DISABLE;

  // --- 5. RX Telemetry ---
  if (Receive(HoverSerial, feedback)) {
      packetCounter++;
      cartStatus.voltage = feedback.iVolt / 100.0;
      cartStatus.speedL = feedback.iSpeedL;
      cartStatus.speedR = feedback.iSpeedR;
      cartStatus.odom = feedback.iOdomL;
  }

  // --- 6. UI Update ---
  ui.update(cartStatus, manualTargetSpeed, turnSteer, slaveComp, manualActive, isParked, showDevScreen, hasRemote);

  if (millis() - lastSerialDebug > 1000) {
      Serial.printf("V: %.2fV | L: %d R: %d | Mode: %d | Brake: %s\n", 
          cartStatus.voltage, cartStatus.speedL, cartStatus.speedR, hoverMode, isParked ? "ON" : "OFF");
      packetCounter = 0;
      lastSerialDebug = millis();
  }

  // --- 7. TX Transmission ---
  if (millis() - lastSend > SEND_INTERVAL_MS) {
      lastSend = millis();
      HoverSend(HoverSerial, speedL, speedR, controlState, controlState, hoverMode, curLimit, inertia);
  }
}
