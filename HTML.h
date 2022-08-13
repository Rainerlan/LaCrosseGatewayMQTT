#ifndef _HTML_h
#define _HTML_h

#include "Arduino.h"
#include "IPAddress.h"

class HTML {
public:
  static String Unescape(String escaped);
  static IPAddress IPAddressFromString(String ipString);
  static byte UTF8ToASCII(byte ascii);
  static String UTF8ToASCII(String utf8);

protected:


};


#endif

