#include "AlarmHandler.h"

AlarmHandler::AlarmHandler(byte pin, TPinCallback pinFunction) {
  m_pinCallback = pinFunction;
  m_pin = pin;
  m_active = 0;
}

void AlarmHandler::Begin() {
  if (m_pinCallback) {
    m_pinCallback(1, m_pin, OUTPUT);
  }
  else {
    pinMode(m_pin, OUTPUT);
  }
  Off();
}

void AlarmHandler::Begin(byte pin) {
  m_pin = pin;
  Begin();
}

void AlarmHandler::On() {
  if (m_pinCallback) {
    m_pinCallback(2, m_pin, HIGH);
  }
  else {
    digitalWrite(m_pin, HIGH);
  }
  m_lastToggle = millis();
}

void AlarmHandler::Off() {
  if (m_pinCallback) {
    m_pinCallback(2, m_pin, LOW);
  }
  else {
    digitalWrite(m_pin, LOW);
  }
  m_lastToggle = millis();
}

void AlarmHandler::Handle() {
  if (m_active) {
    if (millis() > m_startTime + m_duration * 1000) {
      m_active = false;
      Off();
    }
    else {
      if (millis() > m_lastToggle + 2000) {
        for (byte i = 0; i < m_sequence; i++) {
          On();
          delay(100);
          Off();
          delay(i+1 < m_sequence ? 100 : 0);
        }
      }
    }
  }
}

void AlarmHandler::Set(byte sequence, word duration) {
  m_sequence = sequence > 5 ? 5 : sequence;
  m_duration = duration;
  
  if (m_duration == 0) {
    m_active = false;
    Off();
  }
  else {
    m_startTime = millis();
    m_active = true;
  }

}


