#include "EC3000.h"

// 868.300 kHz
// 20.000 kbps

/*
// Message-Format
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------
00  01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40
< ID >  <TSL >  zz  zz  <OSL >  zz  zz  zz  <   CONL     >  <PWR >  <MPWR>                                          <TSH>z  zz  zz  <CONH>  <OSH><RS>O  z<  CRC >
7E  43  D8  8B  00  00  D3  DD  00  00  00  00  30  0C  FB  00  54  00  6B  44  B9  04  AE  47  4A  E5  84  AE  07  00  50  00  00  00  00  00  50  18  02  46  37 ]  
7E  43  D8  90  00  00  D3  E2  00  00  00  00  30  0D  24  00  52  00  6B  44  B9  24  AE  47  4A  E5  84  AE  07  00  50  00  00  00  00  00  50  18  0F  58  37 ]

ID     device ID          bit  1 ... 16
TSL    total seconds      bit  1 ... 16
TSH    total seconds      bit 17 ... 28
OSL    on seconds         bit  1 ... 16
OSH    on seconds         bit 17 ... 28
CONL   consumption        bit  1 ... 32
CONH   consumption        bit 33 ... 48
PWR    power              bit  1 ... 16
MPWR   maximum power      bit  1 ... 16
RS     number of resets   bit  1 ... 12
CRC    CRC                bit  1 ... 16
O      on flag            0x08=on  0x00=off
z      always zero
*/


bool EC3000::IsValidDataRate(unsigned long dataRate) {
  return dataRate == 20000ul;
}

bool EC3000::TryHandleData(byte *payload) {
  String fhemString = GetFhemDataString(payload);

  if (fhemString.length() > 0) {
    Serial.println(fhemString);
  }

  return fhemString.length() > 0;
}

String EC3000::GetFhemDataString(byte *payload) {
  String fhemString = "";
  struct Frame frame;

  DecodeFrame(payload, &frame);
  if(frame.IsValid) {
    fhemString = BuildFhemDataString(&frame);
  }

  return fhemString;
}

String EC3000::AnalyzeFrame(byte *payload) {
  String result;
  struct Frame frame;
  DecodeFrame(payload, &frame);

  // Show the raw data bytes
  result += "EC3000 [";
  for (int i = 0; i < FRAME_LENGTH; i++) {
    result += String(payload[i], HEX);
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

  // ID
  result += " ID:";
  result += String(frame.ID, HEX);

  // TotalSeconds
  result += " TS:";
  result += frame.TotalSeconds;

  // OnSeconds
  result += " OS:";
  result += frame.OnSeconds;

  // Power
  result += " W:";
  result += frame.Power;

  // MaximumPower
  result += " max-W:";
  result += frame.MaximumPower;

  // Consumption
  result += " Cons:";
  result += frame.Consumption;

  // NumberOfResets
  result += " Res.:";
  result += frame.NumberOfResets;
 
  // Reception
  result += " Rec.:";
  result += frame.Reception;
  
  // IsOn
  result += " IsOn:";
  result += frame.IsOn;

  // CRC
  result += " CRC:";
  result += String(frame.CRC, HEX);
 
  return result;
}

String EC3000::BuildFhemDataString(struct Frame *frame) {
  // Format
  // 
  // OK  22 126  67   0   6 166 228   0   6 162  54   0   0   3 227   0  84   0 107   1   5
  // OK  22  ID  ID  TS  TS  TS  TS  OS  OS  OS  OS  CO  CO  CO  CO  PO  PO  PM  PM  NR  CR
  //     |    |   |   |           |   |           |   |           |   |   |   |   |   |   |
  //     |    |   |   |           |   |           |   |           |   |   |   |   |   |   `---| bits without ClockRecoverLock on = 5
  //     |    |   |   |           |   |           |   |           |   |   |   |   |   |
  //     |    |   |   |           |   |           |   |           |   |   |   |   |   `---| Nbr of resets = 1
  //     |    |   |   |           |   |           |   |           |   |   |   |   |
  //     |    |   |   |           |   |           |   |           |   |   |   |   `---| Max-Power = 10.7W
  //     |    |   |   |           |   |           |   |           |   |   |   `-------| 
  //     |    |   |   |           |   |           |   |           |   |   |   
  //     |    |   |   |           |   |           |   |           |   |   `---| Power = 8.4W
  //     |    |   |   |           |   |           |   |           |   `-------|
  //     |    |   |   |           |   |           |   |           `---|       
  //     |    |   |   |           |   |           |   |               | Consumption = 0.995KWh
  //     |    |   |   |           |   |           |   `-------------- |
  //     |    |   |   |           |   |           `---|
  //     |    |   |   |           |   |               | On seconds = 434742
  //     |    |   |   |           |    `--------------|
  //     |    |   |   |            `---| 
  //     |    |   |   |                | Total seconds = 435940
  //     |    |   |    `---------------| 
  //     |    |    `---| ID = 7E43
  //     |     `-------| 
  //     |     
  //      `--- fix "22"

  // Header and ID
  String result;
  result += "OK 22 ";
  result += (byte)(frame->ID >> 8);
  result += " ";
  result += (byte)(frame->ID);
  result += " ";
  result += (byte)(frame->TotalSeconds >> 24);
  result += " ";
  result += (byte)(frame->TotalSeconds >> 16);
  result += " ";
  result += (byte)(frame->TotalSeconds >> 8);
  result += " ";
  result += (byte)(frame->TotalSeconds);
  result += " ";
  result += (byte)(frame->OnSeconds >> 24);
  result += " ";
  result += (byte)(frame->OnSeconds >> 16);
  result += " ";
  result += (byte)(frame->OnSeconds >> 8);
  result += " ";
  result += (byte)(frame->OnSeconds);
  result += " ";
  uint32_t consumption10 = (uint32_t)(frame->Consumption * 1000);
  result += (byte)(consumption10 >> 24);
  result += " ";
  result += (byte)(consumption10 >> 16);
  result += " ";
  result += (byte)(consumption10 >> 8);
  result += " ";
  result += (byte)consumption10;
  result += " ";
  uint32_t power10 = (uint32_t)(frame->Power * 10);
  result += (byte)(power10 >> 8);
  result += " ";
  result += (byte)power10;
  result += " ";
  uint32_t maximumPower10 = (uint32_t)(frame->MaximumPower * 10);
  result += (byte)(maximumPower10 >> 8);
  result += " ";
  result += (byte)(maximumPower10);
  result += " ";
  result += (frame->NumberOfResets);
  result += " ";
  result += (frame->Reception);
  
  return result;
}


void EC3000::DescramblePayload(byte* payload) {
  byte ctr = EC3000::PAYLOAD_SIZE;
  uint8_t inpbyte, outbyte = 0;


  uint32_t scramshift = 0xF185D3AC;
  while (ctr--) {
    inpbyte = *payload;
    for (byte bit = 0; bit < 8; ++bit) {
      byte ibit = (inpbyte & 0x80) >> 7;
      byte obit = ibit ^ (Count1bits(scramshift & 0x31801) & 0x01);
      scramshift = scramshift << 1 | ibit;
      inpbyte <<= 1;
      outbyte = outbyte << 1 | obit;
    }
    *payload++ = outbyte ^ 0xFF;
  }
}

word EC3000::UpdateCRC(word crc, byte data) {
  data ^= crc & 0xFF;
  data ^= data << 4;
  return ((((word)data << 8) | (crc >> 8)) ^ (uint8_t)(data >> 4) ^ ((word)data << 3));
}

byte EC3000::Count1bits(uint32_t v) {
  byte c; // c accumulates the total bits set in v
  for (c = 0; v; c++) {
    v &= v - 1; // clear the least significant bit set
  }
  return c;
}

word EC3000::ShiftReverse(byte *payload) {
  byte rblen = 47;
  uint16_t i, ec3klen;
  word crc;

  ec3klen = rblen - 1;
  Del0BitsAndRevBits(payload + 1, ec3klen);
  crc = 0xFFFF;
  if (ec3klen >= FRAME_LENGTH) {
    for (i = 0; i < FRAME_LENGTH; ++i) {
      crc = UpdateCRC(crc, payload[1 + i]);
    }
  }
  ShiftLeft(payload, rblen, 4 + 8);
  
  return crc;
}

void EC3000::ShiftLeft(byte * payload, byte blen, byte shift) {
  uint8_t offs, bits, slen, i;
  uint16_t wbuf;

  if (shift == 0) {
    return;
  }
  offs = shift / 8;
  bits = 8 - shift % 8;
  slen = blen - offs - 1;
  wbuf = payload[offs];
  for (i = 0; i < slen; ++i) {
    wbuf = wbuf << 8 | payload[i + offs + 1];
    payload[i] = wbuf >> bits;
  }
  payload[slen] = wbuf << (uint8_t)(8 - bits);
}

void EC3000::Del0BitsAndRevBits(byte * payload, byte blen) {
  //  delete 0-bits inserted after 5 consecutive 1-bits and reverse bit-sequence
  //  EC3k packets are transmitted with HDLC bit-stuffing and LSBit first
  //  decode EC3k packets needs these 0-bits deleted and bits reversed to MSBit first
  uint8_t sval, dval, bit;
  uint8_t si, sbi, di, dbi, n1bits;

  di = dval = dbi = n1bits = 0;
  for (si = 0; si < blen; ++si) {
    sval = payload[si];      // get source byte
    for (sbi = 0; sbi < 8; ++sbi) {
      bit = sval & 0x80;      // get source-bit
      sval <<= 1;       // process source MSBit to LSBit
      if (n1bits >= 5 && bit == 0) {  // 5 1-bits and 1 0-bit
        n1bits = 0;     //  reset counter
        continue;     //  and skip 0-bit
      }
      if (bit) {
        n1bits++;   // count consecutive 1-bits
      }
      else {
        n1bits = 0;   // 0-bit: reset counter
      }
      dval = dval >> 1 | bit;   // add source-bit to destination
      dbi++;        //  reversing destin bit-sequence
      if (dbi == 8) {     // destination byte complete
        payload[di++] = dval;    //  store it
        dval = dbi = 0;     //  reset for next destin byte
      }
    }
  }
  if (dbi) {
    payload[di] = dval >> (uint8_t)(8 - dbi);
  }
}

void EC3000::DecodeFrame(byte *payload, struct Frame *frame) {
  frame->IsValid = false;

  DescramblePayload(payload);
  frame->CRC = ShiftReverse(payload);
  
  uint32_t mustBeZero = payload[4] + payload[5] + payload[8] + payload[9] + payload[10] + payload[31] + payload[32];
  
  if (frame->CRC == 0xF0B8 && mustBeZero == 0 && frame->ID != 0x00) {
    frame->IsValid = true;
  }

  frame->ID = (payload[0] << 8) | payload[1];
  frame->TotalSeconds = (uint32_t)payload[29] << 20 | (uint32_t)payload[30] << 12 | (uint32_t)payload[2] << 8 | (uint32_t)payload[3];
  frame->OnSeconds = (uint32_t)payload[35] << 20 | (uint32_t)payload[36] << 12 | (uint32_t)payload[6] << 8 | (uint32_t)payload[7];

  uint64_t cons = 0;
  cons |= payload[14];
  cons |= (uint16_t)payload[13] << 8;
  cons |= (uint16_t)payload[12] << 16;
  cons |= (uint32_t)(payload[11] &0x0F) << 24;
  cons |= (uint64_t)payload[34] << 28;
  cons |= (uint64_t)payload[33] << 36;
  frame->Consumption = cons / 3600000.0;

  frame->Power = ((word)payload[15] << 8 | (word)payload[16]) / 10.0;
  frame->MaximumPower = ((word)payload[17] << 8 | (word)payload[18]) / 10.0;
  frame->NumberOfResets = (payload[36] << 4) | (payload[37] >> 4);
  frame->IsOn = payload[37] & 0x08;
  frame->Reception = 0;
  
}



