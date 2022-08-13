#include "DHTxx.h"

DHTxx::DHTxx() {

}

bool DHTxx::TryInitialize(byte pin) {
  bool result = false;
  m_pin = pin;
 
  m_type = 22;
  if (Read()) {
    result = true;
  }
  else {
    m_type = 11;
    result = Read();
  }

  return result;
}

float DHTxx::GetTemperature() {
  return m_temperature;
}

int DHTxx::GetHumidity() {
  return m_humidity;
}

DHTxxValue DHTxx::GetLastMeasuredValue() {
  return m_lastValue;
}

String DHTxx::GetType() {
  return "DHT" + String(m_type);
}

bool DHTxx::TryMeasure() {
  bool result = false;
  
  if (Read()) {
    if(m_type == 22) {
      // DHT22
      m_temperature = (((word)(m_data[2] & 0x7F) << 8) + m_data[3]) * 0.1;
      m_humidity = (((word)m_data[0] << 8) + m_data[1]) * 0.1;

      if (m_data[2] & 0x80) {
        m_temperature = -m_temperature;
      }
    }
    else {  
      // DHT11
      m_humidity    = m_data[0];
      m_temperature = m_data[2];
    }
    m_temperature = ((int)(m_temperature * 10)) / 10.0;
    m_humidity = round(m_humidity);

    m_lastValue.Temperature = m_temperature;
    m_lastValue.Humidity = m_humidity;

    byte CheckSum;
    if(m_type == 22) {
      // DHT22
      CheckSum = m_data[0] + m_data[1] + m_data[2] + m_data[3];
    }
    else {
      // DHT11
      CheckSum = m_data[0] + m_data[2];
    }
    if (m_data[4] == CheckSum) {
      result = true;
    }
    
  }
  
  return result;
}

bool DHTxx::Read() {
  unsigned int to = F_CPU / 40000;
  byte mask = 128;
  byte idx = 0;

  for (byte i=0; i < 5; i++) {
    m_data[i] = 0;
  }

  // Ask him
  pinMode(m_pin, OUTPUT);
  digitalWrite(m_pin, LOW);
  delayMicroseconds(m_type == 22 ? 1000 : 18000);
  digitalWrite(m_pin, HIGH);
  pinMode(m_pin, INPUT);
  delayMicroseconds(40);

  unsigned int loopCnt = to;
  while(digitalRead(m_pin) == LOW) {
    if (--loopCnt == 0) {
      return false;
    }
  }

  loopCnt = to;
  while(digitalRead(m_pin) == HIGH) {
    if (--loopCnt == 0) {
      return false;
    }
  }

  for (byte i = 40; i != 0; i--) {
    loopCnt = to;
    while(digitalRead(m_pin) == LOW) {
      if (--loopCnt == 0) {
        return false;
      }
    }

    uint32_t t = micros();

    loopCnt = to;
    while(digitalRead(m_pin) == HIGH) {
      if (--loopCnt == 0) {
        return false;
      }
    }

    if ((micros() - t) > 40) { 
      m_data[idx] |= mask;
    }
    mask >>= 1;
    if (mask == 0) {
      mask = 128;
      idx++;
    }
  }

  return true;
}



