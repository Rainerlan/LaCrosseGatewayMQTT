#ifndef _SUBPROCESSOR_H
#define _SUBPROCESSOR_H

#include "AddOnSerialBase.h"

class SubProcessor : public AddOnSerialBase {
public:
  SubProcessor(SC16IS750 *expander, byte dtrPort, String hexFilename);
  void Begin(ESP8266WebServer *server);
  String Handle();
  bool HasReceivedData();
  String GetReceivedData();
 
private:
  String m_receivedData;
  String m_dispatchData;
  String GetSubDataValue(String data, String key, String defaultValue);

};


#endif

