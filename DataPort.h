#ifndef _DATAPORT_h
#define _DATAPORT_h

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "TypedQueue.h"

typedef String CommandCallbackType(String);
typedef void LogItemCallbackType(String);
typedef void ConnectCallbackType(bool isConnected);

class DataPort {
 private:
   WiFiServer m_server;
   uint m_port;
   unsigned long m_lastMillis = 0;
   WiFiClient m_client;
   bool m_initialized = false;
   TypedQueue<String> m_queue;
   bool m_enabled;
   LogItemCallbackType *m_logItemCallback;
   ConnectCallbackType *m_ConnectCallback;
   bool m_isConnected;
   
 public:
   DataPort();
   void Begin(uint port);
   void Send(String data);
   bool Handle(CommandCallbackType* callback);
   void AddPayload(String payload);
   void SetLogItemCallback(LogItemCallbackType* callback);
   void SetConnectCallback(ConnectCallbackType* callback);
   uint GetPort();
   bool IsConnected();
};

#endif

