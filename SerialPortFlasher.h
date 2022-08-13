#ifndef _SERIALPORTFLASHER_h
#define _SERIALPORTFLASHER_h
#include "Arduino.h"
#include "Updater.h"

class SerialPortFlasher {
public:
  SerialPortFlasher();
  void Begin();
  ////void End();
  bool IsUploading();
  void Add(byte c);
  void Handle();

private:
  enum State {
    GetSize,
    GetBlockSize,
    GetBytes,
    Finished
  };

  ////bool m_isUploading;
  uint m_receivedSize;
  uint m_expectedSize;
  unsigned long m_lastReception;
  String m_sizeBuffer;
  byte m_dataBuffer[256];
  byte m_dataBufferPointer;
  State m_state;
  byte m_blockSize;
  byte m_checksum;
  

};
#endif

