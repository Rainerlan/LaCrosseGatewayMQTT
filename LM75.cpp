#include <Wire.h>
#include "LM75.h"


LM75::LM75() {
}

bool LM75::TryInitialize(byte address) {
  bool result = false;
  m_address = address;

  word wd = Read16(LM75_TEMPERATURE_REGISTER);
  if (wd > 0 && wd < 65535) {
    result = true;
  }

  return result;
}

float LM75::GetTemperature() {
  word w = Read16(LM75_TEMPERATURE_REGISTER);
  byte b1 = w >> 8;
  byte b2 = w;
  float t;

  t = b1 & 0b01111111;
  if (b1 & 0b10000000) {
    t -= 127;
  }
  if (b2 & 0b10000000) {
    t += 0.5; 
  }

  m_lastValue.Temperature = t;
  
  return t;
}

LM75Value LM75::GetLastMeasuredValue(){
  return m_lastValue;
}
