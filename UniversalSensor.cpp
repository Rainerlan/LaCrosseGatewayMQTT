#include "UniversalSensor.h"

/*
 0              5             10             15             20             25             30
--------------------------------------------------------------------------------------------
CD ID TT TT HH RR RR DD DD SS SS GG GG FF PP PP G1 G1 G1 G2 G2 G2 LU LU LU VV VO xx xx xx CR CR
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |--CRC 16
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |---- Debug            1er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |------- Debug            256er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |---------- Debug            65536er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |------------- Voltage*10
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |---------------- Version*10       V3.5 == 35
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |------------------- LUX              1er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |---------------------- LUX              256er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |------------------------- LUX              65536er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |---------------------------- GAS2             1er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |------------------------------- GAS2             256er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |---------------------------------- GAS2             65536er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |------------------------------------- GAS1             1er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |---------------------------------------- GAS1             256er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |------------------------------------------- GAS1             65536er
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |---------------------------------------------- Pressure*10      1er       hPa
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |------------------------------------------------- Pressure*10      256er
|  |  |  |  |  |  |  |  |  |  |  |  |  |---------------------------------------------------- Flags            see below
|  |  |  |  |  |  |  |  |  |  |  |  |------------------------------------------------------- WindGust*10      1er       m/s
|  |  |  |  |  |  |  |  |  |  |  |---------------------------------------------------------- WindGust*10      256er
|  |  |  |  |  |  |  |  |  |  |------------------------------------------------------------- WindSpeed*10     1er       m/s
|  |  |  |  |  |  |  |  |  |---------------------------------------------------------------- WindSpeed*10     256er
|  |  |  |  |  |  |  |  |------------------------------------------------------------------- WindDirection*10 1er       0.0 ... 365.0 Degrees
|  |  |  |  |  |  |  |---------------------------------------------------------------------- WindDirection*10 256er
|  |  |  |  |  |  |------------------------------------------------------------------------- Rain*0.5mm       1er       mm
|  |  |  |  |  |---------------------------------------------------------------------------- Rain*0.5mm       256er     mm
|  |  |  |  |------------------------------------------------------------------------------- Humidity         plain     %
|  |  |  |---------------------------------------------------------------------------------- Temp*10+1000     1er       °C
|  |  |------------------------------------------------------------------------------------- Temp*10+1000     256er     °C
|  |---------------------------------------------------------------------------------------- Sensor ID        1 ... 255
|------------------------------------------------------------------------------------------- fix "CD"

Missing values shall be set to 0xFF
Example: CD 01 FF FF 57 FF FF ... 44(CRC) if only humidity is available

Flags: 128  64  32  16  8   4   2   1
         |   |   |
         |   |   |-- New battery
         |   |------ ERROR
         |---------- Low battery

Protocol from LGW to FHEM is the same but:
- starts with "OK WS ID 5" instead of "0xCD ID" (type 5 UniversalSensor)
- ends with the byte before CRC

Needs LGW V1.31 or higher
Needs 36_LaCrosse.pm newer than 30.12.2017

*/

void UniversalSensor::Add2BytesF(float value, byte *bytes, byte position) {
  if(value >= 65536) {
    bytes[position +0] = 0xFF;
    bytes[position +1] = 0xFF;
  }
  else {
    bytes[position +0] = (byte)((uint16_t)value >> 8);
    bytes[position +1] = (byte)((uint16_t)value & 0xFF);
  }
}

void UniversalSensor::Add2BytesI(uint16_t value, byte *bytes, byte position) {
  bytes[position + 0] = (byte)(value >> 8);
  bytes[position + 1] = (byte)(value & 0xFF);
}

void UniversalSensor::Add3BytesI(uint32_t value, byte *bytes, byte position) {
  bytes[position +0] = (byte)(value >> 16);
  bytes[position +1] = (byte)(value >> 8);
  bytes[position +2] = (byte)(value & 0xFF);
}

void UniversalSensor::EncodeFrame(struct UniversalSensor::Frame *frame, byte bytes[FRAME_LENGTH]) {
  bytes[0] = UNIVERSAL_SENSOR_HEADER;
  bytes[1] = frame->ID;
  Add2BytesF(frame->Temperature * 10 + 1000, bytes, 2);
  bytes[4] = frame->Humidity;
  Add2BytesI(frame->Rain, bytes, 5);
  Add2BytesF(frame->WindDirection * 10.0, bytes, 7);
  Add2BytesF(frame->WindSpeed * 10.0, bytes, 9);
  Add2BytesF(frame->WindGust * 10.0, bytes, 11);
  bytes[13] = frame->Flags;
  Add2BytesF(frame->Pressure * 10.0, bytes, 14);
  Add3BytesI(frame->Gas1, bytes, 16);
  Add3BytesI(frame->Gas2, bytes, 19);
  Add3BytesI(frame->Lux, bytes, 22);
  bytes[25] = (byte)(frame->Version);
  bytes[26] = (byte)(frame->Voltage * 10.0);
  Add3BytesI(frame->Debug, bytes, 27);

  uint16_t crc16 = CalculateCRC16(bytes, FRAME_LENGTH - 2);
  bytes[FRAME_LENGTH - 2] = (byte)(crc16 >> 8);
  bytes[FRAME_LENGTH - 1] = (byte)(crc16 & 0xFF);

}

String UniversalSensor::GetFhemDataString(byte *data) {
  String result = "";

  if (data[0] == UNIVERSAL_SENSOR_HEADER) {
    uint16_t crc16 = CalculateCRC16(data, FRAME_LENGTH - 2);
    if((byte)(crc16 >> 8) == data[FRAME_LENGTH -2] && (byte)(crc16 & 0xFF) == data[FRAME_LENGTH - 1]) {
      result += "OK WS ";
      result += String(data[1], DEC);
      result += " 5";
      for(byte i = 2; i < FRAME_LENGTH; i++) {
        result += " ";
        result += String(data[i], DEC);
      }
    }
  }

  return result;
}

bool UniversalSensor::IsValidDataRate(unsigned long dataRate) {
  return dataRate == 17241ul;
}