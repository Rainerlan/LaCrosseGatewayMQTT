#ifndef _SC16IS750_H_
#define _SC16IS750_H_

#include "Arduino.h"

class SC16IS750  { 
public:
  SC16IS750(byte mode, byte address);
  void PinMode(byte pin, byte mode);
  void DigitalWrite(byte pin, byte value);
  bool DigitalRead(byte pin);
  bool Begin(uint32_t baud, bool isClone);
  byte Read();
  void Write(byte value);
  byte Available();
  void Reset();
  bool IsConnected();
  void SetBaudrate(unsigned long baudrate);
  unsigned long GetBaudrate();
  void SetLine(byte dataBits, byte parity, uint8_t stopBits);
  bool IsClone();
  
private:
  bool m_isClone;
  byte m_address;
  byte m_mode;
  bool m_connected;
  unsigned long m_baudrate;
  byte ReadRegister(byte reg_addr);
  void WriteRegister(byte reg, byte value);

};

// General Registers
#define     SC16IS750_REG_RHR        (0x00)
#define     SC16IS750_REG_THR        (0X00)
#define     SC16IS750_REG_IER        (0X01)
#define     SC16IS750_REG_FCR        (0X02)
#define     SC16IS750_REG_IIR        (0X02)
#define     SC16IS750_REG_LCR        (0X03)
#define     SC16IS750_REG_MCR        (0X04)
#define     SC16IS750_REG_LSR        (0X05)
#define     SC16IS750_REG_MSR        (0X06)
#define     SC16IS750_REG_SPR        (0X07)
#define     SC16IS750_REG_TCR        (0X06)
#define     SC16IS750_REG_TLR        (0X07)
#define     SC16IS750_REG_TXLVL      (0X08)
#define     SC16IS750_REG_RXLVL      (0X09)
#define     SC16IS750_REG_IODIR      (0X0A)
#define     SC16IS750_REG_IOSTATE    (0X0B)
#define     SC16IS750_REG_IOINTENA   (0X0C)
#define     SC16IS750_REG_IOCONTROL  (0X0E)
#define     SC16IS750_REG_EFCR       (0X0F)

// Special Registers
#define     SC16IS750_REG_DLL        (0x00)
#define     SC16IS750_REG_DLH        (0X01)

// Enhanced Registers
#define     SC16IS750_REG_EFR        (0X02)
#define     SC16IS750_REG_XON1       (0X04)
#define     SC16IS750_REG_XON2       (0X05)
#define     SC16IS750_REG_XOFF1      (0X06)
#define     SC16IS750_REG_XOFF2      (0X07)

//
#define     SC16IS750_INT_CTS        (0X80)
#define     SC16IS750_INT_RTS        (0X40)
#define     SC16IS750_INT_XOFF       (0X20)
#define     SC16IS750_INT_SLEEP      (0X10)
#define     SC16IS750_INT_MODEM      (0X08)
#define     SC16IS750_INT_LINE       (0X04)
#define     SC16IS750_INT_THR        (0X02)
#define     SC16IS750_INT_RHR        (0X01)

//Application Related 
#define     SC16IS750_CRYSTCAL_FREQ  14745600UL
#define     SC16IS750_MODE_I2C       0
#define     SC16IS750_MODE_SPI       1

#endif





    
    
