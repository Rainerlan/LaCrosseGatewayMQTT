#include "SensorBase.h"

bool SensorBase::m_debug = false;

byte SensorBase::CalculateCRC(byte *data, byte len) {
  int i, j;
  byte res = 0;
  for (j = 0; j < len; j++) {
    uint8_t val = data[j];
    for (i = 0; i < 8; i++) {
      uint8_t tmp = (uint8_t)((res ^ val) & 0x80);
      res <<= 1;
      if (0 != tmp) {
        res ^= 0x31;
      }
      val <<= 1;
    }
  }
  return res;
}

uint16_t SensorBase::CalculateCRC16(byte *data, byte len) {
  byte x;
  uint16_t crc = 0xFFFF;

  while(len--) {
    x = crc >> 8 ^ *data++;
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^ ((uint16_t)x);
  }
  return crc;
}

void SensorBase::SetDebugMode(boolean mode) {
  m_debug = mode;
}

