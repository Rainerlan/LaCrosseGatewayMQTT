#include "I2CBase.h"

void I2CBase::Write8(byte reg, byte value) {
  Wire.beginTransmission(m_address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

byte I2CBase::Read8(byte reg) {
  uint8_t value;

  Wire.beginTransmission(m_address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(m_address, (byte)1);
  value = Wire.read();
  Wire.endTransmission();

  return value;
}

uint16_t I2CBase::Read16(byte reg) {
  uint16_t value;

  Wire.beginTransmission(m_address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(m_address, (byte)2);
  value = (Wire.read() << 8) | Wire.read();
  Wire.endTransmission();

  return value;
}

uint16_t I2CBase::Read16_LE(byte reg) {
  uint16_t temp = Read16(reg);
  return (temp >> 8) | (temp << 8);
}

int16_t I2CBase::ReadS16_LE(byte reg){
  return (int16_t)Read16_LE(reg);
}

void I2CBase::Read(byte reg, byte count, byte* pBuf) {
  Wire.beginTransmission(m_address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(m_address, count);
  while (Wire.available()) {
    *pBuf = Wire.read();
    pBuf++;
  }
}

void I2CBase::Write(byte reg, byte count, byte* pBuf) {
  Wire.beginTransmission(m_address);
  Wire.write(reg);
  byte i = 0;
  while(i < count) {
    Wire.write(*pBuf);
    pBuf++;
    i++;
  }
  Wire.endTransmission();
}