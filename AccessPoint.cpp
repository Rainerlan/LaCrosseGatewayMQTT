#include "AccessPoint.h"

AccessPoint::AccessPoint(IPAddress ip, IPAddress gateway, IPAddress subnet, String ssidPrefix) {
  m_ip = ip;
  m_gateway = gateway;
  m_subnet = subnet;
  m_ssidPrefix = ssidPrefix;
}

void AccessPoint::SetLogItemCallback(LogItemCallbackType *callback) {
  m_logItemCallback = callback;
}

void AccessPoint::Begin(int autoClose) {
  if (m_logItemCallback != NULL) {
    m_logItemCallback("Starting ...");
  }
 
  WiFi.mode(WiFiMode::WIFI_AP);
  WiFi.softAPConfig(m_ip, m_ip, m_subnet);
  String ssid = m_ssidPrefix + "_" + String((unsigned int)ESP.getChipId());
  WiFi.softAP(ssid.c_str());
  
  if (m_logItemCallback != NULL) {
    m_logItemCallback("running, SSID=" + ssid);
  }

  m_autoClose = autoClose;
  m_startMillis = millis();
  m_running = true;
}

void AccessPoint::End() {
  if (m_logItemCallback != NULL) {
    m_logItemCallback("Closing  ...");
  }
  m_running = false;
  m_autoClose = 0;
  WiFi.mode(WiFiMode::WIFI_STA);
  
  if (m_logItemCallback != NULL) {
    m_logItemCallback("closed");
  }
}

void AccessPoint::Handle() {
  if(m_autoClose > 0 && m_running) {
    if(millis() - m_startMillis > (uint)m_autoClose * 1000) {
      End();
    }
  } 
}
