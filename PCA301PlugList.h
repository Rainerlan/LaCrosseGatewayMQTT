#ifndef _PCA301PLUGLIST_h
#define _PCA301PLUGLIST_h

#include "Arduino.h"
#include "HashMap.h"
#include "PCA301Plug.h"
#include <functional>

class PCA301PlugList{
public:
  typedef std::function<void(byte payload[10])> TCallbackFunction;
  typedef std::function<void(String)> TLogItemCallback;
  typedef std::function<String(String key, String Value, bool write)> TSettingsCallback;

  PCA301PlugList();
  void Begin(word pollInterval);
  void HandleReceivedPlug(byte id[3], byte channel);
  void Poll();
  byte GetNextFreeChannel();
  void OnSendPayload(TCallbackFunction fn);
  void SetLogItemCallback(TLogItemCallback callback);
  void SetSettingsCallback(TSettingsCallback callback);
  bool IsKnownPlug(byte id[3]);

protected:
  HashMap<String, PCA301Plug, 30> m_plugs;
  unsigned long m_lastPoll;
  word m_pollInterval;
  PCA301PlugList::TCallbackFunction m_sendPayloadCallback;
  PCA301PlugList::TLogItemCallback m_logItemCallback;
  PCA301PlugList::TSettingsCallback m_settingCallback;
  void Log(String logItem);

};


#endif

