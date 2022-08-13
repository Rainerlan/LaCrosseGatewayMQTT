#ifndef ESP8266TOOLS_H
#define ESP8266TOOLS_H

#include "Arduino.h"
#include "IPAddress.h"
#include "Wire.h"

class ESPTools {
public:
  ESPTools(byte ledPin);
  ESPTools();
  void Blink(byte ct, bool force=false);
  void EnableLED(bool enable);
  void SwitchLed(boolean on, bool force=false);
  void SetBlinkDuration(uint duration);
  void SetInverted(bool inverted);
  void SetLedPin(byte pin);
  void ScanI2C();

private:
  byte m_ledPin;
  bool m_ledEnabled;
  uint m_blinkDuration;
  bool m_inverted;

};


#endif

