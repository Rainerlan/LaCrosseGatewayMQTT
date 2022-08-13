#include "EMT7110.h"

// Data rate: 9.579 kbit/s

// Message-Format
// ---------------------------------------------------------------------------
// Byte  0 + 1        Identification normally: 25 6A  during Pairing: 25 2A
// Byte  2 + 3 	      ID
// Byte  4 Bit 6 	    Device connected
// Byte  4 Bit 7      Pairing flag
// Byte  4 Bit 0..5   Power 0.5 W steps
// Byte  5            Power 0.5 W steps
// Byte  6 + 7 	      Current (mA) (MSB first),
// Byte  8 	          Voltage (0.5 V steps - 128 V)
// Byte  9 Bit 6      ???
// Byte  9 Bit 7      ???
// Byte  9 Bit 0..5   Accumulated Power (0.01 kWh steps)
// Byte 10            Accumulated Power (0.01 kWh steps) 
// Byte 11 	          CRC sum on all 12 bytes == 0

void EMT7110::DecodeFrame(byte *data, struct Frame *frame) {
  frame->IsValid = true;
  frame->Header1 = data[0];
  frame->Header2 = data[1];
  frame->PairingFlag = (data[4] & 0b10000000) > 0;
  frame->ID = (data[2] << 8) | data[3];
  frame->Byte9_6 = (data[9] & 0b01000000) > 0;
  frame->Byte9_7 = (data[9] & 0b10000000) > 0;

  if (frame->PairingFlag) {
    frame->Voltage = 0.0;
    frame->Current = 0.0;
    frame->Power = 0.0;
    frame->AccumulatedPower = 0.0;
    frame->ConsumersConnected = false;
    frame->CRC = 0;
    frame->IsValid = false;
  }
  else {
    frame->Voltage = 128.0 + data[8] * 0.5;
    frame->Current = (data[6] << 8) | data[7];
    frame->Power = ((data[4] & 0x3F) << 8 | data[5]) / 2;
    frame->AccumulatedPower = ((data[9] & 0x3F) << 8 | data[10]) / 100.0;
    frame->ConsumersConnected = (data[4] & 0b01000000) > 0;
    frame->CRC = data[11];
    frame->IsValid = CrcIsValid(data);
  }

  // [25 6A 54 AE 2A AA AA AA AA AA AA 49]
  // There is a user who has an EMT7110 that sometimes (every 30 seconds) sends these strange packets.
  // CRS is OK, means the EMT really sends this.
  // Until I get an idea what the EMT wants to tell us, I filter them.
  // At least the current AA AA would be 43,69 Ampere what simply can't be true.
  // Until now I am really sure that it is no "low batt" 
  if (data[6] == 0xAA && data[7] == 0xAA) {
    frame->IsValid = false;
  }


}


String EMT7110::AnalyzeFrame(byte *data) {
  String result;
  struct Frame frame;
  DecodeFrame(data, &frame);

  // Show the raw data bytes
  result += "EMT7110 [";
  for (int i = 0; i < FRAME_LENGTH; i++) {
    result += String(data[i], HEX);
    result += " ";
  }
  result += "]";

  // Check CRC
  if (!frame.IsValid) {
    result += " CRC:WRONG";
  }
  else {
    result += " CRC:OK";
  }
  // Start
  result += " S:";
  result += String(frame.Header1, HEX);
  result += " ";
  result += String(frame.Header2, HEX);

  // ID
  result += " ID:";
  result += String(frame.ID, HEX);

  // Voltag
  result += " V:";
  result += frame.Voltage;

  // Current
  result += " mA:";
  result += frame.Current;

  // Power
  result += " W:";
  result += frame.Power;

  // AccumulatedPower
  result += " kWh:";
  result += frame.AccumulatedPower;

  // Connected
  result += " Con.:";
  result += frame.ConsumersConnected;

  // Pairing
  result += " Pair:";
  result += frame.PairingFlag;

  // CRC
  result += " CRC:";
  result += frame.CRC;
 
  return result;
}


bool EMT7110::CrcIsValid(byte data[]) {
  byte crc = 0;
  for (int i=0; i < FRAME_LENGTH; i++) {
    crc += data[i];
  }

  return crc == 0;
}


String EMT7110::BuildFhemDataString(struct Frame *frame) {
  // Format
  // 
  // OK  EMT7110  84 81  8  237 0  13  0  2   1  6  1  -> ID 5451   228,5V   13mA   2W   2,62kWh
  // OK  EMT7110  84 162 8  207 0  76  0  7   0  0  1
  // OK  EMT7110  ID ID  VV VV  AA AA  WW WW  KW KW Flags
  //     |        |  |   |  |   |  |   |  |   |  |
  //     |        |  |   |  |   |  |   |  |   |   `--- AccumulatedPower * 100 LSB
  //     |        |  |   |  |   |  |   |  |    `------ AccumulatedPower * 100 MSB
  //     |        |  |   |  |   |  |   |   `--- Power (W) LSB
  //     |        |  |   |  |   |  |    `------ Power (W) MSB
  //     |        |  |   |  |   |   `--- Current (mA) LSB
  //     |        |  |   |  |    `------ Current (mA) MSB
  //     |        |  |   |  `--- Voltage (V) * 10 LSB
  //     |        |  |    `----- Voltage (V) * 10 MSB
  //     |        |    `--- ID
  //     |         `------- ID
  //      `--- fix "EMT7110"

  // Header and ID
  String result;
  result += "OK EMT7110 ";
  result += (byte)(frame->ID >> 8);
  result += " ";
  result += (byte)(frame->ID);
  result += " ";

  // Voltage (V * 10)
  int volt = (int)(frame->Voltage * 10);
  result += (byte)(volt >> 8);
  result += " ";
  result += (byte)(volt);
  result += " ";

  // Current (mA)
  result += (byte)((int)frame->Current >> 8);
  result += " ";
  result += (byte)(frame->Current);
  result += " ";

  // Power (W)
  result += (byte)((int)frame->Power >> 8);
  result += " ";
  result += (byte)(frame->Power);
  result += " ";

  // AccumulatedPower (kWh * 100)
  int acp = (int)(frame->AccumulatedPower * 100);
  result += (byte)(acp >> 8);
  result += " ";
  result += (byte)(acp);
  result += " ";

  // Flags
  byte flags = 0;
  flags += frame->ConsumersConnected * 1;
  flags += frame->PairingFlag * 2;
  result += flags;
  
  return result;
}


String EMT7110::GetFhemDataString(byte *data) {
  String fhemString = "";

  if (data[0] == 0x25 && (data[1] == 0x6A || data[1] == 0x2A || data[1] == 0x40)) {
    struct Frame frame;
    DecodeFrame(data, &frame);
    if (frame.IsValid) {
      fhemString = BuildFhemDataString(&frame);
    }

  }

  return fhemString;
}

bool EMT7110::TryHandleData(byte *data) {
  String fhemString = GetFhemDataString(data);

  if (fhemString.length() > 0) {
    Serial.println(fhemString);
  }

  return fhemString.length() > 0;
}

bool EMT7110::IsValidDataRate(unsigned long dataRate) {
  return dataRate == 9579ul;
}