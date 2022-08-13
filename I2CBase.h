#ifndef _I2CBASE_h
#define _I2CBASE_h

#include "Arduino.h"
#include <Wire.h>

class I2CBase {

protected:
  byte m_address;

  void Write8(byte reg, byte value);
  byte Read8(byte reg);
  uint16_t Read16(byte reg);
  uint16_t Read16_LE(byte reg);
  int16_t ReadS16_LE(byte reg);
  void Read(byte addr, byte count, byte* data);
  void Write(byte addr, byte count, byte* data);

};

#endif

