#include "WSBase.h"

bool WSBase::m_sendDecimals = false;

String WSBase::BuildFhemDataString(struct Frame *frame, byte sensorType) {
  /* Format
  OK WS 60  1   4   193 52    2 88  4   101 15  20   ID=60  21.7°C  52%rH  600mm  Dir.: 112.5°  Wind:15m/s  Gust:20m/s
  OK WS 213 4   5   126 40  255 255 255 255 255 255 255 255 0   40  53  0   48  57
  OK WS ID  XXX TTT TTT HHH RRR RRR DDD DDD SSS SSS GGG GGG FFF PPP PPP GAS GAS GAS DEB DEB DEB LUX LUX LUX
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |-- LUX * 1 
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |------ LUX * 256
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |---------- LUX * 65536
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |-------------- DebugInfo * 1 
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |------------------ DebugInfo * 256
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |---------------------- DebugInfo * 65536 
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |-------------------------- GAS * 1
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |------------------------------ GAS * 256
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |---------------------------------- GAS * 65536
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |-------------------------------------- Pressure LSB
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |------------------------------------------ Pressure MSB
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |   |-- Flags *
  |  |  |   |   |   |   |   |   |   |   |   |   |   |   |------ WindGust * 10 LSB (0.0 ... 50.0 m/s)           FF/FF = none
  |  |  |   |   |   |   |   |   |   |   |   |   |   |---------- WindGust * 10 MSB
  |  |  |   |   |   |   |   |   |   |   |   |   |-------------- WindSpeed  * 10 LSB(0.0 ... 50.0 m/s)          FF/FF = none
  |  |  |   |   |   |   |   |   |   |   |   |------------------ WindSpeed  * 10 MSB
  |  |  |   |   |   |   |   |   |   |   |---------------------- WindDirection * 10 LSB (0.0 ... 365.0 Degrees) FF/FF = none
  |  |  |   |   |   |   |   |   |   |-------------------------- WindDirection * 10 MSB
  |  |  |   |   |   |   |   |   |------------------------------ Rain LSB (0 ... 9999 mm)                       FF/FF = none
  |  |  |   |   |   |   |   |---------------------------------- Rain MSB
  |  |  |   |   |   |   |-------------------------------------- Humidity (1 ... 99 %rH)                        FF = none
  |  |  |   |   |   |------------------------------------------ Temp * 10 + 1000 LSB (-40 ... +60 °C)          FF/FF = none
  |  |  |   |   |---------------------------------------------- Temp * 10 + 1000 MSB
  |  |  |   |-------------------------------------------------- Sensor type (1=TX22IT, 2=NodeSensor, 3=WS1080)
  |  |  |------------------------------------------------------ Sensor ID (1 ... 63)
  |  |--------------------------------------------------------- fix "WS"
  |------------------------------------------------------------ fix "OK"

  * Flags: 128  64  32  16  8   4   2   1
  |   |   |
  |   |   |-- New battery
  |   |------ ERROR
  |---------- Low battery
  */

  String pBuf = "";

  // Check if data is in the valid range
  bool isValid = true;
  if (frame->HasTemperature && (frame->Temperature < -40.0 || frame->Temperature > 59.9)) {
    isValid = false;
  }
  if (frame->HasHumidity && (frame->Humidity < 1 || frame->Humidity > 100)) {
    isValid = false;
  }

  if (isValid) {
    pBuf += "OK WS ";
    pBuf += frame->ID;
    pBuf += " ";
    pBuf += sensorType;

    // add temperature
    pBuf += AddWord(frame->Temperature * 10 + 1000, frame->HasTemperature);

    // add humidity
    pBuf += AddByte(frame->Humidity, frame->HasHumidity);

    // add rain
    pBuf += AddWord(frame->Rain, frame->HasRain);

    // add wind direction
    pBuf += AddWord(frame->WindDirection * 10, frame->HasWindDirection);

    // add wind speed
    pBuf += AddWord(frame->WindSpeed * 10, frame->HasWindSpeed);

    // add gust
    pBuf += AddWord(frame->WindGust * 10, frame->HasWindGust);

    // add Flags
    byte flags = 0;
    if (frame->NewBatteryFlag) {
      flags += 1;
    }
    if (frame->ErrorFlag) {
      flags += 2;
    }
    if (frame->LowBatteryFlag) {
      flags += 4;
    }
    pBuf += AddByte(flags, true);

    // add pressure
    pBuf += AddWord(m_sendDecimals ? frame->Pressure * 10 : frame->Pressure, frame->HasPressure);
    

    // add Gas
    pBuf += AddTreeBytes(frame->Gas, frame->HasGas);

    // add debug
    pBuf += AddTreeBytes(frame->Debug, frame->HasDebug);

    // add Illuminance
    pBuf += AddTreeBytes(frame->Illuminance, frame->HasIlluminance);
    
  }

  return pBuf;
}


String WSBase::AddWord(word value, bool hasValue) {
  String result;

  if (!hasValue) {
    value = 0xFFFF;
  }

  result += ' ';
  result += (byte)(value >> 8);
  result += ' ';
  result += (byte)(value);

  return result;
}

String WSBase::AddByte(byte value, bool hasValue) {
  String result;
  result += ' ';
  result += hasValue ? value : 0xFF;

  return result;
}

String WSBase::AddTreeBytes(int32_t value, bool hasValue) {
  String result;

  if(!hasValue) {
    value = 0xFFFFFF;
  }

  result += ' ';
  result += (byte)(value >> 16);
  result += ' ';
  result += (byte)(value >> 8);
  result += ' ';
  result += (byte)(value);

  return result;
}

float WSBase::DecodeValue(byte q1, byte q2, byte q3) {
  float result = 0;

  result += q1 * 100;
  result += q2 * 10;
  result += q3;

  return result;
}

String WSBase::AnalyzeFrame(byte *data, Frame *frame, byte frameLength, String prefix) {
  String result;

  // Show the raw data bytes
  result += prefix;
  result += " [";
  for (int i = 0; i < frameLength; i++) {
    result += String(data[i], HEX);
    if (i < frameLength) {
      result += " ";
    }
  }
  result += "]";

  // CRC
  if (!frame->IsValid) {
    result += " CRC:WRONG";
  }
  else {
    result += " CRC:OK";

    // Start
    result += " S:";
    result += String(frame->Header, HEX);

    // Sensor ID
    result += " ID:";
    result += String(frame->ID, HEX);

    // New battery flag
    result += " NewBatt:";
    result += String(frame->NewBatteryFlag, DEC);

    // Low battery flag
    result += " LowBatt:";
    result += String(frame->LowBatteryFlag, DEC);

    // Error flag
    result += " Error:";
    result += String(frame->ErrorFlag, DEC);

    // Temperature
    result += " Temp:";
    if (frame->HasTemperature) {
      result += frame->Temperature;
    }
    else {
      result += "---";
    }

    // Humidity
    result += " Hum:";
    if (frame->HasHumidity) {
      result += frame->Humidity;
    }
    else {
      result += "---";
    }

    // Rain
    result += " Rain:";
    if (frame->HasRain) {
      result += frame->Rain;
    }
    else {
      result += "---";
    }

    // Wind speed
    result += " Wind:";
    if (frame->HasWindSpeed) {
      result += frame->WindSpeed;
      result += "m/s";
    }
    else {
      result += "---";
    }

    // Wind direction
    result += " from:";
    if (frame->HasWindDirection) {
      result += frame->WindDirection;
    }
    else {
      result += "---";
    }

    // Wind gust
    result += " Gust:";
    if (frame->HasWindGust) {
      result += frame->WindGust;
      result += " m/s";
    }
    else {
      result += "---";
    }

    // CRC
    result += " CRC:";
    result += String(frame->CRC, HEX);

  }

  return result;
}