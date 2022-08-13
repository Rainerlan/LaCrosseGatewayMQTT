#ifndef _LEVELSENDERLIB_h
#define _LEVELSENDERLIB_h

#include "Arduino.h"
#include "SensorBase.h"

class LevelSenderLib : public SensorBase {
public:
  struct Frame {
    byte  Header;
    byte  ID;
    float Level;
    float Temperature;
    float Voltage;
    byte  CRC;
    bool  IsValid;
  };
  static byte CalculateCRC(byte data[]);
  static const byte FRAME_LENGTH = 6;
  static void EncodeFrame(struct Frame *frame, byte *bytes);
  static void DecodeFrame(byte *data, struct Frame *frame);
  static String AnalyzeFrame(byte *data);
  static bool TryHandleData(byte *data);
  static String GetFhemDataString(byte *data);
  static bool IsValidDataRate(unsigned long dataRate);
  

protected:
  static String BuildFhemDataString(struct LevelSenderLib::Frame *frame);

};

#endif

