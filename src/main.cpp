#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
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

// Tuning Parameters
uint8_t curLimit = 15;  
uint8_t inertia = 10;   

// UI Object (CS=7, DC=2, RST=3)
TrundlerDisplay ui(TFT_CS, TFT_DC, TFT_RST);

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // If you see this in the serial monitor, but REMOTE icon is OFF, 
  // it means the struct size is wrong!
  /*
  Serial.printf("DEBUG RX from: %02X:%02X:%02X:%02X:%02X:%02X | Len: %d\n", 
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], len);
  */

  if (len == sizeof(InputState)) {
    memcpy(&remoteInputs, incomingData, sizeof(InputState));
    lastRecvTime = millis();
    hasRemote = true;
  }
}

void setup() {
  // Primary Serial
  Serial.begin(115200);
  delay(100);

  Serial.println("\n\n=== TRUNDLER S3 BOOTING ===");
  Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
  
  initInput();
  ui.begin();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  
  // Explicitly lock to Channel 1 (must be same as Remote)
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
  }
  esp_now_register_recv_cb(onDataRecv);

  HoverSerial.begin(HOVER_BAUD, SERIAL_8N1, HOVER_SERIAL_RX, HOVER_SERIAL_TX); 
  
  delay(1000); 
  Serial.println("System Ready.");
}

unsigned long lastSend = 0;

void loop() {
  // --- 1. Inputs ---
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
  
  // FWD / REV Speed Adjustments (Only while ACTIVE)
  if (manualActive && (millis() - lastFwdRevAction > 150)) { 
      if (currentInputs.fwd) {
          manualTargetSpeed = CLAMP(manualTargetSpeed + 25, -175, 350); 
          lastFwdRevAction = millis();
      }
      if (currentInputs.rev) {
          manualTargetSpeed = CLAMP(manualTargetSpeed - 25, -175, 350); 
          lastFwdRevAction = millis();
      }
  }

  // --- 2. STOP Button Logic ---
  static unsigned long stopPressStart = 0;
  static bool longPressTriggered = false;

  if (currentInputs.stop && !lastInputs.stop) {
      stopPressStart = millis();
      longPressTriggered = false;
  }
  
  if (currentInputs.stop && !longPressTriggered && (millis() - stopPressStart > 1000)) {
      isParked = !isParked;
      if (isParked) {
          manualActive = false; 
          if (manualTargetSpeed < 0) manualTargetSpeed = 0; // Safety reset for reverse
      }
      longPressTriggered = true;
  }

  if (!currentInputs.stop && lastInputs.stop) {
      if (!longPressTriggered && (millis() - stopPressStart > 50)) {
          if (isParked) {
              isParked = false; manualActive = false; 
          } else {
              manualActive = !manualActive; 
              if (!manualActive && manualTargetSpeed < 0) manualTargetSpeed = 0; // Safety reset for reverse
          }
      }
  }

  // --- 3. Dev Mode Combo (L + R) ---
  static bool chordHandled = false;
  if (currentInputs.left && currentInputs.right) {
      if (!chordHandled) {
          showDevScreen = !showDevScreen;
          chordHandled = true;
      }
  } else chordHandled = false;
  
  lastInputs = currentInputs;

  // --- 4. Ramping & Tank Mix ---
  static unsigned long lastMoveRamp = 0;
  if (millis() - lastMoveRamp > 50) {
      lastMoveRamp = millis();
      int16_t target = (manualActive && !isParked) ? manualTargetSpeed : 0;
      if (manualCurrentSpeed < target) manualCurrentSpeed += 5;
      else if (manualCurrentSpeed > target) manualCurrentSpeed -= 5;
  }
  
  int16_t driveSpeed = manualCurrentSpeed;
  int16_t turnSteer = currentInputs.left ? -100 : (currentInputs.right ? 100 : 0);

  // Governor
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

  if (isParked && !manualActive) { speedL = 0; speedR = 0; }

  uint8_t hoverMode = (isParked && !manualActive) ? 1 : 0;
  uint8_t controlState = (manualActive || isParked) ? STATE_BATT_ONLY : STATE_DISABLE;

  // --- 5. RX Telemetry ---
  if (Receive(HoverSerial, feedback)) {
      cartStatus.voltage = feedback.iVolt / 100.0;
      cartStatus.speedL = feedback.iSpeedL;
      cartStatus.speedR = feedback.iSpeedR;
      cartStatus.odom = feedback.iOdomL;
  }

  // --- 6. UI Update ---
  ui.update(cartStatus, manualTargetSpeed, turnSteer, slaveComp, manualActive, isParked, showDevScreen, hasRemote, curLimit, inertia);

  // --- 7. TX Transmission ---
  if (millis() - lastSend > SEND_INTERVAL_MS) {
      lastSend = millis();
      HoverSend(HoverSerial, speedL, speedR, controlState, controlState, hoverMode, curLimit, inertia);
  }
}
