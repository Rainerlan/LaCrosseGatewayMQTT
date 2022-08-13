#include "Arduino.h"
#include "BME680.h"

bool BME680::TryInitialize(uint8_t address) {
  bool result = false;
  m_address = address;
  m_lastValue.Temperature = 0.0;
  m_lastValue.Humidity = 0;
  m_lastValue.Pressure = 0;
  m_lastValue.Gas = 0;
  m_lastValue.IsValid = false;
  m_hasMeasured = false;
  m_lastMeasurement = 0;
  m_mode = DeviceMode::Forced;
  m_pressureOverSampling = OverSamplingRate::OSR_08;
  m_temperatureOverSampling = OverSamplingRate::OSR_04;
  m_humidityOverSampling = OverSamplingRate::OSR_02;
  m_filterSize = FilterSize::Size_3;
  m_heaterTemperature = 320;
  m_heaterDuration = 197;

  Write8(BME680_CTRL_RESET, BME680_CMD_RESET);
  delay(100);
  uint8_t id = Read8(BME680_CTRL_ID);
  
  if (id == 0x61) {
    uint8_t calib[41] = { 0 };
    Read(BME680_DATA_CALIB_1, 25, &calib[0]);
    Read(BME680_DATA_CALIB_2, 16, &calib[25]);
    m_compensation.T1 =  (uint16_t)(((uint16_t)calib[34] << 8) | calib[33]);
    m_compensation.T2 =  (int16_t)(((int16_t)calib[2] << 8) | calib[1]);
    m_compensation.T3 =  (int8_t)(calib[3]);
    m_compensation.P1 =  (uint16_t)(((uint16_t)calib[6] << 8) | calib[5]);
    m_compensation.P2 =  (int16_t)(((int16_t)calib[8] << 8) | calib[7]);
    m_compensation.P3 =  (int8_t)(calib[9]);
    m_compensation.P4 =  (int16_t)(((int16_t)calib[12] << 8) | calib[11]);
    m_compensation.P5 =  (int16_t)(((int16_t)calib[14] << 8) | calib[13]);
    m_compensation.P6 =  (int8_t)(calib[16]);
    m_compensation.P7 =  (int8_t)(calib[15]);
    m_compensation.P8 =  (int16_t)(((int16_t)calib[20] << 8) | calib[19]);
    m_compensation.P9 =  (int16_t)(((int16_t)calib[22] << 8) | calib[21]);
    m_compensation.P10 = (uint8_t)(calib[23]);
    m_compensation.H1 =  (uint16_t)(((uint16_t)calib[27] << 4) | (calib[26] & 0x0F));
    m_compensation.H2 =  (uint16_t)(((uint16_t)calib[25] << 4) | (calib[26] >> 4));
    m_compensation.H3 =  (int8_t)calib[28];
    m_compensation.H4 =  (int8_t)calib[29];
    m_compensation.H5 =  (int8_t)calib[30];
    m_compensation.H6 =  (uint8_t)calib[31];
    m_compensation.H7 =  (int8_t)calib[32];
    m_compensation.G1 =  (uint8_t)calib[37];
    m_compensation.G2 =  (uint16_t)(((uint16_t)calib[36] << 8) | (uint16_t)calib[35]);
    m_compensation.G3 =  (uint8_t)calib[38];

    m_compensation.HRR = ((Read8(BME680_DATA_HEAT_RANGE) & BME680_MASK_HEAT_RANGE) / 16);
    m_compensation.HRV = (int8_t)Read8(BME680_DATA_HEAT_VAL);
    m_compensation.SWE = ((int8_t)Read8 (BME680_DATA_SW_ERR) & BME680_MASK_SW_ERR) / 16;


    Write8(BME680_CTRL_GAS_0, 0);
    Write8(BME680_CTRL_GAS_1, BME680_CMD_RUN_GAS | BME680_CMD_HSP_0);
    Write8(BME680_CTRL_GAS_WAIT, CalculateHeaterDuration(m_heaterDuration));
    Write8(BME680_CTRL_RES_HEAT, CalculateHeaterResistance(m_heaterTemperature, 25));
    Write8(BME680_CTRL_CONFIG, m_filterSize << 2);


    Measure(true);
    CalculateTemperature();
    CalculateHumidity();
    CalculatePressure();
    CalculateGas();

    result = true;
  }

  return result;
}

float BME680::GetTemperature() {
  return m_lastValue.Temperature;
}

float BME680::GetHumidity() {
  return m_lastValue.Humidity;
}

float BME680::GetPressure() {
  return m_lastValue.Pressure;
}

float BME680::GetGas() {
  return m_lastValue.Gas;
}

void BME680::Measure(bool waitForResult) {
  
  
  Write8(BME680_CTRL_HUM, m_humidityOverSampling);
  Write8 (BME680_CTRL_GAS_WAIT, CalculateHeaterDuration (m_heaterDuration));
  Write8 (BME680_CTRL_RES_HEAT, CalculateHeaterResistance (m_heaterTemperature, 25));

  Write8 (BME680_CTRL_MEAS, m_temperatureOverSampling << 5 | m_pressureOverSampling << 2 | m_mode);

  if(waitForResult) {
    bool isMeasuring = true;
    unsigned long startTime = millis();
    while (isMeasuring && millis() - startTime < 250) {
      delay(1);
      uint8_t status = Read8(BME680_STATUS_MEAS_0);
      isMeasuring = bitRead(status, 5);
    }
  }
  
}

void BME680::Handle() {
  if(millis() < m_lastMeasurement) {
    m_lastMeasurement = 0;
  }
  if(millis() > m_lastMeasurement + 3000) {
    uint8_t status = Read8(BME680_STATUS_MEAS_0);
    uint8_t gasLsb = Read8(BME680_DATA_GAS_LSB);

    ////bool isMeasuring = bitRead(status,5);
    ////bool isGasMeasuring = bitRead(status, 6);
    ////bool newData = bitRead(status, 7);
    ////uint8_t gasMeasureIndex = status && 0x0F;
    ////bool gasIsValid = bitRead(gasLsb, 5);
    ////bool heatIsStable = bitRead(gasLsb, 4);
    ////Serial.print("IM:");
    ////Serial.print(isMeasuring);
    ////Serial.print(" ND:");
    ////Serial.print(newData);
    ////Serial.print(" GM:");
    ////Serial.print(isGasMeasuring);
    ////Serial.print(" GV:");
    ////Serial.print(gasIsValid);
    ////Serial.print(" HS");
    ////Serial.print(heatIsStable);
    ////Serial.print(" GI");
    ////Serial.print(gasMeasureIndex);
    ////Serial.println();

    //// depends on what?
    m_lastValue.IsValid = true;

    CalculateTemperature();
    CalculateHumidity();
    CalculatePressure();
    CalculateGas();

    Measure(false);
    m_lastMeasurement = millis();
  }
}

void BME680::SetAltitude(int altitude) {
  m_altitude = altitude;
}

BME680Value BME680::GetLastMeasuredValue() {
  return m_lastValue;
}

void BME680::CalculateTemperature() {
  uint8_t adcReadings[3];
  Read(BME680_DATA_TEMP_MSB, 3, &adcReadings[0]);
  uint32_t temp_adc = (uint32_t) (((uint32_t)adcReadings[0] << 12) | ((uint32_t)adcReadings[1] << 4) | ((uint32_t)adcReadings[2] >> 4) & 0b00001111);
 
  int64_t var1, var2;
  var1 = ((((temp_adc >> 3) - ((int32_t)m_compensation.T1 << 1))) * ((int32_t)m_compensation.T2)) >> 11;
  var2 = (((((temp_adc >> 4) - ((int32_t)m_compensation.T1)) * ((temp_adc >> 4) - ((int32_t)m_compensation.T1))) >> 12) * ((int32_t)m_compensation.T3)) >> 14;
  m_rawTemperature = var1 + var2;

  int16_t temperature = (int16_t) (((m_rawTemperature * 5) + 128) >> 8);  
  m_lastValue.Temperature = ((float)(temperature / 10)) / 10.0;
 }

void BME680::CalculateHumidity() {
  uint8_t adcReadings[2];
  Read(BME680_DATA_HUM_MSB, 2, &adcReadings[0]);
  uint16_t hum_adc = (uint16_t) (((uint16_t)adcReadings[0] << 8 | ((uint16_t)adcReadings[1])) );
  int32_t temp_scaled = (((int32_t) m_rawTemperature * 5) + 128) >> 8;
  int32_t var1 = (int32_t) (hum_adc - ((int32_t) ((int32_t) m_compensation.H1 << 4))) - (((temp_scaled * (int32_t) m_compensation.H3) / ((int32_t) 100)) >> 1);
  int32_t var2 = ((int32_t) m_compensation.H2 * (((temp_scaled * (int32_t) m_compensation.H4) / ((int32_t) 100)) + (((temp_scaled * ((temp_scaled * (int32_t) m_compensation.H5) / ((int32_t) 100))) >> 6) / ((int32_t) 100)) + (int32_t) (1 << 14))) >> 10;
  int32_t var3 = var1 * var2;
  int32_t var4 = (int32_t) m_compensation.H6 << 7;
  var4 = ((var4) + ((temp_scaled * (int32_t) m_compensation.H7) / ((int32_t) 100))) >> 4;
  int32_t var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
  int32_t var6 = (var4 * var5) >> 1;
  float humidity = (((var3 + var6) >> 10) * ((int32_t) 1000)) >> 12;
  
  m_lastValue.Humidity = ((float)(humidity / 100)) / 10.0;

  if (m_lastValue.Humidity > 100) {
    m_lastValue.Humidity = 100;
  }
  else if (m_lastValue.Humidity < 0) {
    m_lastValue.Humidity = 0;
  }

 }


void BME680::CalculatePressure() {
  uint8_t adcReadings[3];
  Read(BME680_DATA_PRESS_MSB, 3, &adcReadings[0]);
  uint32_t pres_adc = (uint32_t)(((uint32_t)adcReadings[0] << 12) | ((uint32_t)adcReadings[1] << 4) | ((uint32_t)adcReadings[2] >> 4) & 0b00001111);

  int32_t var1 = (((int32_t)m_rawTemperature) >> 1) - 64000;
  int32_t var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)m_compensation.P6) >> 2;
  var2 = var2 + ((var1 * (int32_t)m_compensation.P5) << 1);
  var2 = (var2 >> 2) + ((int32_t)m_compensation.P4 << 16);
  var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)m_compensation.P3 << 5)) >> 3) + (((int32_t)m_compensation.P2 * var1) >> 1);
  var1 = var1 >> 18;
  var1 = ((32768 + var1) * (int32_t)m_compensation.P1) >> 15;
  int32_t pressure = 1048576 - pres_adc;
  pressure = (int32_t)((pressure - (var2 >> 12)) * ((uint32_t)3125));
  int32_t var4 = (1 << 31);

  if (pressure >= var4) {
    pressure = ((pressure / (uint32_t)var1) << 1);
  }
  else {
    pressure = ((pressure << 1) / (uint32_t)var1);
  }
  var1 = ((int32_t)m_compensation.P9 * (int32_t)(((pressure >> 3) * (pressure >> 3)) >> 13)) >> 12;
  var2 = ((int32_t)(pressure >> 2) * (int32_t)m_compensation.P8) >> 13;
  int32_t var3 = ((int32_t)(pressure >> 8) * (int32_t)(pressure >> 8) * (int32_t)(pressure >> 8) * (int32_t)m_compensation.P10) >> 17;
  pressure = (int32_t)(pressure)+((var1 + var2 + var3 + ((int32_t)m_compensation.P7 << 7)) >> 4);

  pressure /= pow((float)((float) 1.0 - ((float)m_altitude / 44330.0)), (float) 5.255);

  m_lastValue.Pressure = (float)pressure / 100.0;

}

void BME680::CalculateGas() {
  uint8_t adcReadings[2];
  Read(BME680_DATA_GAS_MSB, 2, adcReadings);
  m_lastValue.GasADC = (int16_t)((int16_t)adcReadings[0] << 2 | (int16_t)(adcReadings[1] && BME680_MASK_GAS_LSB) >> 6);
  m_lastValue.GasRange = adcReadings[1] & BME680_MASK_GAS_RANGE;

  int64_t var1 = (int64_t)((1340 + (5 * (int64_t)m_compensation.SWE)) * ((int64_t)m_gasRanges1[m_lastValue.GasRange])) / 65536;
  int64_t var2 = (((int64_t)((int64_t)m_lastValue.GasADC * 32768) - (int64_t)(16777216)) + var1);
  int64_t var3 = (((int64_t)m_gasRanges2[m_lastValue.GasRange] * (int64_t)var1) / 512);
  uint32_t gas_res = (uint32_t)((var3 + ((int64_t)var2 / 2)) / (int64_t)var2);

  m_lastValue.Gas = gas_res;

}

uint8_t BME680::CalculateHeaterResistance(uint16_t heaterTemperature, uint16_t ambientTemperature) {
  int32_t var1 = (((int32_t)ambientTemperature * m_compensation.G3) / 1000) * 256;
  int32_t var2 = (m_compensation.G1 + 784) * (((((m_compensation.G2 + 154009) * heaterTemperature * 5) / 100) + 3276800) / 10);
  int32_t var3 = var1 + (var2 / 2);
  int32_t var4 = (var3 / (m_compensation.HRR + 4));
  int32_t var5 = (131 * m_compensation.HRV) + 65536;
  int32_t heatr_res_x100 = (int32_t)(((var4 / var5) - 250) * 34);
  uint8_t result = (uint8_t)((heatr_res_x100 + 50) / 100);
  return result;

  ////double var1 = 0;
  ////double var2 = 0;
  ////double var3 = 0;
  ////double var4 = 0;
  ////double var5 = 0;
  ////double res_heat = 0;
  ////var1 = (((double)m_compensation.G1 / (16.0)) + 49.0);
  ////var2 = ((((double)m_compensation.G2 / (32768.0)) * (0.0005)) + 0.00235);
  ////var3 = ((double)m_compensation.G3 / (1024.0));
  ////var4 = (var1 * (1.0 + (var2 * (double)heaterTemperature)));
  ////var5 = (var4 + (var3 * (double)ambientTemperature));
  ////res_heat = (uint8_t)(3.4 * ((var5 * (4 / (4 + (double)m_compensation.HRR)) * (1 / (1 + ((double)m_compensation.HRV * 0.002)))) - 25));

  ////return res_heat;


}

uint8_t BME680::CalculateHeaterDuration(uint16_t duration) {
  uint8_t factor = 0;
  uint8_t result;

  if (duration >= 0xFC0) {
    result = 0xFF;
  }
  else {
    while (duration > 0x3F) {
      duration /= 4;
      factor += 1;
    }
    result = (uint8_t)(duration + (factor * 64));
  }

  return result;
}















