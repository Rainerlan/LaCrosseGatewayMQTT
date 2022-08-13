#ifndef _PCA301_h
#define _PCA301_h

#include "Arduino.h"
#include "RFMxx.h"
#include "PCA301Plug.h"
#include "PCA301PlugList.h"

enum class ExpectedAnswers {
  None = 0,
  On = 1,
  Off = 2,
  Values = 3
};
enum class ActionTypes {
  None = 0,
  Poll = 1,
  Pair = 2,
};

class PCA301Action {
public:
  ActionTypes Action;
  byte ID[3];
  byte Channel;
  unsigned long StartTime;
};


class PCA301 {
public:
  struct Frame {
    byte Channel;
    byte Command;
    byte ID[3];
    byte Data;
    float Power;
    float AccumulatedPower;
    byte CRC1;
    byte CRC2;
    bool IsValid;
    bool IsFromPlug;
    bool IsPairingCommand;
    bool IsOnOffCommand;
    bool IsOn;
  };
  
  ////typedef std::function<String(String key, String Value, bool write)> TSettingsCallback;
  typedef String TSettingsCallback(String key, String value, bool write);
  PCA301();
  void Begin(RFMxx *rfm, unsigned long frequency, word interval, TSettingsCallback callback);
  bool IsInitialized();
  void Handle();
  String GetFhemDataString(byte *data);
  static bool IsValidDataRate(unsigned long dataRate);
  void SendPayload(byte bytes[10], bool isRetry);
  RFMxx *GetUsedRadio();
  typedef void LogItemCallbackType(String);
  void SetLogItemCallback(LogItemCallbackType* callback);
  String AnalyzeFrame(byte *payload);
  void EnableLogging(bool enabled);

protected:
  static const byte FRAME_LENGTH = 12;
  bool m_doLogging;
  RFMxx *m_rfm;
  unsigned long m_lastPoll;
  unsigned long m_lastCommand;
  ExpectedAnswers m_expectedAnswer;
  byte m_retries;
  byte m_lastPayload[10];
  PCA301Action m_nextAction;
  PCA301PlugList m_plugList;

  void DecodeFrame(byte *bytes, struct PCA301::Frame *frame);
  void CalculateCRC(byte data[], byte result[2]);
  LogItemCallbackType *m_logItemCallback;
  void Log(String logItem, bool force=false);
  String GetPlugIdentifier(byte id1, byte id2, byte id3, byte channel);
  TSettingsCallback *m_settingsCallback;
};



#endif

