#ifndef _BME680_h
#define _BME680_h

#include "Arduino.h"
#include "Wire.h"
#include "I2CBase.h"

// control registers
#define BME680_CTRL_GAS_0               0x70  
#define BME680_CTRL_GAS_1               0x71
#define BME680_CTRL_HUM                 0x72  
#define BME680_CTRL_STATUS              0x73
#define BME680_CTRL_MEAS                0x74  
#define BME680_CTRL_CONFIG              0x75  
#define BME680_CTRL_IDAC_HEAT           0x50
#define BME680_CTRL_RES_HEAT            0x5A
#define BME680_CTRL_GAS_WAIT            0x64
#define BME680_CTRL_ID                  0xD0
#define BME680_CTRL_RESET               0xE0  

// data registers
#define BME680_DATA_PRESS_MSB           0x1F
#define BME680_DATA_PRESS_LSB           0x20
#define BME680_DATA_PRESS_XLSB          0x21
#define BME680_DATA_TEMP_MSB            0x22
#define BME680_DATA_TEMP_LSB            0x23
#define BME680_DATA_TEMP_XLSB           0x24
#define BME680_DATA_HUM_MSB             0x25
#define BME680_DATA_HUM_LSB             0x26
#define BME680_DATA_GAS_MSB             0x2A
#define BME680_DATA_GAS_LSB             0x2B
#define BME680_DATA_CALIB_1             0x89
#define BME680_DATA_CALIB_2             0xE1
#define BME680_DATA_SW_ERR              0x04
#define BME680_DATA_HEAT_RANGE          0x02
#define BME680_DATA_HEAT_VAL            0x00


// status registers
#define BME680_STATUS_MEAS_0            0x1D

// commands
#define BME680_CMD_RESET                0xB6
#define BME680_CMD_RUN_GAS              0x10
#define BME680_CMD_HSP_0                0x00

// Masks
#define BME680_MASK_GAS_RANGE           0x0F
#define BME680_MASK_SW_ERR              0xF0
#define BME680_MASK_GAS_VALID           0x20
#define BME680_MASK_HEAT_STABLE         0x10
#define BME680_MASK_OSH                 0x07
#define BME680_MASK_GAS_LSB             0xC0
#define BME680_MASK_HEAT_RANGE          0x30


enum OverSamplingRate {
  OSR_00 = 0,
  OSR_01,
  OSR_02,
  OSR_04,
  OSR_08,
  OSR_16
};

enum DeviceMode {
  Sleep = 0,
  Forced,
  Parallel,
  Sequential
};

enum FilterSize {
  Size_0 = 0,
  Size_1,
  Size_3,
  Size_7,
  Size_15,
  Size_31,
  Size_63,
  Size_127
};

typedef struct {
  float Temperature;
  float Humidity;
  float Pressure;
  float Gas;
  bool IsValid;
  int16_t GasADC;
  int8_t GasRange;
} BME680Value;

typedef struct {
  uint16_t T1;
  int16_t  T2;
  int8_t   T3;

  uint16_t P1;
  int16_t  P2;
  int8_t   P3;
  int16_t  P4;
  int16_t  P5;
  int8_t   P6;
  int8_t   P7;
  int16_t  P8;
  int16_t  P9;
  uint8_t  P10;

  uint16_t H1;
  uint16_t H2;
  int8_t   H3;
  int8_t   H4;
  int8_t   H5;
  uint8_t  H6;
  int8_t   H7;

  uint8_t   G1;
  uint16_t  G2;
  uint8_t   G3;

  uint8_t   HRR;
  int8_t    HRV;
  int8_t    SWE;

} bme680_compensation;

class BME680 : public I2CBase {
private:
  int m_altitude;
  unsigned long m_lastMeasurement;
  BME680Value m_lastValue;
  int32_t m_rawTemperature;
  bme680_compensation m_compensation;
  bool m_hasMeasured;
  uint32_t m_gasRanges1[16] = { 2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2126008810, 2147483647, 2130303777, 2147483647, 2147483647, 2143188679, 2136746228, 2147483647, 2126008810, 2147483647, 2147483647 };
  uint32_t m_gasRanges2[16] = { 4096000000, 2048000000, 1024000000, 512000000,  255744255,  127110228,  64000000,   32258064, 16016016,   8000000,    4000000,    2000000,    1000000,    500000,     250000,     125000 };
  DeviceMode m_mode = DeviceMode::Forced;
  uint8_t m_pressureOverSampling = OverSamplingRate::OSR_08;
  uint8_t m_temperatureOverSampling = OverSamplingRate::OSR_04;
  uint8_t m_humidityOverSampling = OverSamplingRate::OSR_02;
  uint16_t m_heaterTemperature = 200;
  uint16_t m_heaterDuration = 150;
  uint8_t m_filterSize = 0;

  void Measure(bool waitForResult);
  void CalculateTemperature();
  void CalculateHumidity();
  void CalculatePressure();
  void CalculateGas();
  uint8_t CalculateHeaterResistance(uint16_t heaterTemperature, uint16_t ambientTemperature);
  uint8_t CalculateHeaterDuration(uint16_t duration);
  
public:
  bool TryInitialize(uint8_t address);
  void Handle();
  void SetAltitude(int altitude);
  float GetTemperature();
  float GetHumidity();
  float GetPressure();
  float GetGas();
  BME680Value GetLastMeasuredValue();
  
};

extern "C" double pow(double, double);

#endif