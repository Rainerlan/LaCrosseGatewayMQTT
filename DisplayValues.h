#ifndef _DISPLAY_VALUES_h
#define _DISPLAY_VALUES_h

#include "Arduino.h"

class DisplayValues {
public:
  String FhemText1;
  String FhemText2;
  String FhemText3;
  String FhemIcon;
  float Temperature;
  byte Humidity;
  int Pressure;
  unsigned int FramesPerMinute;
  String Version;
  uint32_t RSSI;
};
#endif