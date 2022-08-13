#ifndef _UNIVERSALSENSOR_h
#define _UNIVERSALSENSOR_h

#include "SensorBase.h"

#define UNIVERSAL_SENSOR_HEADER         0xCD

class UniversalSensor : public SensorBase {
public:
  struct Frame {
    byte  Header;
    byte  ID;
    float Temperature;     // °C 
    byte  Humidity;        // %rH
    uint16_t Rain;         // mm      (0.5 steps)
    float WindDirection;   // Degree  (0.0 - 365.0)
    float WindSpeed;       // m/s
    float WindGust;        // m/s
    byte  Flags;           // see cpp 
    float Pressure;        // hPa
    uint32_t Gas1;         // Gas1
    uint32_t Gas2;         // Gas2
    uint32_t Lux;          // Lux
    byte Version;          // *10 V3.6 is 36
    float Voltage;         // V
    uint32_t Debug;        // 3 byte, range 0 ... 16.777.215
    uint16_t CRC;
    bool  IsValid;
  };
  static const byte FRAME_LENGTH = 32;

  static void EncodeFrame(struct UniversalSensor::Frame *frame, byte bytes[FRAME_LENGTH]);
  static String GetFhemDataString(byte *data);
  static bool IsValidDataRate(unsigned long dataRate);

protected:
  static void Add2BytesF(float value, byte *bytes, byte position);
  static void Add2BytesI(uint16_t value, byte *bytes, byte position);
  static void Add3BytesI(uint32_t value, byte *bytes, byte position);

};


#endif

