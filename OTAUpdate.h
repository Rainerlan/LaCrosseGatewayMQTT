
#ifndef _OTAUPDATE_h
#define _OTAUPDATE_h

#include "Arduino.h"
#include "ESP8266httpUpdate.h"
#include "Logger.h"

class OTAUpdate {
protected:
  static bool m_debug;


public:
  static String Start(Logger *logger);
  static void SetDebugMode(boolean mode);



};


#endif

