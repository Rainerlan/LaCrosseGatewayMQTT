#ifndef __MCP23008_H__
#define __MCP23008_H__

#include "Arduino.h"
#include "Wire.h"
#include "I2CBase.h"

class MCP23008 : public I2CBase {
public:
  bool TryInitialize(byte address);

  void PinMode(byte port, byte mode);
  void PullUp(byte port, byte direction);
  byte DigitalRead(byte port);
  void DigitalWrite(byte port, byte data);
  byte GetGPIOs();
  

private:
  void SetGPIOs(byte);
  

};

#define MCP23008_IODIR 0x00
#define MCP23008_IPOL 0x01
#define MCP23008_GPINTEN 0x02
#define MCP23008_DEFVAL 0x03
#define MCP23008_INTCON 0x04
#define MCP23008_IOCON 0x05
#define MCP23008_GPPU 0x06
#define MCP23008_INTF 0x07
#define MCP23008_INTCAP 0x08
#define MCP23008_GPIO 0x09
#define MCP23008_OLAT 0x0A

#endif
