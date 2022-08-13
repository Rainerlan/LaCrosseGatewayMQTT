#include "BH1750.h"

boolean BH1750::TryInitialize(uint8_t address) {
  boolean result = false;
  m_address = address;
  m_mode = 0x10;
  m_lastValue = -1;
  
  Wire.beginTransmission(m_address);
  Wire.write(m_mode);
  result = Wire.endTransmission() == 0;
  delay(200);

  return result;
}

uint32_t BH1750::GetIlluminance(void) {
  m_lastValue = -1;
  uint32_t result = -1;

  Wire.beginTransmission(m_address);
  Wire.write(m_mode);
  Wire.endTransmission();

  if(Wire.requestFrom(m_address, 2u) == 2) {
    result = 0;
    result = Wire.read();
    result <<= 8;
    result |= Wire.read();
    result /= 1.2;

    m_lastValue = result;
  }

  return result;
}

uint32_t BH1750::GetLastMeasuredValue() {
  return m_lastValue;
}
