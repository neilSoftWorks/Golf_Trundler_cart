#ifndef HOVERSERIAL_H
#define HOVERSERIAL_H

#include <Arduino.h>
#include "util.h"

#define START_FRAME 0xABCD
#define CMD_START   '/'       // 0x2F - Single byte start for commands

// Received from Hoverboard (Feedback) - 30 bytes
typedef struct __attribute__((packed, aligned(1))) {
   uint16_t cStart;
   int16_t  iSpeedL;   
   int16_t  iSpeedR;   
   uint16_t iVolt;     
   int16_t  iAmpL;     
   int16_t  iAmpR;     
   int32_t  iOdomL;    
   int32_t  iOdomR;    
   uint16_t checksum;
} SerialHover2Server;

// Sent to Hoverboard (Command) - 9 bytes total
// This matches the "old version" in RoboDurden's hoverserial.h
typedef struct __attribute__((packed, aligned(1))) { 
   uint8_t  cStart;       // Single byte '/'
   int16_t  iSpeed;       // -1000 to 1000
   int16_t  iSteer;       // -1000 to 1000
   uint8_t  wStateMaster; 
   uint8_t  wStateSlave;  
   uint16_t checksum;
} SerialServer2Hover;

// CRC16 Calculation
uint16_t CalcCRC(uint8_t *ptr, int count) {
  uint16_t  crc;
  uint8_t i;
  crc = 0;
  while (--count >= 0) {
    crc = crc ^ (uint16_t) *ptr++ << 8;
    i = 8;
    do {
      if (crc & 0x8000) {
        crc = crc << 1 ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    } while(--i);
  }
  return (crc);
}

// Helper to send data
template <typename O> 
void HoverSend(O& oSerial, int16_t iSteer, int16_t iSpeed, uint8_t wStateMaster=32, uint8_t wStateSlave=32) {
  SerialServer2Hover oData;
  oData.cStart = CMD_START; // Use '/'
  oData.iSpeed = iSpeed;
  oData.iSteer = iSteer;
  oData.wStateMaster = wStateMaster;
  oData.wStateSlave  = wStateSlave;
  oData.checksum = CalcCRC((uint8_t*)&oData, sizeof(SerialServer2Hover)-2); 
  
  oSerial.write((uint8_t*) &oData, sizeof(SerialServer2Hover)); 
}

// Helper to receive data
template <typename O> 
boolean Receive(O& oSerial, SerialHover2Server& Feedback) {
  if (oSerial.available() < sizeof(SerialHover2Server)) return false;

  // Sync to 0xABCD
  while (oSerial.available() >= 2) {
      if (oSerial.peek() != 0xCD) { // LSB of 0xABCD
          oSerial.read();
          continue;
      }
      break;
  }
  
  if (oSerial.available() < sizeof(SerialHover2Server)) return false;

  uint8_t buffer[sizeof(SerialHover2Server)];
  oSerial.readBytes(buffer, sizeof(SerialHover2Server));
  
  SerialHover2Server* pData = (SerialHover2Server*)buffer;
  
  if (pData->cStart != START_FRAME) return false;

  uint16_t calcChecksum = CalcCRC(buffer, sizeof(SerialHover2Server)-2);
  if (calcChecksum == pData->checksum) {
      memcpy(&Feedback, pData, sizeof(SerialHover2Server));
      return true;
  }
  return false;
}

#endif // HOVERSERIAL_H