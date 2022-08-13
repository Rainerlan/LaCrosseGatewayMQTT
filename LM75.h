#ifndef __LM75_h__
#define __LM75_h__

#include "Arduino.h"
#include "I2CBase.h"

#define LM75_TEMPERATURE_REGISTER 0

typedef struct {
  float Temperature;
} LM75Value;

class LM75 : public I2CBase {
public:
  LM75();
  bool TryInitialize(byte address);
  float GetTemperature();
  LM75Value GetLastMeasuredValue();

private:
  LM75Value m_lastValue;

};

#endif
