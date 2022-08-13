#ifndef __DHTxx__h
#define __DHTxx__h

#include <Arduino.h>

typedef struct {
  float Temperature;
  int Humidity;
} DHTxxValue;

class DHTxx {
public:
  DHTxx();
  bool TryInitialize(byte pin);
  bool TryMeasure();
  float GetTemperature();
  int GetHumidity();
  DHTxxValue GetLastMeasuredValue();
  String GetType();

private:
  byte m_pin;
  byte m_type;
  int m_humidity;
  float m_temperature;
  byte m_data[5];
  DHTxxValue m_lastValue;
  bool Read();
};
#endif
