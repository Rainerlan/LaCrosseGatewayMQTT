#include "Settings.h"
#include "EEPROM.h"
#include "Logger.h"

Settings::Settings() {
}

void Settings::Read(Logger *logger) {
  m_data.Clear();

  EEPROM.begin(EEPROM_SIZE);
  String rawData;
  int i;
  for (i = 0; i < EEPROM_SIZE; i++) {
    rawData += (char)EEPROM.read(i);
  }
  EEPROM.end();
  
  logger->print("Read bytes from EEPROM: ");
  logger->print(rawData);
  logger->println(i);
  
  if(rawData[0] == 1) {
    String key = "";
    String value = "";
    bool keyDone = false;
    bool valueDone = false;
    for (uint i=1; i < rawData.length(); i++) {
      if (!keyDone) {
        if (rawData[i] != 2) {
          key += (char)rawData[i];
        }
        else {
          keyDone = true;
          continue;
        }
      }
      
      if (keyDone && !valueDone) {
        if (rawData[i] != 1) {
          value += (char)rawData[i];
        }
        else {
          valueDone = true;
        }
      }
    
      if (keyDone && valueDone) {
        keyDone = false;
        valueDone = false;
        if(key.length() > 0 && value.length() > 0) {
          m_data.Put(key, value);
        }
        key = "";
        value = "";
      }
      
    }
  }

}

String Settings::Write() {
  String result;
  String rawData;
  
  rawData += (char)1;
  for(uint i=0; i < m_data.Size(); i++) {
    rawData += m_data.GetKeyAt(i);
    rawData += (char)2;
    rawData += m_data.GetValueAt(i);
    rawData += (char)1;
  }
 
  result += rawData.length();
  result += " Byte (max. ";
  result += EEPROM_SIZE;
  result += ") and ";
  result += m_data.Size();
  result += " values (max. ";
  result += m_data.GetCapacity();
  result += ")";

  while (rawData.length() < EEPROM_SIZE) {
    rawData += (char)0;
  }
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, rawData[i]);
  }
  EEPROM.end();
  
  return result;
}

void Settings::Dump() {
  m_data.Dump();
}

String Settings::Get(String key, String defaultValue) {
  return m_data.Get(key, defaultValue);
}

int Settings::GetInt(String key, int defaultValue) {
  return atoi(Get(key, String(defaultValue)).c_str());
}

unsigned long Settings::GetUnsignedLong(String key, unsigned long defaultValue) {
  return strtoul(Get(key, String(defaultValue)).c_str(), NULL, DEC);
}

bool Settings::GetBool(String key) {
  return Get(key, "false").equals("true");
}

bool Settings::GetBool(String key, bool value) {
  return Get(key, value ? "true" : "false").equals("true");
}

void Settings::Add(String key, String value) {
  if (m_data.ContainsKey(key)) {
    m_data.Remove(key);
  }
  if(value.length() > 0) {
    m_data.Put(key, value);
  }
}

bool Settings::Change(String key, String value) {
  bool result = false;
  if (m_data.ContainsKey(key)) {
    m_data.Remove(key);
    m_data.Put(key, value);
    result = true;
  }
  return result;
}

void Settings::Remove(String key) {
  return m_data.Remove(key);
}

byte Settings::GetByte(String key, byte defaultValue) {
  String strVal = Get(key, (String)defaultValue);
  strVal.toLowerCase();
  if (strVal.startsWith("0x")) {
    return (byte)strtol(strVal.substring(2).c_str(), NULL, HEX);
  }
  else {
    return (byte)strtol(strVal.c_str(), NULL, DEC);
  }
  
}

String Settings::ToString() {
  String result = "SETUP ";
  for (uint i = 0; i < m_data.Size(); i++) {
    result += m_data.GetKeyAt(i);
    result += " ";
    result += m_data.GetValueAt(i);
    result += "; ";
  }
  
  return result;
}

bool Settings::FromString(String settings) {
  bool result = false;
  
  settings.trim();
  if (!settings.endsWith(";")) {
    settings += ";";
  }
 
  byte step = 0;
  String key = "";
  String value = "";
  for (uint i = 0; i < settings.length(); i++) {
    char cc = settings[i];
    if (step == 0) {
      if (cc == ' ' && key.length() > 0) {
        step = 1;
      }
      else {
        key += cc;
      }
    }
    else if (step == 1) {
      if (cc == ';') {
        step = 0;
        key.trim();
        value.trim();
        if (key.length() > 0 && value.length() > 0) {
          Add(key, value);
          key = "";
          value = "";
        }
      }
      else {
        value += cc;
      }
    }

  }
  
  return result;
}
