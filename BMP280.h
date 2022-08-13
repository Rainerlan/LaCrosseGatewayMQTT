#ifndef __BMP280_H__
#define __BMP280_H__

#include "Arduino.h"
#include "I2CBase.h"

enum {
  BMP280_REGISTER_T1              = 0x88,
  BMP280_REGISTER_T2              = 0x8A,
  BMP280_REGISTER_T3              = 0x8C,

  BMP280_REGISTER_P1              = 0x8E,
  BMP280_REGISTER_P2              = 0x90,
  BMP280_REGISTER_P3              = 0x92,
  BMP280_REGISTER_P4              = 0x94,
  BMP280_REGISTER_P5              = 0x96,
  BMP280_REGISTER_P6              = 0x98,
  BMP280_REGISTER_P7              = 0x9A,
  BMP280_REGISTER_P8              = 0x9C,
  BMP280_REGISTER_P9              = 0x9E,

  BMP280_REGISTER_CHIPID          = 0xD0,
  BMP280_REGISTER_VERSION         = 0xD1,
  BMP280_REGISTER_SOFTRESET       = 0xE0,

  BMP280_REGISTER_CAL26           = 0xE1,

  BMP280_REGISTER_CONTROLHUMID    = 0xF2,
  BMP280_REGISTER_STATUS          = 0xF3,
  BMP280_REGISTER_CONTROL         = 0xF4,
  BMP280_REGISTER_CONFIG          = 0xF5,
  BMP280_REGISTER_PRESSUREDATA    = 0xF7,
  BMP280_REGISTER_TEMPDATA        = 0xFA,
  BMP280_REGISTER_HUMIDDATA       = 0xFD,
};

// Oversampling Pressure - written to ctrl_meas (<< 2)
enum {
  BMP280_OSRS_Px00  = 0,
  BMP280_OSRS_Px01  = 1,
  BMP280_OSRS_Px02  = 2,
  BMP280_OSRS_Px04  = 3,
  BMP280_OSRS_Px08  = 4,
  BMP280_OSRS_Px16  = 5,
};

// Oversampling Temperature - written to ctrl_meas (<< 5)
enum {
  BMP280_OSRS_Tx00  = 0,
  BMP280_OSRS_Tx01  = 1,
  BMP280_OSRS_Tx02  = 2,
  BMP280_OSRS_Tx04  = 3,
  BMP280_OSRS_Tx08  = 4,
  BMP280_OSRS_Tx16  = 5,
};

// Operation mode - written to ctrl_meas
enum {
  BMP280_MODE_SLEEP    = 0,
  BMP280_MODE_FORCED   = 1,   // Forced mode
  BMP280_MODE_NORMAL   = 3,
};

// Standby setting - written to config (<< 5)
enum {
  BMP280_T_SB_00_5   = 0,
  BMP280_T_SB_62_5   = 1,
  BMP280_T_SB_125    = 2,
  BMP280_T_SB_250    = 3,
  BMP280_T_SB_500    = 4,
  BMP280_T_SB_1000   = 5,
  BMP280_T_SB_10     = 6,
  BMP280_T_SB_20     = 7,
};

// IIR Filter settings - written to config (<< 2)
enum {
  BMP280_FILTER_OFF        = 0,
  BMP280_FILTER_COEF_2     = 1,
  BMP280_FILTER_COEF_4     = 2,
  BMP280_FILTER_COEF_8     = 3,
  BMP280_FILTER_COEF_16    = 4,
};

typedef struct {
  uint16_t T1;
  int16_t  T2;
  int16_t  T3;

  uint16_t P1;
  int16_t  P2;
  int16_t  P3;
  int16_t  P4;
  int16_t  P5;
  int16_t  P6;
  int16_t  P7;
  int16_t  P8;
  int16_t  P9;

} bmp280_compensation;

typedef struct {
  int32_t ADCT;
  int32_t ADCP;

  float Temperature;
  float Pressure;
} BMP280Value;

class BMP280 : public I2CBase {
public:
  BMP280();
  void SetAltitudeAboveSeaLevel(int altitude);
  bool  TryInitialize(byte address);
  float GetTemperature();
  float GetPressure();
  bmp280_compensation GetCompensationValues();
  BMP280Value GetLastMeasuredValue();

protected:
  void ReadCompensation();

  int m_altitudeAboveSeaLevel;
  int32_t m_compensatedTemperature;
  bmp280_compensation m_compensation;
  BMP280Value m_lastValue;

};

#endif
