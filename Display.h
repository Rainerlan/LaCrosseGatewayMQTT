
#ifndef _DISPLAY_h
#define _DISPLAY_h

#include "Arduino.h"
#include "StateManager.h"
#include "OLED.h"
#include "DisplayAreas.h"
#include "DisplayValues.h"
#include "WSBase.h"

class Display {
protected:
  OLED m_display;
  bool m_isConnected;
  StateManager *m_stateManager;
  unsigned long m_lastHeaderUpdate;
  unsigned long m_lastPageUpdate;
  unsigned int m_currentProgress;
  unsigned int m_maxProgress;
  String m_mode;
  byte m_currentPage;
  unsigned long m_offTime;
  int m_interval;
  int32_t m_rssi;
  void UpdatePages();
  bool m_wifiFlag;
  bool m_fhemFlag;
  bool m_addonFlag;

public:
  DisplayValues Values;
  Display();
  bool Begin(StateManager *stateManager, int startMode, OLED::Controllers controller = OLED::Controllers::SSD1306);
  void Print(String text, const int area[4], OLED::Alignments alignment = OLED::Alignments::Left, bool largeFont = false);
  void Command(String command);
  bool IsConnected();
  void Handle(WSBase::Frame frame, unsigned int framesPerMinute, String version, int32_t rssi, bool wifiFlag);
  void Clear();
  void Clear(const int area[4]);
  void DrawXBM(const int area[4], const char *xbm);
  void SetWifiFlag(bool flag);
  void SetFhemFlag(bool flag);
  void SetAddonFlag(bool flag);
  void SetLED(bool show);
  void ShowProgress(unsigned int maxValue, String message);
  void MoveProgress(unsigned long offset=0);
  void HideProgress();
  void PushContent();
  void PopContent();
  void Refresh();
  bool IsOn();

};


#endif

