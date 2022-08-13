#include "RFMxx.h"

void RFMxx::EnableReceiver(bool enable) {
  if(enable) {
    if(IsRFM69) {
      WriteReg(REG_OPMODE, (ReadReg(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
    }
    else if(IsRFM95) {
      WriteReg(REG_RFM95_FIFO_ADDR_PTR, 0);
      WriteReg(REG_RFM95_OP_MODE, MODE_RFM95_LONG_RANGE_MODE | MODE_RFM95_RX_SINGLE);
    }
  }
  else {
    if(IsRFM69) {
      WriteReg(REG_OPMODE, (ReadReg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
    }
    else if(IsRFM95) {
      WriteReg(REG_RFM95_OP_MODE, MODE_RFM95_LONG_RANGE_MODE | MODE_RFM95_STDBY);
    }
  }
  ClearFifo();
}

void RFMxx::InitializeLaCrosse() {
  m_modulation = RFMxx::Modulation::FSK;
  if(IsRFM69) {
    /* 0x01 */ WriteReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY);
    /* 0x02 */ WriteReg(REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00);
    /* 0x05 */ WriteReg(REG_FDEVMSB, RF_FDEVMSB_90000);
    /* 0x06 */ WriteReg(REG_FDEVLSB, RF_FDEVLSB_90000);
    /* 0x11 */ WriteReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | RF_PALEVEL_OUTPUTPOWER_11111);
    /* 0x13 */ WriteReg(REG_OCP, RF_OCP_OFF);
    /* 0x19 */ WriteReg(REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2);
    /* 0x28 */ WriteReg(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);
    /* 0x29 */ WriteReg(REG_RSSITHRESH, 220);
    /* 0x2E */ WriteReg(REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_2 | RF_SYNC_TOL_0);
    /* 0x2F */ WriteReg(REG_SYNCVALUE1, 0x2D);
    /* 0x30 */ WriteReg(REG_SYNCVALUE2, 0xD4);
    /* 0x37 */ WriteReg(REG_PACKETCONFIG1, RF_PACKET1_CRCAUTOCLEAR_OFF);
    /* 0x38 */ WriteReg(REG_PAYLOADLENGTH, m_fifoSize);
    /* 0x3C */ WriteReg(REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE);
    /* 0x3D */ WriteReg(REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF);
    /* 0x6F */ WriteReg(REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0);
  }

  ClearFifo();
}

void RFMxx::SetFrequency(unsigned long kHz) {
  m_frequency = kHz;

  if(IsRFM69) {
    unsigned long f = (((kHz * 1000) << 2) / (32000000L >> 11)) << 6;
    WriteReg(0x07, f >> 16);
    WriteReg(0x08, f >> 8);
    WriteReg(0x09, f);
  }
  else if(IsRFM95) {
    uint64_t frf = ((uint64_t)m_frequency << 19) / 32000;
    WriteReg(REG_RFM95_FRF_MSB, (uint8_t)(frf >> 16));
    WriteReg(REG_RFM95_FRF_MID, (uint8_t)(frf >> 8));
    WriteReg(REG_RFM95_FRF_LSB, (uint8_t)(frf >> 0));
  }
}

void RFMxx::SetDataRate(unsigned long dataRate) {
  m_dataRate = dataRate;

  if(IsRFM69) {
    word r = ((32000000UL + (m_dataRate / 2)) / m_dataRate);
    WriteReg(0x03, r >> 8);
    WriteReg(0x04, r & 0xFF);
  }
}

void RFMxx::Receive() {
  if(IsRFM69) {
    if(ReadReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY) {
      for(int i = 0; i < m_fifoSize; i++) {
        byte bt = GetByteFromFifo();
        m_payload[i] = bt;
        m_payloadPointer++;
      }
      m_payloadReady = true;
    }
  }
  else if(IsRFM95) {
    int irqFlags = ReadReg(REG_RFM95_IRQ_FLAGS);

    // clear IRQ's
    WriteReg(REG_RFM95_IRQ_FLAGS, irqFlags);

    if((irqFlags & RFM95_IRQ_RX_DONE_MASK) && (irqFlags & RFM95_IRQ_PAYLOAD_CRC_ERROR_MASK) == 0) {
      uint8_t packetLength = ReadReg(REG_RFM95_RX_NB_BYTES);

      packetLength = packetLength > m_fifoSize ? m_fifoSize : packetLength;
      for(int i = 0; i < packetLength; i++) {
        m_payload[m_payloadPointer++] = ReadReg(REG_RFM95_FIFO);
      }
      m_payloadReady = true;

      WriteReg(REG_RFM95_FIFO_ADDR_PTR, ReadReg(REG_RFM95_FIFO_RX_CURRENT_ADDR));

      EnableReceiver(false);
    }
    else if(ReadReg(REG_RFM95_OP_MODE) != (MODE_RFM95_LONG_RANGE_MODE | MODE_RFM95_RX_SINGLE)) {
      EnableReceiver(true);
    }
  }
}

RFMxx::RFMxx(byte mosi, byte miso, byte sck, byte ss, byte irq, TPinCallback pinFunction) {
  m_mosi = mosi;
  m_miso = miso;
  m_sck = sck;
  m_ss = ss;
  m_irq = irq;

  m_fifoSize = 64;
  m_debug = false;
  m_dataRate = 17241;
  m_frequency = 868300;
  m_payloadPointer = 0;
  m_lastReceiveTime = 0;
  m_payloadReady = false;
  m_radioType = RFMxx::RadioType::None;
  m_modulation = RFMxx::Modulation::Unknown;

  pinMode(m_mosi, OUTPUT);
  pinMode(m_miso, INPUT);
  pinMode(m_sck, OUTPUT);

  m_pinCallback = pinFunction;
  if(!m_pinCallback) {
    pinMode(m_ss, OUTPUT);
    digitalWrite(m_ss, HIGH);
  }

}

bool RFMxx::Begin(bool isPrimary) {
  // No radio found until now
  m_radioType = RFMxx::RadioType::None;
  m_modulation = RFMxx::Modulation::Unknown;

  if(m_pinCallback) {
    m_pinCallback(1, m_ss, OUTPUT);
    m_pinCallback(2, m_ss, HIGH);
  }

  // Is there a RFM95 ?
  if(m_radioType == RFMxx::None) {
    uint8_t version = ReadReg(REG_RFM95_VERSION);
    if(version == 0x12) {
      m_radioType = RFMxx::RFM95W;
      m_fifoSize = 250;
    }
  }

  // Is there a RFM69 ?
  if(m_radioType == RFMxx::None) {
    WriteReg(REG_PAYLOADLENGTH, 0xA);
    if(ReadReg(REG_PAYLOADLENGTH) == 0xA) {
      WriteReg(REG_PAYLOADLENGTH, 0x40);
      if(ReadReg(REG_PAYLOADLENGTH) == 0x40) {
        m_radioType = RFMxx::RFM69CW;
      }
    }
  }

  if(m_radioType != RFMxx::None) {
    EnableReceiver(false);
  }

  return true;
}

bool RFMxx::IsConnected() {
  return m_radioType != RFMxx::None;
}

RFMxx::RadioType RFMxx::GetRadioType() {
  return m_radioType;
}

bool RFMxx::PayloadIsReady() {
  return m_payloadReady;
}

byte RFMxx::GetPayload(byte *data) {
  byte result = m_payloadPointer;
  for(int i = 0; i < m_payloadPointer; i++) {
    data[i] = m_payload[i];
  }
  m_payloadReady = false;
  m_payloadPointer = 0;
  return result;
}

String RFMxx::GetPayload() { 
  String result = "";
  for(int i = 0; i < m_payloadPointer; i++) {
    result += (char)m_payload[i];
  }
  m_payloadReady = false;
  m_payloadPointer = 0;
  return result;
}

String RFMxx::GetRadioName() {
  switch(GetRadioType()) {
    case RFMxx::RFM69CW:
      return String("RFM69");
      break;
    case RFMxx::RFM95W:
      return String("RFM95");
      break;
    default:
      return String("None");
  }
}

String RFMxx::GetInfoString() {
  String result = "";

  result += GetRadioName();
  result += " ";
  result += String(GetFrequency());
  result += " kHz ";
  if(m_modulation == RFMxx::Modulation::FSK) {
    result += "FSK ";
    result += String(GetDataRate()) + " kbps";
  }
  else if(m_modulation == RFMxx::Modulation::LoRa) {
    result += "LoRa SF=";
    result += String(GetSpreadingFactor());
    result += " BW=";
    result += String(GetBandwidthHz());
  }

  return result;
}

void RFMxx::SetPin(byte pin, bool value) {
  if(m_pinCallback) {
    m_pinCallback(2, pin, value);
  }
  else {
    digitalWrite(pin, value);
  }
}

void RFMxx::EnableTransmitter(bool enable) {
  if(enable) {
    if(IsRFM69) {
      WriteReg(REG_OPMODE, (ReadReg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
    }
  }
  else {
    if(IsRFM69) {
      WriteReg(REG_OPMODE, (ReadReg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
    }
  }
}

byte RFMxx::GetByteFromFifo() {
  return ReadReg(0x00);
}

void RFMxx::ClearFifo() {
  if(IsRFM69) {
    WriteReg(REG_IRQFLAGS2, 16);
  }
}

void RFMxx::PowerDown() {
  if(IsRFM69) {
    WriteReg(REG_OPMODE, (ReadReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
  }
  else if(IsRFM95) {
    WriteReg(REG_RFM95_OP_MODE, MODE_RFM95_LONG_RANGE_MODE | MODE_RFM95_SLEEP);
  }
}

void RFMxx::InitializePCA301() {
  m_modulation = RFMxx::Modulation::FSK;
  /*
  PCA      LGW
  0x94C5                 // RX: LNA Gain Max / Pin VDI / Bandwidth 67kHz  / VDI FAST / DRSSI -73dBm
  0x94a0        // RX: LNA Gain Max / Pin VDI / Bandwidth 134kHz / VDI FAST / DRSSI -103dBm

  0xCA83                 // FIFO: INT Level 8 / Sync 2 Byte / FillStart=Sync / Sens low  /  Enabled
  0xCA12        // FIFO: INT Level 1 / Sync 2 Byte / FillStart=Sync / Sens high / Enabled

  0xC477                 // AFC: Enabled / once after power up  / Limit +3..-4         / High Accuracy     / Enable  frequenct offset register / no strobe
  0xC481        // AFC: Enabled / only during VDI=high / Limit no restriction / NO High Accuracy  / Disable frequenct offset register / no strobe

  0xC2AF                 // Filter Digital / Recovery Auto    / Quality Tresh. 7 / Recovery Slow
  0xC26a        // Filter Digital / Recovery Manuell / Quality Tresh. 0 / Recovery Fast


  */

  if(IsRFM69) {
    WriteReg(REG_FDEVMSB, RF_FDEVMSB_45000);
    WriteReg(REG_FDEVLSB, RF_FDEVLSB_45000);
    WriteReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_OUTPUTPOWER_10110);
  }

  SetFrequency(868950);
  SetDataRate(6631);


}

void RFMxx::InitializeEC3000() {
  m_modulation = RFMxx::Modulation::FSK;
  if(IsRFM69) {
    /* 0x01 */ WriteReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY);
    /* 0x02 */ WriteReg(REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00);
    /* 0x05 */ WriteReg(REG_FDEVMSB, RF_FDEVMSB_20000);
    /* 0x06 */ WriteReg(REG_FDEVLSB, RF_FDEVLSB_20000);
    /* 0x11 */ WriteReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | RF_PALEVEL_OUTPUTPOWER_11111);
    /* 0x13 */ WriteReg(REG_OCP, RF_OCP_OFF);
    /* 0x18 */ WriteReg(REG_LNA, RF_LNA_GAINSELECT_MAX | RF_LNA_ZIN_200);
    /* 0x19 */ WriteReg(REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2);
    /* 0x28 */ WriteReg(REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN);
    /* 0x29 */ WriteReg(REG_RSSITHRESH, 220);
    /* 0x2E */ WriteReg(REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_5 | RF_SYNC_TOL_0);
    /* 0x2F */ WriteReg(REG_SYNCVALUE1, 0x13);
    /* 0x30 */ WriteReg(REG_SYNCVALUE2, 0xF1);
    /* 0x31 */ WriteReg(REG_SYNCVALUE3, 0x85);
    /* 0x32 */ WriteReg(REG_SYNCVALUE4, 0xD3);
    /* 0x33 */ WriteReg(REG_SYNCVALUE5, 0xAC);
    /* 0x37 */ WriteReg(REG_PACKETCONFIG1, RF_PACKET1_CRCAUTOCLEAR_OFF);
    /* 0x38 */ WriteReg(REG_PAYLOADLENGTH, m_fifoSize);
    /* 0x3C */ WriteReg(REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE);
    /* 0x3D */ WriteReg(REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF);
    /* 0x6F */ WriteReg(REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0);
  }

}

void RFMxx::InitializeLoRa() {
  m_modulation = RFMxx::Modulation::LoRa;
  if(IsRFM95) {
    WriteReg(REG_RFM95_OP_MODE, 0b10000000);                 // LoRa mode
    WriteReg(REG_RFM95_FIFO_TX_BASE_ADDR, 0);                // FIFO base address
    WriteReg(REG_RFM95_FIFO_RX_BASE_ADDR, 0);                // FIFO base address
    WriteReg(REG_RFM95_LNA, ReadReg(REG_RFM95_LNA) | 0x03);  // LNA boost
    WriteReg(REG_RFM95_MODEM_CONFIG_1, 0x72);                // 0111 0010 -> Explicit header mode, Error coding rate: 4/5, Signal bandwidth: 125 kHz 
    WriteReg(REG_RFM95_MODEM_CONFIG_2, 0xA4);                // 1010 0100 -> CRC on, 1024 chips/symbol (Spreading factor 10)
    WriteReg(REG_RFM95_MODEM_CONFIG_3, 0x04);                // 0000 0100 -> AGC auto
    
    SetTxPower(17);
    SetFrequency(868200);
    SetBandwidth(BW125);
    SetSpreadingFactor(SF7);
  }
}

const uint8_t RFMxx::GetFiFoSize() {
  return m_fifoSize;
}

void RFMxx::SetHFParameter(byte address, byte value) {
  WriteReg(address, value);
  if(m_debug) {
    Serial.print("WriteReg:");
    Serial.print(address);
    Serial.print("->");
    Serial.print(value);
  }
}


int RFMxx::GetRSSI(bool doTrigger) {
  int rssi = -1024;
  if(IsRFM69) {
    if(doTrigger) {
      WriteReg(REG_RSSICONFIG, RF_RSSI_START);
      unsigned long to = millis() + 100;
      while((ReadReg(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00 && millis() < to);
    }
    rssi = -ReadReg(REG_RSSIVALUE);
    rssi >>= 1;
  }
  else if(IsRFM95) { 
    rssi = ReadReg(REG_RFM95_PKT_RSSI_VALUE) - 137;
  }

  return rssi;
}

float RFMxx::GetSNR() {
  float snr = -1024.0;
  if(IsRFM69) {
  }
  else if(IsRFM95) {
    snr = (float)ReadReg(REG_RFM95_PKT_SNR_VALUE) / 4.0;
  }
  return snr;
}


byte RFMxx::ReadReg(byte addr) {
  SetPin(m_ss, LOW);
  SPI.transfer(addr & 0x7F);
  byte result = SPI.transfer(0);
  SetPin(m_ss, HIGH);
  return result;
}

void RFMxx::WriteReg(byte addr, byte value) {
  SetPin(m_ss, LOW);
  SPI.transfer(addr | 0x80);
  SPI.transfer(value);
  SetPin(m_ss, HIGH);
}

void RFMxx::SetDebugMode(boolean mode) {
  m_debug = mode;
}

unsigned long RFMxx::GetDataRate() {
  return m_dataRate;
}

unsigned long RFMxx::GetFrequency() {
  return m_frequency;
}


void RFMxx::SendArray(byte *data, byte length) {
  if(IsRFM69) {
    WriteReg(REG_PACKETCONFIG2, (ReadReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
    EnableReceiver(false);
    ClearFifo();
    noInterrupts();
    SetPin(m_ss, LOW);
    SPI.transfer(REG_FIFO | 0x80);
    for(byte i = 0; i < length; i++) {
      SPI.transfer(data[i]);
    }

    SetPin(m_ss, HIGH);
    interrupts();

    EnableTransmitter(true);

    // Wait until transmission is finished
    unsigned long txStart = millis();
    while(!(ReadReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PACKETSENT) && millis() - txStart < 30);

    EnableTransmitter(false);
  }
  else if(IsRFM95) {
    EnableReceiver(false);

    WriteReg(REG_RFM95_FIFO_ADDR_PTR, 0);
    WriteReg(REG_RFM95_PAYLOAD_LENGTH, 0);

    // write data
    for(byte i = 0; i < length; i++) {
      WriteReg(REG_RFM95_FIFO, data[i]);
    }
    WriteReg(REG_RFM95_PAYLOAD_LENGTH, length);

    // put in TX mode
    WriteReg(REG_RFM95_OP_MODE, MODE_RFM95_LONG_RANGE_MODE | MODE_RFM95_TX);

    // wait for TX done
    while((ReadReg(REG_RFM95_IRQ_FLAGS) & RFM95_IRQ_TX_DONE_MASK) == 0) {
      yield();
    }

    // clear IRQ's
    WriteReg(REG_RFM95_IRQ_FLAGS, RFM95_IRQ_TX_DONE_MASK);
  }
 
  if(m_debug) {
    Serial.print("Sending data: ");
    for(int p = 0; p < length; p++) {
      Serial.print(data[p], DEC);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void RFMxx::SendString(String data) { 
  SendArray((byte*)data.c_str(), data.length());
}

void RFMxx::SetTxPower(int level) {
  WriteReg(REG_RFM95_PA_CONFIG, RFM95_PA_BOOST | (level - 2));
}

void RFMxx::SetLdoFlag() {
  long symbolDuration = 1000 / (GetBandwidthHz() / (1L << (uint8_t)GetSpreadingFactor()));
  boolean ldoOn = symbolDuration > 16;
  uint8_t config3 = ReadReg(REG_RFM95_MODEM_CONFIG_3);
  bitWrite(config3, 3, ldoOn);
  WriteReg(REG_RFM95_MODEM_CONFIG_3, config3);
}

RFMxx::SpreadingFactor RFMxx::GetSpreadingFactor() {
  return static_cast<SpreadingFactor>(ReadReg(REG_RFM95_MODEM_CONFIG_2) >> 4);
}

void RFMxx::SetSpreadingFactor(SpreadingFactor sf) {
  if((uint8_t)sf == 6) {
    WriteReg(REG_RFM95_DETECTION_OPTIMIZE, 0xc5);
    WriteReg(REG_RFM95_DETECTION_THRESHOLD, 0x0c);
  }
  else {
    WriteReg(REG_RFM95_DETECTION_OPTIMIZE, 0xc3);
    WriteReg(REG_RFM95_DETECTION_THRESHOLD, 0x0a);
  }

  WriteReg(REG_RFM95_MODEM_CONFIG_2, (ReadReg(REG_RFM95_MODEM_CONFIG_2) & 0x0f) | (((uint8_t)sf << 4) & 0xf0));
  SetLdoFlag();
}

void RFMxx::SetBandwidth(RFMxx::Bandwidth bandwidth) {
  WriteReg(REG_RFM95_MODEM_CONFIG_1, (ReadReg(REG_RFM95_MODEM_CONFIG_1) & 0x0f) | ((uint8_t(bandwidth)) << 4));
  SetLdoFlag();
}

RFMxx::Bandwidth RFMxx::GetBandwidth() {
  return static_cast<Bandwidth>((ReadReg(REG_RFM95_MODEM_CONFIG_1) >> 4));
}

long RFMxx::GetBandwidthHz() {
  switch(GetBandwidth()) {
    case BW7_8: return 7.8E3;
    case BW10_4: return 10.4E3;
    case BW15_6: return 15.6E3;
    case BW20_8: return 20.8E3;
    case BW31_25: return 31.25E3;
    case BW41_7: return 41.7E3;
    case BW62_5: return 62.5E3;
    case BW125: return 125E3;
    case BW250: return 250E3;
    case BW500: return 500E3;
  }
}