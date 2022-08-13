#ifndef _ANALOGPORT_h
#define _ANALOGPORT_h

#include "Arduino.h"

class AnalogPort {
public: 
  AnalogPort();
  bool TryInitialize(unsigned int u1023);
  String GetFhemDataString();
  word GetLastValue();
  bool IsEnabled();
  unsigned int GetU1023();


protected:
  unsigned long m_lastMeasurement;
  word m_lastValue;
  bool m_enabled;
  unsigned int m_u1023;
};


#endif

