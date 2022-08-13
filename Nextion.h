#ifndef _NEXTION_H
#define _NEXTION_H

#include "Arduino.h"
#include "WSBase.h"
#include "ESP8266WebServer.h"
#include "ESP8266SoftSerial.h"
#include "FS.h"
#include "StateManager.h"

typedef void ProgressCallbackType(byte action, unsigned long offset, unsigned long maxValue, String message);

class Nextion {
public:
  Nextion();
  bool Begin(ESP8266WebServer *webServer, ESP8266SoftSerial *softSerial, unsigned long baud, bool addUnits, bool pressureDecimals);
  void Handle(WSBase::Frame frame, StateManager *stateManager, int32_t rssi, bool fhemIsConnected, bool bridge1Connected, bool bridge2Connected);
  void UploadeFile(File *file);
  void SendCommand(String command);
  void Log(String text, bool newLine = true);
  bool IsConnected();
  void SetProgressCallback(ProgressCallbackType* callback);
  
  void ShowProgress(unsigned int maxValue, String message);
  void MoveProgress(unsigned int offset=0);
  void HideProgress();
  void Print(String data);


private:
  unsigned long m_lastUpdate;
  ESP8266WebServer *m_server;
  ESP8266SoftSerial *m_softSerial;
  WiFiServer m_wifiServer;
  WiFiClient m_client;
  String m_log;
  File m_uploadFile;
  bool m_isConnected;
  bool m_addUnits;
  bool m_pressureDecimals;
  unsigned long m_baud;
  unsigned int m_currentProgress;
  unsigned int m_maxProgress;
  ProgressCallbackType *m_progressCallback;

  String Receive(unsigned int timeout, String waitFor = "");
  String Receive(unsigned int timeout, byte waitFor);
  void Connect();
};


#endif

