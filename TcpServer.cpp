#include "TcpServer.h"
#include <algorithm>

TcpServer::TcpServer(String typeName) : m_ConnectCallback(nullptr), m_enabled(false), m_typeName(typeName), m_wifiServer(0) {
}

void TcpServer::Begin(uint tcpPort) {
  m_tcpPort = tcpPort;
  m_wifiServer = WiFiServer(m_tcpPort);
  m_wifiServer.begin();
  m_wifiServer.setNoDelay(true);
  m_enabled = true;
}

void TcpServer::SetConnectCallback(ConnectCallbackType* callback) {
  m_ConnectCallback = callback;
}

void TcpServer::Handle() {
  if (m_wifiServer.hasClient()) {
    WiFiClient client = m_wifiServer.available();
    client.setNoDelay(true);
    m_clients.push_back(client);

    onClientConnected(&client);
    delay(250);
   
    doLog(m_typeName + " #" + String(m_tcpPort) + ": Client " + client.remoteIP().toString() + ":" + String(client.remotePort()) + " connected"); 

    if (m_ConnectCallback && m_clients.size() == 1) { // first client connected
      m_ConnectCallback(true);
    }
  }

  m_clients.erase(std::remove_if(m_clients.begin(), m_clients.end(), [=](WiFiClient c) { 
    if (!c.connected()) {
      doLog(m_typeName + " #" + String(m_tcpPort) + ": Client disconnected");
      if (m_clients.size() == 1) { // last entry will be removed now
        m_ConnectCallback(false);
      }
      return true;
    } else {
      return false;
    }
  }), m_clients.end());
  
  for (unsigned idx = 0; idx < m_clients.size(); idx++) {
    if (m_clients[idx].available()) {
      while (m_clients[idx].available()) {
        dispatchData(m_clients[idx].read());
      }
      delay(10);
    }
  }
}

void TcpServer::writeToClients(const uint8_t *buf, size_t size) {
  for (unsigned idx = 0; idx < m_clients.size(); idx++) {
    m_clients[idx].write(buf, size);
  }
}

void TcpServer::writeToClients(uint8_t b) {
  for (unsigned idx = 0; idx < m_clients.size(); idx++) {
    m_clients[idx].write(b);
  }
}

void TcpServer::sendToClients(String data) {
  if (m_enabled) {
    for (unsigned idx = 0; idx < m_clients.size(); idx++) {
      m_clients[idx].println(data);
    }
  }
}

