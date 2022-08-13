#include "ESPTools.h"

ESPTools::ESPTools(byte ledPin) {
  m_ledPin = ledPin;
  m_ledEnabled = true;
  m_blinkDuration = 5;
  m_inverted = false;
}

ESPTools::ESPTools() {
  ESPTools(0);
  m_ledEnabled = false;
}

void ESPTools::SwitchLed(boolean on, bool force) {
  if (m_ledEnabled || force) {
    pinMode(m_ledPin, OUTPUT);
    digitalWrite(m_ledPin, m_inverted ? on : !on);
  }
}

void ESPTools::Blink(byte ct, bool force) {
  if (m_ledEnabled || force) {
    for (int i = 0; i < ct; i++) {
      SwitchLed(true, force);
      delay(m_blinkDuration);
      SwitchLed(false, force);
      if (ct > 1) {
        delay(m_blinkDuration);
      }
    }
  }

}

void ESPTools::SetBlinkDuration(uint duration) {
  m_blinkDuration = duration;
}

void ESPTools::SetLedPin(byte pin) {
  m_ledPin = pin;
  m_ledEnabled = true;
}

void ESPTools::SetInverted(bool inverted) {
  m_inverted = inverted;
}

void ESPTools::ScanI2C() {
  byte error, address;
 
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found: 0x" + String(address < 16 ? "0" : ""));
      Serial.println(address, HEX);
    }
    ////else if (error == 4) {
    ////  Serial.print("Unknow error at address 0x" + String(address < 16 ? "0" : ""));
    ////  Serial.println(address, HEX);
    ////}
  }
 
}

void ESPTools::EnableLED(bool enable) {
  m_ledEnabled = enable;

  if (!m_ledEnabled) {
    SwitchLed(false);
  }

}

