#include "Arduino.h"
extern "C" {
#include "gpio.h"
}
#include "ESP8266SoftSerial.h"

#define WAIT { while (ESP.getCycleCount()-start < wait); wait += m_bitTime; }

ESP8266SoftSerial *ObjList[16];
void ICACHE_RAM_ATTR sws_isr_0() { ObjList[0]->OnRXD(); };
void ICACHE_RAM_ATTR sws_isr_1() { ObjList[1]->OnRXD(); };
void ICACHE_RAM_ATTR sws_isr_2() { ObjList[2]->OnRXD(); };
void ICACHE_RAM_ATTR sws_isr_3() { ObjList[3]->OnRXD(); };
void ICACHE_RAM_ATTR sws_isr_4() { ObjList[4]->OnRXD(); };
void ICACHE_RAM_ATTR sws_isr_5() { ObjList[5]->OnRXD(); };
void ICACHE_RAM_ATTR sws_isr_12() { ObjList[12]->OnRXD(); };
void ICACHE_RAM_ATTR sws_isr_13() { ObjList[13]->OnRXD(); };
void ICACHE_RAM_ATTR sws_isr_14() { ObjList[14]->OnRXD(); };
void ICACHE_RAM_ATTR sws_isr_15() { ObjList[15]->OnRXD(); };

static void(*ISRList[16])() = {
  sws_isr_0,
  sws_isr_1,
  sws_isr_2,
  sws_isr_3,
  sws_isr_4,
  sws_isr_5,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  sws_isr_12,
  sws_isr_13,
  sws_isr_14,
  sws_isr_15
};

ESP8266SoftSerial::ESP8266SoftSerial(byte rxd, byte txd, unsigned int bufferSize) {
  m_rxd = rxd;
  m_txd = txd;
  m_bufferSize = bufferSize;
}

void ESP8266SoftSerial::Begin(unsigned long baud) {
  m_buffer = (byte*)malloc(m_bufferSize);
  m_inPos = m_outPos = 0;
  pinMode(m_rxd, INPUT);
  ObjList[m_rxd] = this;
  attachInterrupt(m_rxd, ISRList[m_rxd], FALLING);
  SetBaudrate(baud);
  pinMode(m_txd, OUTPUT);
  digitalWrite(m_txd, HIGH);
}

void ESP8266SoftSerial::SetBaudrate(unsigned long baud) {
  m_bitTime = ESP.getCpuFreqMHz() * 1000000 / baud;
}

ESP8266SoftSerial::~ESP8266SoftSerial() {
  detachInterrupt(m_rxd);
  ObjList[m_rxd] = NULL;
  if (m_buffer) {
    free(m_buffer);
  }
}

int ESP8266SoftSerial::Read() {
  int result = -1;
  if (m_inPos != m_outPos) {
    result = m_buffer[m_outPos];
    m_outPos = (m_outPos + 1) % m_bufferSize;
  }

  return result;
}

int ESP8266SoftSerial::Available() {
  int avail = m_inPos - m_outPos;
  if (avail < 0) avail += m_bufferSize;
  return avail;
}

void ESP8266SoftSerial::Write(byte data) {
  cli();
  unsigned long wait = m_bitTime;
  digitalWrite(m_txd, HIGH);
  unsigned long start = ESP.getCycleCount();
  digitalWrite(m_txd, LOW);
  WAIT;
  for (int i = 0; i < 8; i++) {
    digitalWrite(m_txd, (data & 1) ? HIGH : LOW);
    WAIT;
    data >>= 1;
  }
  digitalWrite(m_txd, HIGH);
  WAIT;
  sei();
}

void ESP8266SoftSerial::Print(String data) {
  for (unsigned int i = 0; i < data.length(); i++) {
    Write(data[i]);
  }
}

void ESP8266SoftSerial::Println(String data) {
  Print(data);
  Write(13);
}

void ICACHE_RAM_ATTR ESP8266SoftSerial::OnRXD() {
  unsigned long wait = m_bitTime + m_bitTime / 3 - 500;
  unsigned long start = ESP.getCycleCount();
  byte rec = 0;
  for (int i = 0; i < 8; i++) {
    WAIT;
    rec >>= 1;
    if (digitalRead(m_rxd)) {
      rec |= 0x80;
    }
  }
  WAIT;
  int next = (m_inPos + 1) % m_bufferSize;
  if (next != m_inPos) {
    m_buffer[m_inPos] = rec;
    m_inPos = next;
  }
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxd);
}
