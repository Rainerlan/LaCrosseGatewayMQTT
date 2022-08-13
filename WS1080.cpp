#include "WS1080.h"

/*
WS 1080  17.241 kbps  868.3 MHz
-------------------------------

A8 C0 58 5E 00 00 00 86 0A D8
ID: 8C, T=  8.8`C, relH= 94%, Wvel=  0.0m/s, Wmax=  0.0m/s, Wdir=SW , Rain=  40.2mm

A8 C0 55 5E 00 00 00 86 04 06
ID: 8C, T=  8.5`C, relH= 94%, Wvel=  0.0m/s, Wmax=  0.0m/s, Wdir=E  , Rain=  40.2mm

A8 C0 50 60 00 00 00 86 04 BF
ID: 8C, T=  8.0`C, relH= 96%, Wvel=  0.0m/s, Wmax=  0.0m/s, Wdir=E  , Rain=  40.2mm

*/


byte WS1080::CalculateCRC(byte data[]) {          
  return SensorBase::CalculateCRC(data, WS1080::FRAME_LENGTH -1);
}

void WS1080::DecodeFrame(byte *bytes, struct Frame *frame) {
  frame->IsValid = true;
  frame->ID = 0;
  frame->NewBatteryFlag = false;
  frame->LowBatteryFlag = false;
  frame->ErrorFlag = false; 

  frame->HasTemperature = true;
  frame->HasHumidity = true;
  frame->HasRain = true;
  frame->HasWindSpeed = true;
  frame->HasWindDirection = true;
  frame->HasWindGust = true;
  frame->HasPressure = false;
  frame->HasDebug = false;
  frame->HasGas = false;
  

  frame->CRC = bytes[WS1080::FRAME_LENGTH - 1];
  if (frame->CRC != CalculateCRC(bytes)) {
    frame->IsValid = false;
  }

  frame->Header = bytes[0] >> 4;
  if (frame->Header != 0xA) {
    frame->IsValid = false;
  }

  if (frame->IsValid) {
    frame->ID = ((bytes[0] & 0xF) << 4) | ((bytes[1] & 0xF0) >> 4);

    frame->NewBatteryFlag = false;
    frame->ErrorFlag = false;
    frame->LowBatteryFlag = false;

    // Temperature (°C)
    byte sign = (bytes[1] >> 3) & 1;
    int temp = ((bytes[1] & 0x07) << 8) | bytes[2];
    if (sign) {
      temp = (~temp) + sign;
    }
    frame->Temperature = temp * 0.1;

    // Humidity (%rH)
    frame->Humidity = bytes[3] & 0x7F;

    // Wind speed (m/s)
    frame->WindSpeed = bytes[4] * 0.34;

    // Wind gust (m/s)
    frame->WindGust = bytes[5] * 0.34;
    
    //  Rain (0.5 mm steps)
    frame->Rain = (((bytes[6] & 0x0F) << 8) | bytes[7]) * 0.6;
    
    // Wind direction (degree  N=0, NNE=22.5, S=180, ... )
    frame->WindDirection = 22.5 * (bytes[8] & 0x0F);

  }
}


String WS1080::AnalyzeFrame(byte *data) {
  struct Frame frame;
  DecodeFrame(data, &frame);

  byte frameLength = WS1080::FRAME_LENGTH;

  return WSBase::AnalyzeFrame(data, &frame, frameLength, "WS1080");
}

String WS1080::GetFhemDataString(byte *data) {
  String fhemString = "";

  if ((data[0] >> 4) == 0x0A) {
    struct Frame frame;
    DecodeFrame(data, &frame);
    if (frame.IsValid) {
      fhemString = BuildFhemDataString(&frame, 3);
    }
  }

  return fhemString;
}

bool WS1080::TryHandleData(byte *data) {
  String fhemString = GetFhemDataString(data);

  if (fhemString.length() > 0) {
    Serial.println(fhemString);
  }
 
  return fhemString.length() > 0;
}



bool WS1080::IsValidDataRate(unsigned long dataRate) {
  return dataRate == 17241ul;
}