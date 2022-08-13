#ifndef _DIGITALPORTS_h
#define _DIGITALPORTS_h
#include "Arduino.h"
#include "MCP23008.h"
#include "Settings.h"

class DigitalPorts {
public:
  typedef void TDataCallback(String data);
  typedef void TCommandCallback(String command);
  DigitalPorts();
  bool Begin(DigitalPorts::TDataCallback dataHandler, DigitalPorts::TCommandCallback commandHandler, Settings settings);
  bool IsConnected();
  void Handle();
  void Command(String command);

private:
  DigitalPorts::TDataCallback *m_dataCallback;
  DigitalPorts::TCommandCallback *m_commandCallback;
  MCP23008 m_mcp;
  bool m_isConnected;
  byte m_lastGPIOs;
  unsigned long m_lastPollTime;
  String m_identity;
  String m_confguration[8];

};
#endif

