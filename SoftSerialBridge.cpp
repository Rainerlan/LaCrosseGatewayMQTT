#include "SoftSerialBridge.h"

SoftSerialBridge::SoftSerialBridge() : TcpServer("Soft serial bridge"), m_softSerial(D4, D3) {
  m_progressCallback = nullptr;
}

void SoftSerialBridge::Begin(uint tcpPort, unsigned long baud, ESP8266WebServer *webServer) {
  TcpServer::Begin(tcpPort);
  m_webServer = webServer;
  m_softSerial.Begin(baud);
}

void SoftSerialBridge::Handle() {
  TcpServer::Handle();

  if (m_softSerial.Available()) {
    byte bt = m_softSerial.Read();
    writeToClients(bt);
  }
}

void SoftSerialBridge::dispatchData(byte data) {
  m_softSerial.Write(data);     
}

ESP8266SoftSerial * SoftSerialBridge::GetSoftSerial() {
  return &m_softSerial;
}

void SoftSerialBridge::SetProgressCallback(ProgressCallbackType * callback) {
  m_progressCallback = callback;
}
