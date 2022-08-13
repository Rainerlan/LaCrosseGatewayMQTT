#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include <vector>
#include <WiFiClient.h>
#include <WiFiServer.h>

class TcpServer {
public:
  TcpServer(String typeName);
  
  typedef void ConnectCallbackType(bool isConnected);
  typedef void LogItemCallbackType(String, bool);

  void Begin(uint tcpPort);
  inline uint GetClientsConnected() const { return m_clients.size(); }
  inline uint GetPort() const { return m_tcpPort; }
  virtual void Handle();
  inline bool IsConnected() const { return m_clients.size() > 0; }
  inline bool IsEnabled() const { return m_enabled; }
  void SetConnectCallback(ConnectCallbackType* callback);

protected:
  virtual void dispatchData(byte data) = 0;
  virtual void doLog(String message) {}
  virtual void onClientConnected(WiFiClient* client) {}
  void sendToClients(String data);
  void writeToClients(const uint8_t *buf, size_t size);
  void writeToClients(uint8_t);
  
private:  
  std::vector<WiFiClient> m_clients;
  ConnectCallbackType* m_ConnectCallback;
  bool m_enabled;
  uint m_tcpPort;
  String m_typeName;
  WiFiServer m_wifiServer;
};

#endif

