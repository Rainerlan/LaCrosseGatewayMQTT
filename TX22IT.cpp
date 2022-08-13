#include "TX22IT.h"

/*
TX22-IT  8842 kbps  868.3 MHz
-----------------------------
Message Format:
SSSS.DDDD DDAE.LQQQ TTTT.VVVV VVVV.VVVV ... CCCC.CCCC 
Data - organized in nibbles - are structured as follows (example with blanks added for clarity):
a 5a 5 0 628 1 033 2 000 3 e00 4 000 bd

data always starts with "a"
from next 1.5 nibbles (here 5a) the 6 msb are identifier of transmitter,
bit 1 indicates acquisition/synchronizing phase
bit 0 will be 1 in case of error
next bit is the low battery flag
next three bits (here 5) is count of quartets transmitted
up to 5 quartets of data follow
each quartet starts with a type indicator (here 0,1,2,3,4)
0: temperature: 3 nibbles bcd coded tenth of °C plus 400 (here 628-400 = 22.8°C)
1: humidity: 3 nibbles bcd coded (here 33 %rH)
2: rain: 3 nibbles, counter of contact closures
3: wind: first nibble direction of wind vane (multiply by 22.5 to obtain degrees, here 0xe*22.5 = 315 degrees)
   next two nibbles wind speed in m/s
4: gust: speed in m/s
next two bytes (here bd) are crc.
During acquisition/synchronizing phase (abt. 5 hours) all 5 quartets are sent, see example above. Thereafter
data strings contain only a few ( 1 up to 3) quartets, so data strings are not always of equal length.


                    |--- acquisition/synchronizing phase
                    ||-- Error
        "A"  -Addr.-|| Nbr.Q
        SSSS.DDDD DDAE.LQQQ  T          H          R          W          G           CRC
TX22IT [A    1    D    3                1  0 7 2   2  0 1 B   3  C F E               C4    ] CRC:OK S:A ID:7 NewBatt:0 Error:1 Temp:---   Hum:72  Rain:27.00 Wind:25.40m/s from:270.00 Gust:---      CRC:C4
TX22IT [A    1    D    2                           2  0 1 B   3  D F E               3A    ] CRC:OK S:A ID:7 NewBatt:0 Error:1 Temp:---   Hum:--- Rain:27.00 Wind:25.40m/s from:292.50 Gust:---      CRC:3A
TX22IT [A    1    D    2                           2  0 1 B   3  E F E               17    ] CRC:OK S:A ID:7 NewBatt:0 Error:1 Temp:---   Hum:--- Rain:27.00 Wind:25.40m/s from:315.00 Gust:---      CRC:17
TX22IT [A    1    C    3                1  0 7 3   2  0 1 B              4  0  0  0  8A    ] CRC:OK S:A ID:7 NewBatt:0 Error:0 Temp:---   Hum:73  Rain:27.00 Wind:---      from:---    Gust:0.00 m/s CRC:8A
TX22IT [A    1    C    1                           2  0 1 B                          E     ] CRC:OK S:A ID:7 NewBatt:0 Error:0 Temp:---   Hum:--- Rain:27.00 Wind:---      from:---    Gust:---      CRC:E
TX22IT [A    1    C    2     0  5 5 3              2  0 1 B                          19    ] CRC:OK S:A ID:7 NewBatt:0 Error:0 Temp:15.30 Hum:--- Rain:27.00 Wind:---      from:---    Gust:---      CRC:19

*/

byte TX22IT::CalculateCRC(byte data[]) {
  byte CRC[8];
  byte bits[8];
  int i,j;
  byte val;
  byte DoInvert;
  byte result;
  
  byte len = TX22IT::GetFrameLength(data) -1;

  for(i=0; i<8; i++) {
    CRC[i] = 0;
  }

  for(j=0; j<len; j++) {
    val = data[j];
    
    for(i=0; i<8; i++) {
      switch(i) {
        case 0: if((val & 0x80) != 0) { bits[i] = 1; } else { bits[i] = 0; } break;
        case 1: if((val & 0x40) != 0) { bits[i] = 1; } else { bits[i] = 0; } break;
        case 2: if((val & 0x20) != 0) { bits[i] = 1; } else { bits[i] = 0; } break;
        case 3: if((val & 0x10) != 0) { bits[i] = 1; } else { bits[i] = 0; } break;
        case 4: if((val & 0x8) != 0) { bits[i] = 1; } else { bits[i] = 0; } break;
        case 5: if((val & 0x4) != 0) { bits[i] = 1; } else { bits[i] = 0; } break;
        case 6: if((val & 0x2) != 0) { bits[i] = 1; } else { bits[i] = 0; } break;
        case 7: if((val & 0x1) != 0) { bits[i] = 1; } else { bits[i] = 0; } break;
      }
  
      if(bits[i] == 1) {
        DoInvert = 1 ^ CRC[7];
      }
      else {
        DoInvert = 0 ^ CRC[7];
      }  
      
      CRC[7] = CRC[6];
      CRC[6] = CRC[5];
      CRC[5] = CRC[4] ^ DoInvert;
      CRC[4] = CRC[3] ^ DoInvert;
      CRC[3] = CRC[2];
      CRC[2] = CRC[1];
      CRC[1] = CRC[0];
      CRC[0] = DoInvert;
    }
  }

  result = (CRC[7] << 7) |
           (CRC[6] << 6) |
           (CRC[5] << 5) |
           (CRC[4] << 4) |
           (CRC[3] << 3) |
           (CRC[2] << 2) |
           (CRC[1] << 1) |
           (CRC[0]);
              
  return result;
}

void TX22IT::DecodeFrame(byte *bytes, struct Frame *frame) {
  frame->IsValid = true;
  frame->Header = 0;
  frame->ID = 0;
  frame->NewBatteryFlag = false;
  frame->LowBatteryFlag = false;
  frame->ErrorFlag = false; 

  frame->HasTemperature = false;
  frame->HasHumidity = false;
  frame->HasRain = false;
  frame->HasWindSpeed = false;
  frame->HasWindDirection = false;
  frame->HasWindGust = false;
  frame->HasPressure = false;
  frame->HasDebug = false;
  frame->HasGas = false;
  
  
  frame->Temperature = 0;
  frame->Humidity = 0;
  frame->Rain = 0;
  frame->WindDirection = 0;
  frame->WindSpeed = 0;
  frame->WindGust = 0;
  frame->CRC = 0;

  frame->CRC = bytes[GetFrameLength(bytes) -1];
  if (frame->CRC != CalculateCRC(bytes)) {
    frame->IsValid = false;
  }

  frame->Header = (bytes[0] & 0xF0) >> 4;
  if (frame->Header != 0xA) {
    frame->IsValid = false;
  }

  if (frame->IsValid) {
    frame->ID = ((bytes[0] & 0xF) << 2) | ((bytes[1] & 0xC0) >> 6);

    frame->NewBatteryFlag = (bytes[1] & 0b00100000) > 0;
    frame->ErrorFlag = (bytes[1] & 0b00010000) > 0;
    frame->LowBatteryFlag = (bytes[1] & 0xF) >> 3;

    byte ct = bytes[1] & 0x7;
    for (int i = 0; i < ct; i++) {
      byte byte1 = bytes[2 + i * 2];
      byte byte2 = bytes[3 + i * 2];

      byte type = (byte1 & 0xF0) >> 4;
      byte q1 = (byte1 & 0xF);
      byte q2 = (byte2 & 0xF0) >> 4;
      byte q3 = (byte2 & 0xF);

      switch (type) {
        case 0:
          frame->HasTemperature = true;
          frame->Temperature = (DecodeValue(q1, q2, q3) - 400) / 10.0;
          if (frame->Temperature > 60 || frame->Temperature < -40) {
            frame->IsValid = false;
          }
          break;
        
        case 1:
          frame->HasHumidity= true;
          frame->Humidity = DecodeValue(q1, q2, q3);
          if (frame->Humidity > 100 || frame->Humidity < 0) {
            frame->IsValid = false;
          }
          break;

        case 2:
          if (!frame->ErrorFlag) {
            frame->HasRain = true;
            frame->Rain = q1 * 256 + q2 * 16 + q3;
          }
          break;

        case 3:
          if (!frame->ErrorFlag) {
            frame->HasWindDirection = true;
            frame->HasWindSpeed = true;

            frame->WindDirection = q1 * 22.5;
            frame->WindSpeed = (q2 * 16 + q3) / 10.0;
          }
          break;

        case 4:
          if (!frame->ErrorFlag) {
            frame->HasWindGust = true;
            frame->WindGust = (q2 * 16 + q3) / 10.0;
          }
          break;
      }

    }
    
  }
}


byte TX22IT::GetFrameLength(byte data[]) {
  return 3 + 2 * (data[1] & 0x7);
}


String TX22IT::AnalyzeFrame(byte *data) {
  struct Frame frame;
  DecodeFrame(data, &frame);

  byte frameLength = TX22IT::GetFrameLength(data);

  return WSBase::AnalyzeFrame(data, &frame, frameLength, "TX22IT");

}

String TX22IT::GetFhemDataString(byte *data) {
  String fhemString = "";

  if ((data[0] & 0xA0) == 0xA0) {
    struct Frame frame;
    DecodeFrame(data, &frame);
    if (frame.IsValid) {
      fhemString = BuildFhemDataString(&frame, 1);
    }
  }

  return fhemString;
}

bool TX22IT::TryHandleData(byte *data) {
  String fhemString = GetFhemDataString(data);

  if (fhemString.length() > 0) {
    Serial.println(fhemString);
  }
 
  return fhemString.length() > 0;
}



void TX22IT::EncodeFrame(struct Frame *frame, byte bytes[4]) {

}

bool TX22IT::IsValidDataRate(unsigned long dataRate) {
  return dataRate == 8842ul;
}