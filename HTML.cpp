#include "HTML.h"


String HTML::Unescape(String escaped) {
  String result = "";
  for (unsigned int i = 0; i < escaped.length(); i++) {
    if (escaped[i] != '%') {
      result += escaped[i];
    }
    else {
      String specialChar = escaped.substring(i +1, i +3);
      result += (char)strtol(specialChar.c_str(), NULL, 16);
      i += 2;
    }
  }
  return result;
}


IPAddress HTML::IPAddressFromString(String ipString) {
  byte o1p = ipString.indexOf(".", 0);
  byte o2p = ipString.indexOf(".", o1p + 1);
  byte o3p = ipString.indexOf(".", o2p + 1);
  byte o4p = ipString.indexOf(".", o3p + 1);

  return IPAddress(strtol(ipString.substring(0, o1p).c_str(), NULL, 10),
    strtol(ipString.substring(o1p + 1, o2p).c_str(), NULL, 10),
    strtol(ipString.substring(o2p + 1, o3p).c_str(), NULL, 10),
    strtol(ipString.substring(o3p + 1, o4p).c_str(), NULL, 10));
}

byte HTML::UTF8ToASCII(byte utf8Char) {
  byte result = 0;
  static byte lastChar;
  if (utf8Char < 128) {
    lastChar = 0;
    result = utf8Char;
  }
  else {
    switch (lastChar) {
    case 0xC2:
      result = utf8Char;
      break;
    case 0xC3:
      result = utf8Char | 0xC0;
      break;
    case 0x82:
      if (utf8Char == 0xAC) {
        result = 0x80;
      }
      else {
        result = 0;
      }
      break;
    }

    lastChar = utf8Char;
  }

  return result;
}
String HTML::UTF8ToASCII(String utf8) {
  String result = "";
  for (uint i = 0; i < utf8.length(); i++) {
    char c = UTF8ToASCII(utf8.charAt(i));
    if (c != 0) {
      result += c;
    }
  }
  return result;
}