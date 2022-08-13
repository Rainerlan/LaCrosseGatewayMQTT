#include "BMP180.h"

BMP180::BMP180() {
}


boolean BMP180::TryInitialize() {
  boolean result = false;

  if (Read8(0xD0) == 0x55) {
    m_compensation.CAC1 = Read16(0xAA);
    m_compensation.CAC2 = Read16(0xAC);
    m_compensation.CAC3 = Read16(0xAE);
    m_compensation.CAC4 = Read16(0xB0);
    m_compensation.CAC5 = Read16(0xB2);
    m_compensation.CAC6 = Read16(0xB4);

    m_compensation.CB1 = Read16(0xB6);
    m_compensation.CB2 = Read16(0xB8);

    m_compensation.CMB = Read16(0xBA);
    m_compensation.CMC = Read16(0xBC);
    m_compensation.CMD = Read16(0xBE);

    result = true;
  }

  return result;
}

void BMP180::SetAltitudeAboveSeaLevel(int32_t altitude) {
  m_altitudeAboveSeaLevel = altitude;
}

int32_t BMP180::CalculateB5(int32_t ut) {
  int32_t x1 = (ut - (int32_t)m_compensation.CAC6) * ((int32_t)m_compensation.CAC5) >> 15;
  int32_t x2 = ((int32_t)m_compensation.CMC << 11) / (x1 + (int32_t)m_compensation.CMD);
  return x1 + x2;
}

uint16_t BMP180::GetRawTemperature(void) {
  Write8(0xF4, 0x2E);
  delay(5);
  uint16_t rt = Read16(0xF6);
  m_lastValue.ADCT = rt;
  return rt;
}

uint32_t BMP180::GetRawPressure(void) {
  uint32_t raw;

  Write8(0xF4, 0x34 + (2 << 6));
  delay(14);

  raw = Read16(0xF6);

  raw <<= 8;
  raw |= Read8(0xF6 + 2);
  raw >>= 6;
  m_lastValue.ADCP = raw;

  return raw;
}


float BMP180::GetPressure(void) {
  int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

  UT = GetRawTemperature();
  UP = GetRawPressure();
  B5 = CalculateB5(UT);

  B6 = B5 - 4000;
  X1 = ((int32_t)m_compensation.CB2 * ((B6 * B6) >> 12)) >> 11;
  X2 = ((int32_t)m_compensation.CAC2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)m_compensation.CAC1 * 4 + X3) << 2) + 2) / 4;
  X1 = ((int32_t)m_compensation.CAC3 * B6) >> 13;
  X2 = ((int32_t)m_compensation.CB1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)m_compensation.CAC4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)(50000UL >> 2);

  if (B7 < 0x80000000) {
    p = (B7 * 2) / B4;
  }
  else {
    p = (B7 / B4) * 2;
  }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

  p = p + ((X1 + X2 + (int32_t)3791) >> 4);
  p /= pow(((float) 1.0 - ((float)m_altitudeAboveSeaLevel / 44330.0)), (float) 5.255);
  p /= 10;

  m_lastValue.Pressure = (float)p / 10.0;

  return m_lastValue.Pressure;
}


float BMP180::GetTemperature(void) {
  int32_t UT, B5;
  float temp;

  UT = GetRawTemperature();

  B5 = CalculateB5(UT);
  temp = (B5 + 8) >> 4;
  temp /= 10.0;
  m_lastValue.Temperature = temp;

  return temp;
}

bmp180_compensation BMP180::GetCompensationValues() {
  return m_compensation;
}

BMP180Value BMP180::GetLastMeasuredValue() {
  return m_lastValue;
}

uint8_t BMP180::Read8(uint8_t a) {
  uint8_t ret;

  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(a);

  Wire.endTransmission();

  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.requestFrom(BMP180_ADDRESS, 1);
  ret = Wire.read();
  Wire.endTransmission();

  return ret;
}

uint16_t BMP180::Read16(uint8_t a) {
  uint16_t ret;

  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(a);
  Wire.endTransmission();

  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.requestFrom(BMP180_ADDRESS, 2);
  ret = Wire.read();
  ret <<= 8;
  ret |= Wire.read();

  Wire.endTransmission();

  return ret;
}

void BMP180::Write8(uint8_t a, uint8_t d) {
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(a);
  Wire.write(d);

  Wire.endTransmission();
}

