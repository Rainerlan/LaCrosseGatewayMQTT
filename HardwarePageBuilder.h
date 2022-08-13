#ifndef _HARDWAREPAGEBUILDER_h
#define _HARDWAREPAGEBUILDER_h

#include "Arduino.h"
#include "RFMxx.h"
#include "OwnSensors.h"
#include "BMP180.h"
#include "BMP280.h"
#include "BME280.h"
#include "BME680.h"
#include "HTML.h"
#include "SC16IS750.h"
#include "DigitalPorts.h"
#include "Display.h"
#include "DataPort.h"
#include "SerialBridge.h"
#include "SoftSerialBridge.h"
#include "AnalogPort.h"
#include "Nextion.h"

class HardwarePageBuilder {
public:
  String Build(RFMxx *rfm1, RFMxx *rfm2, RFMxx *rfm3, RFMxx *rfm4, RFMxx *rfm5, OwnSensors *ownSensors, SC16IS750 *sc16is750, SC16IS750 *sc16is750_2, DigitalPorts *digitalPorts, Display *display, DataPort *dataPort1, DataPort *dataPort2, DataPort *dataPort3, TcpServer *serialBridge, TcpServer *serialBridge2, TcpServer *softSerialBridge, AnalogPort *analogPort, Nextion *nextion);

private:
  String BuildRfmLine(RFMxx *rfm1, String name);
  String BuildDataPortLine(DataPort *dataPort, String name);
  String BuildSerialBridgeLine(TcpServer* serialBridge, String name);
};
#endif

