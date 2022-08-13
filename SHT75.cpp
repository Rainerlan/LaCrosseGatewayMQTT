#include "Arduino.h"
#include "SHT75.h"

#define del 1

SHT75::SHT75(){
  m_isInitialized = false;
}

bool SHT75::TryInitialize(byte dataPin, byte clockPin) {
  m_dataPin = dataPin;
  m_clockPin = clockPin;

  // 12 bit humidity / 14 bit temperature / 3.3V
  m_compensation.CC1 = -2.0468;
  m_compensation.CC2 = 0.0367;
  m_compensation.CC3 = -1.5955E-6;
  m_compensation.CT1 = 0.01;
  m_compensation.CT1 = 0.00008;
  m_compensation.CD1 = -39.7;
  m_compensation.CD2 = 0.01;

  if (Command(0)) {
    m_isInitialized = true;
  }

  return m_isInitialized;
}

bool SHT75::IsInitialized() {
  return m_isInitialized;
}

float SHT75::GetTemperature() {
  float result = -273.0;

  if (Command(0b00000011)) {
    if (WaitForAck()) {
      word raw = ReadWord();
      SuppressCRC();
      result = (raw * m_compensation.CD2) + m_compensation.CD1;
    }
  }

  m_lastValue.Temperature = result;
  return result;
}

float SHT75::GetHumidity() {
  float result = -1;

  if (Command(0b00000101)) {
    if (WaitForAck()) {
      word raw = ReadWord();
      SuppressCRC();

      float compensatedHumidity = m_compensation.CC1 + m_compensation.CC2 * raw + m_compensation.CC3 * raw * raw;
      result = (GetTemperature() - 25.0) * (m_compensation.CT1 + m_compensation.CT2 * raw) + compensatedHumidity;
    }
  }

  m_lastValue.Humidity = result;
  return result;
}

SHT75Value SHT75::GetLastMeasuredValue() {
  return m_lastValue;
}


bool SHT75::Command(int command) {
  bool result = true;

  pinMode(m_dataPin, OUTPUT);
  pinMode(m_clockPin, OUTPUT);
  digitalWrite(m_dataPin, HIGH);
  delayMicroseconds(del);
  digitalWrite(m_clockPin, HIGH);
  delayMicroseconds(del);
  digitalWrite(m_dataPin, LOW);
  delayMicroseconds(del);
  digitalWrite(m_clockPin, LOW);
  delayMicroseconds(del);
  digitalWrite(m_clockPin, HIGH);
  delayMicroseconds(del);
  digitalWrite(m_dataPin, HIGH);
  delayMicroseconds(del);
  digitalWrite(m_clockPin, LOW);
  delayMicroseconds(del);

  ShiftOut(m_dataPin, m_clockPin, MSBFIRST, command);

  digitalWrite(m_clockPin, HIGH);
  pinMode(m_dataPin, INPUT);
  int ack = digitalRead(m_dataPin);
  if (ack != LOW) {
    result = false;
  }
  digitalWrite(m_clockPin, LOW);
  delayMicroseconds(10);
  ack = digitalRead(m_dataPin);
  if (ack != HIGH) {
    result = false;
  }

  return result;
}

bool SHT75::WaitForAck() {
  bool result = false;

  pinMode(m_dataPin, INPUT);

  for (uint i = 0; i < 1000; ++i) {
    delay(1);
    if (digitalRead(m_dataPin) == LOW) {
      result = true;
      break;
    }
  }

  return result;
}

word SHT75::ReadWord() {
  word result;
  
  pinMode(m_dataPin, INPUT);
  pinMode(m_clockPin, OUTPUT);
  result = ShiftIn(m_dataPin, m_clockPin, MSBFIRST);
  result *= 256;

  delayMicroseconds(del);
  pinMode(m_dataPin, OUTPUT);
  digitalWrite(m_dataPin, HIGH);
  delayMicroseconds(del);
  digitalWrite(m_dataPin, LOW);
  delayMicroseconds(del);
  digitalWrite(m_clockPin, HIGH);
  delayMicroseconds(del);
  digitalWrite(m_clockPin, LOW);
  delayMicroseconds(del);

  pinMode(m_dataPin, INPUT);
  delayMicroseconds(del);

  result |= ShiftIn(m_dataPin, m_clockPin, MSBFIRST);

  return result;
}

void SHT75::SuppressCRC() {
  pinMode(m_dataPin, OUTPUT);
  pinMode(m_clockPin, OUTPUT);
  digitalWrite(m_dataPin, HIGH);
  digitalWrite(m_clockPin, HIGH);
  delayMicroseconds(del);
  digitalWrite(m_clockPin, LOW);
}

uint8_t SHT75::ShiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
  uint8_t value = 0;
  uint8_t i;

  for (i = 0; i < 8; ++i) {
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(del);
    if (bitOrder == LSBFIRST) {
      value |= digitalRead(dataPin) << i;
    }
    else {
      value |= digitalRead(dataPin) << (7 - i);
    }
    digitalWrite(clockPin, LOW);
    delayMicroseconds(del);
  }
  return value;
}

void SHT75::ShiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
  uint8_t i;

  for (i = 0; i < 8; i++) {
    if (bitOrder == LSBFIRST) {
      digitalWrite(dataPin, !!(val & (1 << i)));
    }
    else {
      digitalWrite(dataPin, !!(val & (1 << (7 - i))));
    }
    delayMicroseconds(del);
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(del);
    digitalWrite(clockPin, LOW);
  }
}