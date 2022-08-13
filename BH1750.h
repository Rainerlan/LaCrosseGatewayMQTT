#ifndef BH1750_H
#define BH1750_H

#include "Arduino.h"
#include "Wire.h"

class BH1750 {
public:
  boolean TryInitialize(uint8_t address);
  uint32_t GetIlluminance();
  uint32_t GetLastMeasuredValue();

private:
  uint32_t m_lastValue;
  uint8_t m_address;
  uint8_t m_mode;


};

#endif
