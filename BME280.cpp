#include "Arduino.h"
#include <Wire.h>
#include "BME280.h"

BME280::BME280() { 
  
}

void BME280::SetAltitudeAboveSeaLevel(int altitude) {
  m_altitudeAboveSeaLevel = altitude;
}

bool BME280::TryInitialize(byte address) {
  bool result = false;
  m_address = address;

  m_lastValue.Temperature = 1;
  m_lastValue.Humidity = 2;
  m_lastValue.Pressure = 3;


// BME280_REGISTER_CONTROLHUMID == ctrl_hum  == 0xF2
// BME280_REGISTER_STATUS       == status    == 0xF3
// BME280_REGISTER_CONTROL      == ctrl_meas == 0xF4
// BME280_REGISTER_CONFIG       == config    == 0xF5
//
#define OSRS_H BME280_OSRS_Hx01         // Oversampling Humidity - ctrl_hum
#define OSRS_P BME280_OSRS_Px01         // Oversampling Pressure - ctrl_meas (<< 2)
#define OSRS_T BME280_OSRS_Tx01         // Oversampling Temperature - ctrl_meas (<< 5)
#define T_SB   BME280_T_SB_1000         // Standby setting - config (<< 5)
#define FILTER_COEF BME280_FILTER_COEF_4  // IIR Filter settings - config (<< 2)

  if (Read8(BME280_REGISTER_CHIPID) == 0x60) {
    ReadCompensation();
    // Set Standby and IIR Filter
    Write8(BME280_REGISTER_CONTROL, BME280_MODE_SLEEP);
    Write8(BME280_REGISTER_CONFIG, ( (T_SB << 5) | (FILTER_COEF << 2) ));

    // Set Oversampling and OpMode
    Write8(BME280_REGISTER_CONTROLHUMID, OSRS_H);       // Measurement starts in GetTemperature()
    
    GetTemperature();       // GetTemperatue: MODE_FORCED, poll for Measurement finished
    if (GetPressure() > 0) {
      GetHumidity();
      result = true;
    }
  } 
  return result;
}

void BME280::ReadCompensation() {
  m_compensation.T1 = Read16_LE(BME280_REGISTER_T1);
  m_compensation.T2 = ReadS16_LE(BME280_REGISTER_T2);
  m_compensation.T3 = ReadS16_LE(BME280_REGISTER_T3);

  m_compensation.P1 = Read16_LE(BME280_REGISTER_P1);
  m_compensation.P2 = ReadS16_LE(BME280_REGISTER_P2);
  m_compensation.P3 = ReadS16_LE(BME280_REGISTER_P3);
  m_compensation.P4 = ReadS16_LE(BME280_REGISTER_P4);
  m_compensation.P5 = ReadS16_LE(BME280_REGISTER_P5);
  m_compensation.P6 = ReadS16_LE(BME280_REGISTER_P6);
  m_compensation.P7 = ReadS16_LE(BME280_REGISTER_P7);
  m_compensation.P8 = ReadS16_LE(BME280_REGISTER_P8);
  m_compensation.P9 = ReadS16_LE(BME280_REGISTER_P9);

  m_compensation.H1 = Read8(BME280_REGISTER_H1);
  m_compensation.H2 = ReadS16_LE(BME280_REGISTER_H2);
  m_compensation.H3 = Read8(BME280_REGISTER_H3);
  m_compensation.H4 = (Read8(BME280_REGISTER_H4) << 4) | (Read8(BME280_REGISTER_H4 + 1) & 0xF);
  m_compensation.H5 = (Read8(BME280_REGISTER_H5 + 1) << 4) | (Read8(BME280_REGISTER_H5) >> 4);
  m_compensation.H6 = (int8_t)Read8(BME280_REGISTER_H6);
}

bme280_compensation BME280::GetCompensationValues() {
  return m_compensation;
}

BME280Value BME280::GetLastMeasuredValue() {
  return m_lastValue;
}

float BME280::GetTemperature() {
  int32_t var1, var2;

// measurement time
#define T_MEASURE ( 1 + (2 * (OSRS_T)) + (2 * (OSRS_P)) + (2 * (OSRS_H)))

    Write8(BME280_REGISTER_CONTROL, ( (OSRS_T << 5) | (OSRS_P << 2) | BME280_MODE_FORCED ));
    do {
      delay(1);     // 1ms delay
    } while ((1 << 3) & Read8(BME280_REGISTER_STATUS));     // until measurement finished
    
  int32_t adc_T = Read16(BME280_REGISTER_TEMPDATA);
  adc_T <<= 8;
  adc_T |= Read8(BME280_REGISTER_TEMPDATA + 2);
  adc_T >>= 4;
  m_lastValue.ADCT = adc_T;

  var1 = ((((adc_T >> 3) - ((int32_t)m_compensation.T1 << 1))) * ((int32_t)m_compensation.T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((int32_t)m_compensation.T1)) * ((adc_T >> 4) - ((int32_t)m_compensation.T1))) >> 12) * ((int32_t)m_compensation.T3)) >> 14;

  m_compensatedTemperature = var1 + var2;

  float T = (m_compensatedTemperature * 5 + 128) >> 8;
  
  m_lastValue.Temperature = T / 100.0;    // 2 digits precision

  T = round(T / 10) / 10.0;
  return T;
}

float BME280::GetPressure() {
  int64_t var1, var2, p;

  int32_t adc_P = Read16(BME280_REGISTER_PRESSUREDATA);
  adc_P <<= 8;
  adc_P |= Read8(BME280_REGISTER_PRESSUREDATA+2);
  adc_P >>= 4;
  m_lastValue.ADCP = adc_P;

  var1 = ((int64_t)m_compensatedTemperature) - 128000;
  var2 = var1 * var1 * (int64_t)m_compensation.P6;
  var2 = var2 + ((var1*(int64_t)m_compensation.P5) << 17);
  var2 = var2 + (((int64_t)m_compensation.P4) << 35);
  var1 = ((var1 * var1 * (int64_t)m_compensation.P3) >> 8) + ((var1 * (int64_t)m_compensation.P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1))*((int64_t)m_compensation.P1) >> 33;

  if (var1 == 0) {
    return 0;  
  }
  p = 1048576 - adc_P;
  p = (((p<<31) - var2)*3125) / var1;
  var1 = (((int64_t)m_compensation.P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)m_compensation.P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)m_compensation.P7) << 4);
  
  p = (float)p / 256;
  p /= pow(((float) 1.0 - ((float)m_altitudeAboveSeaLevel / 44330.0)), (float) 5.255);
  p /= 10.0;

  m_lastValue.Pressure = (float)p / 10.0;

  return m_lastValue.Pressure;
}


int BME280::GetHumidity() {
  int32_t adc_H = Read16(BME280_REGISTER_HUMIDDATA);
  m_lastValue.ADCH = adc_H;

  int32_t v_x1_u32r;

  v_x1_u32r = (m_compensatedTemperature - ((int32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)m_compensation.H4) << 20) - (((int32_t)m_compensation.H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)m_compensation.H6)) >> 10) * (((v_x1_u32r * ((int32_t)m_compensation.H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)m_compensation.H2) + 8192) >> 14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)m_compensation.H1)) >> 4));
  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  float h = (v_x1_u32r>>12);

  m_lastValue.Humidity = h / 1024;

  return  m_lastValue.Humidity;
}

