#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "AiEsp32RotaryEncoder.h"
#include "util.h"
#include "hoverserial.h"
#include "input.h" 
#include "TrundlerDisplay.h"
#include "communication.h"

// --- Config ---
#define HOVER_BAUD 19200
#define SEND_INTERVAL_MS 20  

// Hardware Serial for Hoverboard (C3 uses pins 20/21)
HardwareSerial HoverSerial(1); 
SerialHover2Server feedback;

// UI Object (CS=7, DC=2, RST=3) - SPI SCK=8, MOSI=10 are hardware-defined
TrundlerDisplay ui(TFT_CS, TFT_DC, TFT_RST);

// Encoder Object (Swapped DT and CLK to invert rotation direction)
AiEsp32RotaryEncoder encoder = AiEsp32RotaryEncoder(ENC_DT, ENC_CLK, ENC_SW, -1, 4);

InputState remoteInputs; 
CartData cartStatus;

bool hasRemote = false;
unsigned long lastRecvTime = 0;

// System States
bool manualActive = false;   
bool isCruiseMode = false;   
int16_t userSpeedLevel = 0;    // Discrete speed level (-10 to 20)
int16_t manualTargetSpeed = 0; 
int16_t manualCurrentSpeed = 0;
float slaveComp = 1.0; 
bool showDevScreen = false; 

// Software Cruise State
int16_t cruiseNudge = 0;

// Tuning Parameters
uint8_t curLimit = 15;  
uint8_t inertia = 10;   

uint8_t remoteMacAddress[6];
bool remotePeerAdded = false;

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(InputState)) {
    memcpy(&remoteInputs, incomingData, sizeof(InputState));
    lastRecvTime = millis();
    if (!hasRemote) {
        Serial.println("Remote Link Established!");
        if (!remotePeerAdded) {
            esp_now_peer_info_t peerInfo;
            memset(&peerInfo, 0, sizeof(peerInfo));
            memcpy(peerInfo.peer_addr, mac, 6);
            peerInfo.channel = 1; 
            peerInfo.encrypt = false;
            if (esp_now_add_peer(&peerInfo) == ESP_OK) {
                memcpy(remoteMacAddress, mac, 6);
                remotePeerAdded = true;
            }
        }
    }
    hasRemote = true;
  } else {
    Serial.printf("!!! Size Mismatch: Expected %d, Recv %d\n", sizeof(InputState), len);
  }
}

void encoder_on_button_click() {
    static unsigned long lastClick = 0;
    if (millis() - lastClick < 300) return;
    lastClick = millis();
    
    manualActive = !manualActive;
    // Always reset speed to 0 when stopped (forward or backward) for safety
    if (!manualActive) {
        userSpeedLevel = 0;
        encoder.setEncoderValue(0);
        manualTargetSpeed = 0;
    }
}

void setup() {
  // --- 1. Safety Boot Sequence ---
  Serial.begin(115200);
  delay(3000); 
  Serial.println("\n\n=== TRUNDLER C3 16-PIN RECOVERY BOOT ===");
  
  // --- 1b. Thermal Management ---
  ledcSetup(0, 5000, 8); 
  ledcAttachPin(TFT_BLK, 0);
  ledcWrite(0, 128); 

  // --- 1c. Wifi/Radio Init ---
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 
  WiFi.setSleep(false);
  esp_wifi_set_max_tx_power(56); 
  
  // Mandatory for ESP-NOW: Set Channel 1 to match the Remote
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  // Print MAC and Channel
  uint8_t mac[6];
  WiFi.macAddress(mac);
  uint8_t chan;
  wifi_second_chan_t sec;
  esp_wifi_get_channel(&chan, &sec);
  Serial.printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X | CH: %d\n", 
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], chan);

  // --- 2. Init Hardware ---
  Serial.println("Initializing TFT...");
  ui.begin();

  Serial.println("Initializing Encoder...");
  encoder.begin();
  encoder.setup([]{ encoder.readEncoder_ISR(); });
  encoder.setBoundaries(0, 20, false); 
  encoder.setAcceleration(0);

  initInput();

  // --- 3. Init Comms ---
  Serial.println("Initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
  }
  esp_now_register_recv_cb(onDataRecv);

  Serial.println("Initializing Hoverboard Serial...");
  HoverSerial.begin(HOVER_BAUD, SERIAL_8N1, HOVER_SERIAL_RX, HOVER_SERIAL_TX); 
  
  Serial.println("System Ready.");
}

void loop() {
  static unsigned long lastSend = 0;
  // --- 1. Inputs ---
  InputState localInputs = readInput();
  
  // Timeout Remote
  if (hasRemote && (millis() - lastRecvTime > 1000)) {
      memset(&remoteInputs, 0, sizeof(remoteInputs));
      hasRemote = false;
  }

  // --- 2. Process Commands ---
  // A. Local Encoder
  if (encoder.encoderChanged()) {
      userSpeedLevel = encoder.readEncoder();
      Serial.printf("Speed Level: %d\n", userSpeedLevel);
  }

  if (encoder.isEncoderButtonClicked()) {
      encoder_on_button_click();
      Serial.printf("Drive Toggle: %s\n", manualActive ? "ON" : "OFF");
  }

  // B. Remote
  static unsigned long lastRemoteAction = 0;
  static bool lastRemoteStopState = false;

  if (hasRemote) {
      // Remote can only change speed when active/driving (for safety when stopped)
      if (manualActive && (millis() - lastRemoteAction > 200)) {
          if (remoteInputs.fwd) { 
              userSpeedLevel = CLAMP(userSpeedLevel + 1, -10, 20); 
              encoder.setEncoderValue(userSpeedLevel);
              lastRemoteAction = millis();
          }
          if (remoteInputs.rev) { 
              userSpeedLevel = CLAMP(userSpeedLevel - 1, -10, 20); 
              encoder.setEncoderValue(userSpeedLevel);
              lastRemoteAction = millis();
          }
      }

      if (remoteInputs.stop && !lastRemoteStopState) {
          encoder_on_button_click();
      }
      lastRemoteStopState = remoteInputs.stop;
  }

  // Merge Steering (Remote Only)
  int16_t turnSteer = remoteInputs.left ? -100 : (remoteInputs.right ? 100 : 0);

  // --- 3. Unified Speed Mapping ---
  if (!manualActive) {
      manualTargetSpeed = 0;
  } else {
      manualTargetSpeed = userSpeedLevel * 17.5f; 
  }

  // --- 4. Ramping ---
  static unsigned long lastMoveRamp = 0;
  if (millis() - lastMoveRamp > 50) {
      lastMoveRamp = millis();
      int16_t target = manualTargetSpeed;
      if (manualCurrentSpeed < target) manualCurrentSpeed += 5;
      else if (manualCurrentSpeed > target) manualCurrentSpeed -= 5;
  }
  
  int16_t driveSpeed = manualCurrentSpeed;

  // --- 5. SOFTWARE CRUISE CONTROL ---
  static unsigned long lastCruiseAdjust = 0;
  if (manualActive && isCruiseMode && abs(manualTargetSpeed) > 50 && turnSteer == 0 && (millis() - lastCruiseAdjust > 100)) {
      lastCruiseAdjust = millis();
      int16_t targetRPM = abs(manualTargetSpeed) * 1.5; 
      int16_t actualRPM = (abs(cartStatus.speedL) + abs(cartStatus.speedR)) / 2;
      if (actualRPM < targetRPM - 15) cruiseNudge += 2;
      else if (actualRPM > targetRPM + 15) cruiseNudge -= 2;
      cruiseNudge = constrain(cruiseNudge, -50, 100); 
  } else if (!isCruiseMode || !manualActive) {
      cruiseNudge = 0;
  }

  // --- 4. Governor ---
  static unsigned long lastCompAdjust = 0;
  if (manualActive && !isCruiseMode && abs(driveSpeed) > 100 && turnSteer == 0 && (millis() - lastCompAdjust > 200)) {
      lastCompAdjust = millis();
      int16_t absL = abs(cartStatus.speedL);
      int16_t absR = abs(cartStatus.speedR);
      if (absR > absL + 20) slaveComp -= 0.002; 
      else if (absR < absL - 20) slaveComp += 0.002; 
      slaveComp = constrain(slaveComp, 0.7, 1.3); 
  }
  
  int16_t finalSpeed = driveSpeed;
  if (isCruiseMode && finalSpeed != 0) {
      if (finalSpeed > 0) finalSpeed += cruiseNudge;
      else finalSpeed -= cruiseNudge;
  }

  int16_t speedL = -(finalSpeed - turnSteer);
  int16_t speedR = (finalSpeed + turnSteer) * slaveComp; 

  uint8_t hoverMode = (manualActive && isCruiseMode) ? 1 : 0;
  
  uint8_t controlState = STATE_BATT_ONLY;
  if (!manualActive || (manualTargetSpeed == 0 && manualCurrentSpeed == 0 && turnSteer == 0)) {
      controlState = STATE_DISABLE;
  }

  // --- 7. RX Telemetry ---
  if (Receive(HoverSerial, feedback)) {
      cartStatus.voltage = feedback.iVolt / 100.0;
      cartStatus.speedL = feedback.iSpeedL;
      cartStatus.speedR = feedback.iSpeedR;
      cartStatus.odom = feedback.iOdomL;
  }

  // --- 8. UI Update ---
  uint8_t rBatt = hasRemote ? remoteInputs.vBatt : 0;
  ui.update(cartStatus, userSpeedLevel, turnSteer, slaveComp, manualActive, isCruiseMode, showDevScreen, hasRemote, rBatt, curLimit, inertia);

  // --- 9. TX Transmission ---
  if (millis() - lastSend > SEND_INTERVAL_MS) {
      lastSend = millis();
      HoverSend(HoverSerial, speedL, speedR, controlState, controlState, hoverMode, curLimit, inertia);
  }

  // --- 10. Send Telemetry back to Remote ---
  static unsigned long lastTelemetrySend = 0;
  if (hasRemote && remotePeerAdded && (millis() - lastTelemetrySend > 100)) {
      lastTelemetrySend = millis();
      cartStatus.isManual = manualActive;
      cartStatus.cmdSpeed = manualTargetSpeed;
      esp_now_send(remoteMacAddress, (uint8_t *) &cartStatus, sizeof(cartStatus));
  }
}
