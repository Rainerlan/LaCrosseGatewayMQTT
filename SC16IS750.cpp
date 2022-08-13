#include "SC16IS750.h"
#include "SPI.h"
#include "Wire.h"

SC16IS750::SC16IS750(byte mode, byte address) {
  m_mode = mode;
  m_isClone = false;
  m_connected = false;
  if (m_mode == SC16IS750_MODE_I2C) {
    m_address = (address >> 1);
  }
  else {
    m_address = address;
  }
}

bool SC16IS750::IsClone() {
  return m_isClone;
}

bool SC16IS750::Begin(uint32_t baud, bool isClone) {
  m_isClone = isClone;
  bool result = true;

  if (m_mode == SC16IS750_MODE_SPI) {
    ::pinMode(m_address, OUTPUT);
    ::digitalWrite(m_address, HIGH);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV4);
    SPI.setBitOrder(MSBFIRST);
    SPI.begin();
  };

  if (m_isClone) {
    delay(5000);
  }

  Reset();

  WriteRegister(SC16IS750_REG_SPR, 0x55);
  if (ReadRegister(SC16IS750_REG_SPR) != 0x55) {
    result = false;
  }

  if (result) {
    WriteRegister(SC16IS750_REG_SPR, 0xAA);
    if (ReadRegister(SC16IS750_REG_SPR) != 0xAA) {
      result = false;
    }
  }

  if (result) {
    m_connected = true;

    // Enable FIFO
    byte fcr = ReadRegister(SC16IS750_REG_FCR);
    fcr |= 0x01;
    WriteRegister(SC16IS750_REG_FCR, fcr);

    SetBaudrate(baud);
    SetLine(8, 0, 1);

    byte iocontrol = ReadRegister(SC16IS750_REG_IOCONTROL);
    iocontrol &= 0xFD;
    WriteRegister(SC16IS750_REG_IOCONTROL, iocontrol);

    for (byte b = 0; b < 8; b++) {
      PinMode(b, INPUT);
    }

  }

  return result;
}

bool SC16IS750::IsConnected() {
  return m_connected;
}

void SC16IS750::Reset() {
  byte iocontrol = ReadRegister(SC16IS750_REG_IOCONTROL);
  iocontrol |= 0x08;
  WriteRegister(SC16IS750_REG_IOCONTROL, iocontrol);
}

void SC16IS750::PinMode(byte pin, byte mode) {
  byte iodir = ReadRegister(SC16IS750_REG_IODIR);
  if (mode == OUTPUT) {
    iodir |= (0x01 << pin);
  }
  else {
    iodir &= (uint8_t)~(0x01 << pin);
  }

  WriteRegister(SC16IS750_REG_IODIR, iodir);
}

bool SC16IS750::DigitalRead(byte pin) {
  byte iostate = ReadRegister(SC16IS750_REG_IOSTATE);
  byte temp = iostate & (0x01 << pin);
  return (bool)temp;
}

void SC16IS750::DigitalWrite(byte pin, byte value) {
  byte temp_iostate = ReadRegister(SC16IS750_REG_IOSTATE);
  if (value == 1) {
    temp_iostate |= (0x01 << pin);
  }
  else {
    temp_iostate &= (uint8_t)~(0x01 << pin);
  }

  WriteRegister(SC16IS750_REG_IOSTATE, temp_iostate);

  if (m_isClone) {
    delayMicroseconds(10);
  }
}

byte SC16IS750::Read() {
  volatile uint8_t val;
  if (Available() == 0) {
    return -1;
  }
  else {
    val = ReadRegister(SC16IS750_REG_RHR);
    return val;
  }

}

void SC16IS750::Write(byte value) {
  byte lsr;
  do {
    lsr = ReadRegister(SC16IS750_REG_LSR);
  } while ((lsr & 0x20) == 0);

  WriteRegister(SC16IS750_REG_THR, value);
}

byte SC16IS750::Available() {
  return ReadRegister(SC16IS750_REG_RXLVL);
}

byte SC16IS750::ReadRegister(byte reg) {
  byte result;
  if (m_mode == SC16IS750_MODE_I2C) {
    Wire.beginTransmission(m_address);
    Wire.write((reg << 3));
    Wire.endTransmission(0);
    Wire.requestFrom(m_address, (byte)1);
    result = Wire.read();
  }
  else {
    ::digitalWrite(m_address, LOW);
    delayMicroseconds(10);
    SPI.transfer(0x80 | (reg << 3));
    result = SPI.transfer(0xff);
    delayMicroseconds(10);
    ::digitalWrite(m_address, HIGH);
  }

  return result;

}

void SC16IS750::WriteRegister(byte reg, byte value) {
  if (m_mode == SC16IS750_MODE_I2C) {
    Wire.beginTransmission(m_address);
    Wire.write((reg << 3));
    Wire.write(value);
    Wire.endTransmission(1);
  }
  else {
    ::digitalWrite(m_address, LOW);
    delayMicroseconds(10);
    SPI.transfer(reg << 3);
    SPI.transfer(value);
    delayMicroseconds(10);
    ::digitalWrite(m_address, HIGH);

  }

}

void SC16IS750::SetLine(byte dataBits, byte parity, uint8_t stopBits) {
  byte lcr = ReadRegister(SC16IS750_REG_LCR);
  lcr &= 0xC0;
  switch (dataBits) {
  case 5:
    break;
  case 6:
    lcr |= 0x01;
    break;
  case 7:
    lcr |= 0x02;
    break;
  case 8:
    lcr |= 0x03;
    break;
  default:
    lcr |= 0x03;
    break;
  }

  if (stopBits == 2) {
    lcr |= 0x04;
  }

  switch (parity) {
  case 0:
    // no parity
    break;
  case 1:
    // odd parity
    lcr |= 0x08;
    break;
  case 2:
    // even parity
    lcr |= 0x18;
    break;
  case 3:
    // force '1' parity
    lcr |= 0x03;
    break;
  case 4:
    // force '0' parity
    break;
  default:
    break;
  }

  WriteRegister(SC16IS750_REG_LCR, lcr);
}

void SC16IS750::SetBaudrate(unsigned long baudrate) {
  m_baudrate = baudrate;

  byte prescaler;
  if ((ReadRegister(SC16IS750_REG_MCR) & 0x80) == 0) {
    prescaler = 1;
  }
  else {
    prescaler = 4;
  }

  uint16_t divisor = (SC16IS750_CRYSTCAL_FREQ / prescaler) / (m_baudrate * 16);
  byte lcr = ReadRegister(SC16IS750_REG_LCR);
  lcr |= 0x80;
  WriteRegister(SC16IS750_REG_LCR, lcr);
  WriteRegister(SC16IS750_REG_DLL, (uint8_t)divisor);
  WriteRegister(SC16IS750_REG_DLH, (uint8_t)(divisor >> 8));
  lcr &= 0x7F;
  WriteRegister(SC16IS750_REG_LCR, lcr);
}

unsigned long SC16IS750::GetBaudrate() {
  return m_baudrate;
}


