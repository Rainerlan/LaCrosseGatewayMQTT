#ifndef __ALARM__H
#define __ALARM__H
#include "Arduino.h"

class AlarmHandler {
public:
  typedef byte TPinCallback(byte command, byte pin, byte value);
  AlarmHandler(byte pin=255, AlarmHandler::TPinCallback pinFunction = nullptr);
  void Begin();
  void Begin(byte pin);
  void Handle();
  void Set(byte sequence, word duration);

private:
  AlarmHandler::TPinCallback *m_pinCallback;
  byte m_pin;
  byte m_sequence;
  word m_duration;
  bool m_active;
  unsigned long m_lastToggle;
  unsigned long m_startTime;

  void On();
  void Off();

};
#endif

