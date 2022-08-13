#ifndef _ADDONSERIALBASE_H
#define _ADDONSERIALBASE_H

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "WiFiClient.h"
#include "SC16IS750.h"
#include "ESP8266WebServer.h"
#include "FS.h"

typedef void ProgressCallbackType(byte action, unsigned long offset, unsigned long maxValue, String message);

class AddOnSerialBase {
public:
  AddOnSerialBase(SC16IS750 *expander, byte dtrPort, String hexFilename);
  typedef void LogItemCallbackType(String, bool);
  void Reset();
  void SetLogItemCallback(LogItemCallbackType callback);
  void Begin();
  void SetBaudrate(unsigned long baudrate);
  void SetProgressCallback(ProgressCallbackType* callback);

protected:
  SC16IS750 *m_expander;
  bool m_doLogging;
  String m_log;
  LogItemCallbackType *m_logItemCallback;
  byte m_dtrPort;
  ESP8266WebServer *m_server;
  File m_uploadFile;
  word m_currentPage;
  unsigned long m_originalBaudRate;
  bool m_isFlashing;
  ProgressCallbackType *m_progressCallback;
  String m_hexFilename;

  void Log(String text, bool newLine = true);
  void BeginFlash();
  void Receive();
  void SendBytes(byte count, ...);
  void SendBytes(byte count, byte *data);
  void FinishFlash();
  void FlashFile(File *file);
  void SendFile(File *file);
  void SendPage(byte packet[128], byte size);


};


#endif

