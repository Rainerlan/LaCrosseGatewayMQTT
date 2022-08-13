#ifndef _EC3000_h
#define _EC3000_h

#include "Arduino.h"
#include "SensorBase.h"
#include "RFMxx.h"

class EC3000 : public SensorBase {

public:
  struct Frame {
    word  ID;
    uint32_t TotalSeconds;
    uint32_t OnSeconds;
    float Consumption;
    float Power;
    float MaximumPower;
    byte NumberOfResets;
    byte Reception;
    bool IsOn;
    word  CRC;
    bool  IsValid;
  };

  static const byte FRAME_LENGTH = 41;
  static const byte PAYLOAD_SIZE = 64;
  static void DecodeFrame(byte *payload, struct EC3000::Frame *frame);
  static String AnalyzeFrame(byte *payload);
  static String GetFhemDataString(byte *payload);
  static bool TryHandleData(byte *payload);
  static bool IsValidDataRate(unsigned long dataRate);

protected:
  static String BuildFhemDataString(struct EC3000::Frame *frame);
  static void DescramblePayload(byte* payload);
  static word UpdateCRC(word crc, byte data);
  static byte Count1bits(uint32_t v);
  static word ShiftReverse(byte *payload);
  static void ShiftLeft(byte * payload, byte blen, byte shift);
  static void Del0BitsAndRevBits(byte * payload, byte blen); 
  
};


#endif

