#include "SubProcessor.h"
#include "LaCrosse.h"

SubProcessor::SubProcessor(SC16IS750 *expander, byte dtrPort, String hexFilename) : AddOnSerialBase(expander, dtrPort, hexFilename) {
}

void SubProcessor::Begin(ESP8266WebServer *server) {
  m_server = server;
  m_log = "";
  m_doLogging = false;
  m_receivedData = "";
  m_dispatchData = "";
  
  AddOnSerialBase::Begin();
}

String SubProcessor::GetSubDataValue(String data, String key, String defaultValue) {
  String result = defaultValue;
  key += "=";
  data += ",";
  int start = data.indexOf(key);
  if (start > -1) {
    int end = data.indexOf(",", start);
    if (end > start) {
      result = data.substring(start + key.length(), end);
    }
  }

  return result;
}


String SubProcessor::Handle(){
  String result = "";
  byte count = m_expander->Available();
  if (m_dispatchData.length() == 0 && count > 0) {
    for (byte i = 0; i < count; i++) {
      byte bt = m_expander->Read();
      if (bt != 13 && bt != 10) {
        m_receivedData += String((char)bt);
      }

      if (bt == 13) {
        m_dispatchData = m_receivedData;
        m_receivedData = "";
      }
    }
  }
  
   if (HasReceivedData()) {
    // Format:  KV <Type> <Address> <Key>=<Value>,<Key>=<Value>,<Key>=<Value>, ...
    // Example: KV DHT 01 Temperature=21.5,Humidity=62
    // -> send it as KeyValueProtocol to FHEM
    if (m_dispatchData.startsWith("KV ") && m_dispatchData.length() > 3) {
      result = "OK VALUES " + m_dispatchData.substring(3);
    }

    // Format:  LC <Address> T=<Temperature>,H=<Humidity>
    // Examples: LC 9F T=21.5,H=62
    //           LC 9F T=21.5
    // -> send it encoded like a LaCrosse sensor (like TX29DTH) to FHEM
    else if (m_dispatchData.startsWith("LC ") && m_dispatchData.length() > 3) {
      m_dispatchData = m_dispatchData.substring(3);

      LaCrosse::Frame frame;
      frame.CRC = 0;
      frame.Header = 9;
      frame.Humidity = 106;
      frame.IsValid = true;
      frame.NewBatteryFlag = false;
      frame.Temperature = 0;
      frame.WeakBatteryFlag = false;
      frame.ID = (byte)strtol(m_dispatchData.substring(0, 2).c_str(), 0, 16);
      m_dispatchData = m_dispatchData.substring(3);
      frame.Temperature = GetSubDataValue(m_dispatchData, "T", "0").toFloat();
      frame.Humidity = (byte)GetSubDataValue(m_dispatchData, "H", "106").toInt();
      String fhemData = LaCrosse::BuildFhemDataString(&frame);

      result = fhemData;
    }
  }
  
  return result;
}

bool SubProcessor::HasReceivedData() {
  return m_dispatchData.length() > 0;
}

String SubProcessor::GetReceivedData() {
  String result = m_dispatchData;
  m_dispatchData = "";
  return result;
}
