#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "util.h"
#include "hoverserial.h"
#include "input.h" 
#include "led_ui.h"
#include "communication.h"

// --- Config ---
#define HOVER_BAUD 19200

// Confirmed Pin Layout
#ifndef HOVER_SERIAL_RX
#define HOVER_SERIAL_RX 26
#endif
#ifndef HOVER_SERIAL_TX
#define HOVER_SERIAL_TX 27
#endif

#define ROTARY_SW  32
#define ROTARY_DT  33
#define ROTARY_CLK 25

#define SEND_INTERVAL_MS 50

HardwareSerial HoverSerial(2); 
SerialHover2Server feedback;

RemoteData remoteCmd;
CartData cartStatus;

bool hasRemote = false;
uint8_t remoteAddress[6];
unsigned long lastRecvTime = 0;

// Manual Mode State
bool isManualMode = true;   
bool manualActive = false;   
bool waitingForRemoteZero = true; // Safety Interlock (Starts TRUE for power-on safety)
int16_t manualTargetSpeed = 150; 
int16_t manualCurrentSpeed = 0;

// Encoder State (Table Based)
const int8_t encoderTable[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
static uint8_t encoderState = 0;
static int8_t encoderAccumulator = 0;

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&remoteCmd, incomingData, sizeof(remoteCmd));
  lastRecvTime = millis();
  
  if (!hasRemote) {
    memcpy(remoteAddress, mac, 6);
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, remoteAddress, 6);
    peerInfo.channel = 0; 
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA; 
    esp_now_add_peer(&peerInfo);
    hasRemote = true;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- GOLF CART STARTING ---");
  
  initLEDs();

  // Init Encoder Pins
  pinMode(ROTARY_CLK, INPUT_PULLUP);
  pinMode(ROTARY_DT, INPUT_PULLUP);
  pinMode(ROTARY_SW, INPUT_PULLUP);
  
  // Init WiFi
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(onDataRecv);

  // Init UART on 26/27
  HoverSerial.begin(HOVER_BAUD, SERIAL_8N1, HOVER_SERIAL_RX, HOVER_SERIAL_TX); 
  
  delay(1000); 
  Serial.println("System Ready.");
}

unsigned long lastSend = 0;
unsigned long lastRamp = 0;
unsigned long lastSerialDebug = 0;

void loop() {
  // --- 1. Encoder Logic (Robust State Table) ---
  encoderState = (encoderState << 2) & 0x0F;
  encoderState |= (digitalRead(ROTARY_CLK) << 1) | digitalRead(ROTARY_DT);
  int8_t motion = encoderTable[encoderState];

  if (motion != 0) {
      encoderAccumulator += motion;
      if (abs(encoderAccumulator) >= 4) {
          int direction = (encoderAccumulator > 0) ? 1 : -1;
          manualTargetSpeed += (direction * 25);
          manualTargetSpeed = CLAMP(manualTargetSpeed, 0, 500);
          encoderAccumulator = 0; 
      }
  }

  // --- 2. Button Logic ---
  static int lastBtnState = HIGH;
  static unsigned long btnPressStart = 0;
  static bool btnHandled = false;
  int btnState = digitalRead(ROTARY_SW);
  
  if (btnState == LOW && lastBtnState == HIGH) { 
      btnPressStart = millis();
      btnHandled = false;
  }
  if (btnState == LOW && !btnHandled && (millis() - btnPressStart > 1000)) { 
      // Long Press: Toggle MODE
      isManualMode = !isManualMode;
      manualActive = false; 
      
      // If switching TO Remote, force it to zero out first
      if (!isManualMode) waitingForRemoteZero = true;
      
      btnHandled = true; 
      Serial.println("MODE: Switched!");
  }
  if (btnState == HIGH && lastBtnState == LOW) { 
      if (!btnHandled && (millis() - btnPressStart > 50)) { 
          if (isManualMode) {
              manualActive = !manualActive;
              Serial.println("MANUAL: Toggled Drive");
          }
      }
  }
  lastBtnState = btnState;

  // --- 3. Ramping ---
  if (millis() - lastRamp > 50) {
      lastRamp = millis();
      int16_t target = (isManualMode && manualActive) ? manualTargetSpeed : 0;
      if (manualCurrentSpeed < target) manualCurrentSpeed += 10;
      else if (manualCurrentSpeed > target) manualCurrentSpeed -= 10;
  }
  
  // --- 4. Arbitration ---
  int16_t finalSpeed = 0;
  int16_t finalSteer = 0;
  bool remoteValid = (lastRecvTime > 0 && millis() - lastRecvTime < 1500);
  
  if (isManualMode || manualCurrentSpeed > 0) {
      finalSpeed = manualCurrentSpeed;
      finalSteer = 0; 
  } else if (remoteValid) {
      // Safety Interlock
      if (waitingForRemoteZero) {
          if (remoteCmd.speed == 0) waitingForRemoteZero = false;
          else finalSpeed = 0;
      } else {
          finalSpeed = remoteCmd.speed;
          finalSteer = remoteCmd.steer;
      }
  }

  // --- 5. Telemetry & Motor ---
  if (Receive(HoverSerial, feedback)) {
      cartStatus.voltage = feedback.iVolt / 100.0;
      cartStatus.ampL = feedback.iAmpL / 100.0;
      cartStatus.ampR = feedback.iAmpR / 100.0;
      cartStatus.speedL = feedback.iSpeedL;
      cartStatus.speedR = feedback.iSpeedR;
      cartStatus.odom = feedback.iOdomL;
  }
  cartStatus.isManual = isManualMode; // Send Mode
  
  // Send Target Speed so Remote can display "Set Speed"
  if (isManualMode) {
      cartStatus.cmdSpeed = manualTargetSpeed;
  } else {
      cartStatus.cmdSpeed = finalSpeed;
  }

  if (millis() - lastSend > SEND_INTERVAL_MS) {
      lastSend = millis();
      HoverSend(HoverSerial, finalSteer, finalSpeed);
      if (hasRemote) esp_now_send(remoteAddress, (uint8_t *) &cartStatus, sizeof(cartStatus));
  }

  // --- 6. LED Status ---
  updateLEDs(isManualMode, manualActive, remoteValid, waitingForRemoteZero, cartStatus.voltage);

  // --- 7. Serial Status ---
  if (millis() - lastSerialDebug > 2000) {
      lastSerialDebug = millis();
      Serial.printf("Mode:%s | Act:%d | Set:%d | WaitZero:%d\n", 
                    isManualMode ? "MANUAL" : "REMOTE", manualCurrentSpeed, manualTargetSpeed, waitingForRemoteZero);
  }
}
