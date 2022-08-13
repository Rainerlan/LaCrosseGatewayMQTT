#ifndef _WEBFRONTEND_h
#define _WEBFRONTEND_h

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "ESP8266WebServer.h"
#include "StateManager.h"
#include "Logger.h"
#include "Settings.h"

class WebFrontend {
 public:
   WebFrontend(int port);
   void Handle();
   void Begin(StateManager *stateManager, Logger *logger);
   ESP8266WebServer *WebServer();
   typedef void CommandCallbackType(String);
   typedef String HardwareCallbackType();
   void SetCommandCallback(CommandCallbackType callback);
   void SetHardwareCallback(HardwareCallbackType callback);
   void SetPassword(String password);

private:
  int m_port;
  ESP8266WebServer m_webserver;
  Logger *m_logger;
  StateManager *m_stateManager;
  CommandCallbackType *m_commandCallback;
  HardwareCallbackType *m_hardwareCallback;
  String m_password;
  String GetNavigation();
  String GetTop();
  String GetBottom();
  String GetIOCombo(byte nbr, String defaultValue);
  String GetMCPCombos(Settings *settings, byte part);
  String GetRedirectToRoot(String message = "");
  bool IsAuthentified();
  String GetDisplayName();
  String BuildHardwareRow(String text1, String text2, String text3);
};

#endif

