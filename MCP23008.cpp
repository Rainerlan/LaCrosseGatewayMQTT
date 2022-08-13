#include "MCP23008.h"

bool MCP23008::TryInitialize(byte address) {
  bool result = false;
  m_address = address;

  Write8(MCP23008_IODIR, 0xAA);
  if(Read8(MCP23008_IODIR) == 0xAA) {
    Write8(MCP23008_IODIR, 0x55);
    if(Read8(MCP23008_IODIR) == 0x55) {
      result = true;
    }
  }
  
  if(result) {
    // Inputs with pull up
    Write8(MCP23008_IODIR, 0xFF);
    Write8(MCP23008_GPPU, 0xFF);
  }
    
  return result;
}

void MCP23008::PinMode(byte port, byte mode) {
  byte iodir;
  
  iodir = Read8(MCP23008_IODIR);
  if (mode == INPUT) {
    iodir |= 1 << port; 
  } else {
    iodir &= ~(1 << port);
  }

  Write8(MCP23008_IODIR, iodir);
}

byte MCP23008::GetGPIOs() {
  return Read8(MCP23008_GPIO);
}

void MCP23008::SetGPIOs(byte gpio) {
  Write8(MCP23008_GPIO, gpio);
}

void MCP23008::PullUp(byte port, byte direction) {
  byte gppu = Read8(MCP23008_GPPU);
  if (direction == HIGH) {
    gppu |= 1 << port; 
  }
  else {
    gppu &= ~(1 << port);
  }
  Write8(MCP23008_GPPU, gppu);
}

byte MCP23008::DigitalRead(byte port) {
  return (GetGPIOs() >> port) & 0x1;
}

void MCP23008::DigitalWrite(byte port, byte data) {
  if (port < 8) {
    byte gpio = GetGPIOs();
    if (data == HIGH) {
      gpio |= 1 << port;
    }
    else {
      gpio &= ~(1 << port);
    }
    SetGPIOs(gpio);
  }
}

