#ifndef HOVERSERIAL_H
#define HOVERSERIAL_H

#include <Arduino.h>
#include "util.h"

#define START_FRAME 0xABCD
#define CMD_START   '/'       

// Control States
#define STATE_LED_GREEN    1
#define STATE_LED_ORANGE   2
#define STATE_LED_RED      4
#define STATE_LED_UP       8
#define STATE_LED_DOWN     16
#define STATE_BATT_ONLY    32  
#define STATE_DISABLE      64  
#define STATE_SHUTDOWN     128

// Sent to Hoverboard (Exactly 12 bytes)
typedef struct __attribute__((packed, aligned(1))) {
   uint8_t  cStart;       
   int16_t  speedL;        // Direct Wheel Control
   int16_t  speedR;        
   uint8_t  wStateMaster; 
   uint8_t  wStateSlave;  
   uint8_t  iMode;         // 0=PWM, 1=Speed, 2=Torque, 3=Odom
   uint8_t  iCurLimit;     // DC Current Limit (Amps)
   uint8_t  iInertia;      // Filter Shift (Smoothness)
   uint16_t checksum;     
} SerialMaster2Slave;

// Received from Hoverboard (Exactly 24 bytes)
typedef struct __attribute__((packed, aligned(1))) {
   uint16_t cStart;   
   int16_t  iSpeedL;   
   int16_t  iSpeedR;   
   uint16_t iVolt;     
   int16_t  iAmpL;     
   int16_t  iAmpR;     
   int32_t  iOdomL;    
   int32_t  iOdomR;    
   uint8_t  iVerM;     
   uint8_t  iVerS;     
   uint16_t checksum;  
} SerialHover2Server;

// --- CRC Function ---
uint16_t CalcCRC(uint8_t *ptr, int count) {
  uint16_t crc = 0;
  uint8_t i;
  while (--count >= 0) {
    crc = crc ^ (uint16_t) *ptr++ << 8;
    i = 8;
    do {
      if (crc & 0x8000) crc = crc << 1 ^ 0x1021;
      else crc = crc << 1;
    } while(--i);
  }
  return (crc);
}

void HoverSend(Stream& serial, int16_t speedL, int16_t speedR, uint8_t stateMaster, uint8_t stateSlave, uint8_t iMode, uint8_t iCurLimit, uint8_t iInertia) {
  SerialMaster2Slave oData;
  oData.cStart = CMD_START;
  oData.speedL = speedL;
  oData.speedR = speedR;
  oData.wStateMaster = stateMaster;
  oData.wStateSlave = stateSlave;
  oData.iMode = iMode;
  oData.iCurLimit = iCurLimit;
  oData.iInertia = iInertia;
  oData.checksum = CalcCRC((uint8_t*)&oData, sizeof(SerialMaster2Slave) - 2);
  serial.write((uint8_t*)&oData, sizeof(SerialMaster2Slave));
}

bool Receive(Stream& serial, SerialHover2Server& Feedback) {
  while (serial.available() >= 24) { 
    uint8_t b = serial.peek();
    if (b != 0xCD && b != 0xAB) { 
        serial.read(); 
        continue;
    }
    uint8_t buffer[24];
    serial.readBytes(buffer, 24);
    SerialHover2Server* pData = (SerialHover2Server*)buffer;
    uint16_t calcChecksum = CalcCRC(buffer, 22);
    if (calcChecksum == pData->checksum) {
        memcpy(&Feedback, pData, 24);
        return true;
    }
  }
  return false;
}

#endif
