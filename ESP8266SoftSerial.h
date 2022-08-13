#ifndef _ESP8266SOFTSERIAL_H
#define _ESP8266SOFTSERIAL_H

#include "Arduino.h"

class ESP8266SoftSerial {
public:
  ESP8266SoftSerial(byte rdx, byte txd, unsigned int bufferSize = 512);
  ~ESP8266SoftSerial();

  void Begin(unsigned long baud);
  void SetBaudrate(unsigned long baud);
  void Write(byte data);
  int Read();
  void OnRXD();
  int Available();
  void Print(String data);
  void Println(String data);

private:
  byte m_rxd, m_txd;
  unsigned long m_bitTime;
  unsigned int m_inPos, m_outPos;
  unsigned int m_bufferSize;
  byte *m_buffer;

};

#endif
