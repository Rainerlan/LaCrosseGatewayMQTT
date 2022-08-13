#ifndef _ESP8266OTA_H
#define _ESP8266OTA_H

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "ESP8266WebServer.h"
#include "ESP8266mDNS.h"
#include "ArduinoOTA.h"

class ESP8266OTA {
public:
  ESP8266OTA();
  void Begin(ESP8266WebServer *server);
  void Handle();

private:
  ESP8266WebServer *m_server;
  String m_log;
  void Log(String text, bool newLine = true);
};


#endif

