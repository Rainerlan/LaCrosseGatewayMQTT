#include "DigitalPorts.h"

DigitalPorts::DigitalPorts() {
  m_isConnected = false;
}

bool DigitalPorts::IsConnected() {
  return m_isConnected;
}

bool DigitalPorts::Begin(DigitalPorts::TDataCallback dataHandler, DigitalPorts::TCommandCallback commandHandler, Settings settings) {
  m_dataCallback = dataHandler;
  m_commandCallback = commandHandler;
  m_identity = settings.Get("KVIdentity", String(ESP.getChipId()));
  m_lastPollTime = 0;

  if (m_mcp.TryInitialize(0x27)) {
    m_lastGPIOs = 0x00;
    m_isConnected = true;
    
    for(byte i = 0; i < 8; i++) {
      m_confguration[i] = settings.Get("IO" + (String)i, "Input");
      if(m_confguration[i] == "Output") {
        m_mcp.PinMode(i, OUTPUT);
        m_mcp.DigitalWrite(i, LOW);
      }
      else {
        m_mcp.PinMode(i, INPUT);
      }
    }
  }
  return m_isConnected;
}

void DigitalPorts::Handle() {
  if (m_isConnected) {
    if (millis() < m_lastPollTime) {
       m_lastPollTime = 0;
    }
    if (millis() > m_lastPollTime + 125) {
      
      byte currentGPIOs = m_mcp.GetGPIOs();
      if (m_lastGPIOs != currentGPIOs) {
        String result;
        result += "OK VALUES LGPB ";
        result += m_identity + " ";
        for (byte i = 0; i < 8; i++) {
          bool isSet = !((currentGPIOs >> i) & 0x1);
          if (m_confguration[i] != "Output") {
            result += ("PB" + String(i) + "=" + isSet + (i < 7 ? "," : ""));
          }
          if (isSet && m_confguration[i].startsWith("OLED ")) {
            m_commandCallback("\"" + m_confguration[i] + "\"");
          }

        }
      
        m_dataCallback(result);
      }
      m_lastGPIOs = currentGPIOs;
      m_lastPollTime = millis();
      
    }
  }
}

void DigitalPorts::Command(String command) {
  command += ", ";
  while (command.indexOf(',') != -1) {
    String part = command.substring(0, command.indexOf(','));
    part.trim();
    command = command.substring(command.indexOf(',') + 1);
    byte port = atoi(part.substring(2, 3).c_str());
    byte highLow = atoi(part.substring(4).c_str());

    if (port >= 0 && port <= 7) {
      if (m_confguration[port] == "Output") {
        m_mcp.DigitalWrite(port, highLow);
      }
    }

  }
}
