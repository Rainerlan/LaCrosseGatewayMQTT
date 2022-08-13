#ifndef _SERIALBRIDGE_H
#define _SERIALBRIDGE_H

#include "AddOnSerialBase.h"
#include "TcpServer.h"

class SerialBridge : public AddOnSerialBase, public TcpServer {
public:
  SerialBridge(SC16IS750 *expander, byte dtrPort, String hexFilename);
  void Begin(uint tcpPort, ESP8266WebServer *server);
  void Handle();
  
private:
  void dispatchData(byte data);
  void doLog(String message);
  void onClientConnected(WiFiClient* client);
};

#endif

