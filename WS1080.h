#ifndef _WS1080_h
#define _WS1080_h

#include "Arduino.h"
#include "WSBase.h"


class WS1080 : public WSBase {
public:
  static const byte FRAME_LENGTH = 10;
  static byte CalculateCRC(byte data[]);
  static void DecodeFrame(byte *bytes, struct WS1080::Frame *frame);
  static String AnalyzeFrame(byte *data);
  static bool TryHandleData(byte *data);
  static String GetFhemDataString(byte *data);
  static bool IsValidDataRate(unsigned long dataRate);


protected:



};


#endif

