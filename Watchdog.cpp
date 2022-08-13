#include "Watchdog.h"


Watchdog::Watchdog() {
  m_enabled = false;
}

void Watchdog::Begin(DispatchCallbackType* callback) {
  m_dispatchCallback = callback;
  m_lastTrigger = 0;
  m_timeout = 0;
  
}

void Watchdog::Command(String command) {
  String upperCommand = command;
  upperCommand.toUpperCase();

  String parameter = "";
  int pos = command.indexOf('=');
  if (pos != -1) {
    parameter = command.substring(pos + 1);
    upperCommand = upperCommand.substring(0, pos);
  }

  if (upperCommand == "PING") {
    m_lastTrigger = millis();
    delay(100);
    m_dispatchCallback("LGW ALIVE");

    if (parameter.length() > 0) {
      m_timeout = parameter.toInt();
    }
    else {
      m_timeout = 0;
    }
  }

}

void Watchdog::Handle() {
  if(m_timeout > 0) {
    if (millis() < m_lastTrigger) { 
      m_lastTrigger = millis();
    }
    if (millis() > m_lastTrigger + m_timeout * 1000) {
      ESP.restart();
    }
  }
}

