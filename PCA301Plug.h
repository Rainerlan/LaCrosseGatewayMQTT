#ifndef _PCA301PLUG_h
#define _PCA301PLUG_h

#include "Arduino.h"

class PCA301Plug {
public:
  byte ID[3];
  byte Channel;
  unsigned long LastPoll;

  PCA301Plug();
  String GetIdString();
  static String BuildIdString(byte id[3]);
  void SetIdString(String id);


private:



};


#endif

