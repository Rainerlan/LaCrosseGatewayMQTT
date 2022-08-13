#include "SerialBridge.h"

SerialBridge::SerialBridge(SC16IS750 *expander, byte dtrPort, String hexFilename) : 
  AddOnSerialBase(expander, dtrPort, hexFilename),
  TcpServer("SerialBridge")
{
  m_doLogging = true;
}

void SerialBridge::Begin(uint tcpPort, ESP8266WebServer *server) {
  m_server = server;
  AddOnSerialBase::Begin();
  TcpServer::Begin(tcpPort);
}

void SerialBridge::Handle() {
  TcpServer::Handle();

  byte len = m_expander->Available();
  if (len > 0) {
    byte buffer[len];
    for (byte b = 0; b < len; b++) {
      buffer[b] = m_expander->Read();
    }
    writeToClients(buffer, len);
  }
}

void SerialBridge::onClientConnected(WiFiClient* client) {
  Reset();
}

void SerialBridge::dispatchData(byte data) {
  m_expander->Write(data);     
}

void SerialBridge::doLog(String message) {
  Log(message);    
}
