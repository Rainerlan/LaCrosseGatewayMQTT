#ifndef BMP180_H
#define BMP180_H

#include "Arduino.h"
#include "Wire.h"

#define BMP180_ADDRESS 0x77

typedef struct {
  int16_t CAC1;
  int16_t  CAC2;
  int16_t  CAC3;
  uint16_t  CAC4;
  uint16_t  CAC5;
  uint16_t  CAC6;
  int16_t CB1;
  int16_t CB2;
  int16_t  CMB;
  int16_t  CMC;
  int16_t  CMD;

} bmp180_compensation;

typedef struct {
  int16_t ADCT;
  int32_t ADCP;

  float Temperature;
  float Pressure;
} BMP180Value;

class BMP180 {
public:
  BMP180();
  boolean TryInitialize();
  void SetAltitudeAboveSeaLevel(int32_t altitude);
  float GetTemperature();
  float GetPressure();
  bmp180_compensation GetCompensationValues();
  BMP180Value GetLastMeasuredValue();

private:
  int32_t m_altitudeAboveSeaLevel = 0;
  uint16_t GetRawTemperature();
  uint32_t GetRawPressure();
  int32_t CalculateB5(int32_t ut);
  uint8_t Read8(uint8_t addr);
  uint16_t Read16(uint8_t addr);
  void Write8(uint8_t addr, uint8_t data);

  bmp180_compensation m_compensation;
  BMP180Value m_lastValue;

};

#endif
