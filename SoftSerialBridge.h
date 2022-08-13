#ifndef _SOFTSERIALBRIDGE_H
#define _SOFTSERIALBRIDGE_H

#include "Arduino.h"
#include "ESP8266SoftSerial.h"
#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "ESP8266WebServer.h"
#include "TcpServer.h"

typedef void ProgressCallbackType(byte action, unsigned long offset, unsigned long maxValue, String message);

class SoftSerialBridge : public TcpServer {
public:
  SoftSerialBridge();
  void Begin(uint tcpPort, unsigned long baud, ESP8266WebServer *webServer);
  void Handle();
  void SetProgressCallback(ProgressCallbackType* callback);
  ESP8266SoftSerial *GetSoftSerial();

private:
  void dispatchData(byte data);

  ESP8266WebServer *m_webServer;
  ProgressCallbackType *m_progressCallback;
  ESP8266SoftSerial m_softSerial;
};

#endif

