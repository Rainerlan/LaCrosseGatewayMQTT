#include "OTAUpdate.h"
#include "Settings.h"
#include "Logger.h"

// define firmware HTTPSRV firmware FHEM/firmware firmware
// OTA-Server: 192.168.11.11
// OTA-Port:   8083
// OTA-url:    /fhem/firmware/LaCrosseGateway.bin

void OTAUpdate::SetDebugMode(boolean mode) {
  m_debug = mode;
}

String OTAUpdate::Start(Logger *logger) {
  String result = "";

  WiFiClient client;
  
  Settings s;
  s.Read(logger);
  String otaServer = s.Get("otaServer", "");
  uint otaPort = s.GetInt("otaPort", 0);
  String otaURL = s.Get("otaURL", "");

  t_httpUpdate_return updateResult = ESPhttpUpdate.update(client, otaServer, otaPort, otaURL);
  switch (updateResult) {
  case HTTP_UPDATE_FAILED:
    result += "FAILED";
    break;

  case HTTP_UPDATE_NO_UPDATES:
    result += "Was up to date";
    break;

  case HTTP_UPDATE_OK:
    result += "OK";
    break;
  }

  return result;
}
