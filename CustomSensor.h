#ifndef _CUSTOMSENSOR_h
#define _CUSTOMSENSOR_h

#include "Arduino.h"
#include "SensorBase.h"
#include "RFMxx.h"

#define CUSTOM_SENSOR_HEADER 0xCC
#define CS_PL_BUFFER_SIZE 128

class CustomSensor : public SensorBase {
public:
  struct Frame {
    byte  ID;
    byte  CRC;
    bool  IsValid;
    byte  Data[128];
    byte  NbrOfDataBytes;
  };

  static byte GetFrameLength(byte data[]);
  static void EncodeFrame(struct CustomSensor::Frame *frame, byte bytes[CS_PL_BUFFER_SIZE]);
  static void DecodeFrame(byte *bytes, struct CustomSensor::Frame *frame);
  static String AnalyzeFrame(byte *data);
  static bool TryHandleData(byte *data);
  static String GetFhemDataString(byte *data);
  static bool IsValidDataRate(unsigned long dataRate);
  static void SendFrame(struct CustomSensor::Frame *frame, RFMxx *rfm, unsigned long dataRate);


protected:
  unsigned long m_dataRate;
  static String BuildFhemDataString(struct CustomSensor::Frame *frame);

};


#endif

