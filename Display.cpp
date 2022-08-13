#include "Display.h"
#include "XBM.h"
#include "DisplayPageF.h"
#include "DisplayPageT.h"
#include "DisplayPageH.h"
#include "DisplayPageP.h"
#include "DisplayPageS.h"


Display::Display() {
  m_isConnected = false;
  m_lastHeaderUpdate = 0;
  m_lastPageUpdate = 0;
  m_wifiFlag = false;
  m_fhemFlag = false;
  m_addonFlag = false;
  m_mode = "";
  m_currentPage = -1;
  m_offTime = 0;
}

bool Display::IsConnected() {
  return m_isConnected;
}

bool Display::Begin(StateManager *stateManager, int startMode, OLED::Controllers controller) {
  m_stateManager = stateManager;
  m_interval = 3;

  m_isConnected = m_display.Begin(0x3C, OLED::Orientations::UpsideDown, controller);

  if(m_isConnected) {
    m_display.Clear();
    
    if (startMode == -1) {
      m_display.On();
      m_offTime = -1;
    }
    else {
      m_offTime = millis() + startMode * 1000;
    }
    
    m_display.SetFont(Roboto_Light_15);
    m_display.Print(DisplayArea_Line2, "LGW V" + m_stateManager->GetVersion(), OLED::Alignments::Center);
    
    m_display.Refresh();
    m_display.SetFont(Roboto_Light_13);
  }

  return m_isConnected;
}

void Display::SetWifiFlag(bool flag) {
  m_wifiFlag = flag;
}

void Display::SetFhemFlag(bool flag) {
  m_fhemFlag = flag;
}

void Display::SetAddonFlag(bool flag) {
  m_addonFlag = flag;
}

void Display::SetLED(bool show) {
  Clear(DisplayArea_LED);
  if (show) {
    m_display.DrawRect(DisplayArea_LED, true, OLED::Colors::White);
  }
  m_display.Refresh();
}

void Display::ShowProgress(unsigned int maxValue, String message) {
  m_maxProgress = maxValue;
  m_currentProgress = 0;

  Print(message, DisplayArea_Line1, OLED::Alignments::Center);
  m_display.DrawRect(DisplayArea_ProgressBar, true, OLED::Colors::Black);
  m_display.DrawRect(DisplayArea_ProgressBar, false);
  
  Print("0%", DisplayArea_Line3, OLED::Alignments::Center);
  m_display.Refresh();
}

void Display::MoveProgress(unsigned long offset) {
  if (offset == 0) {
    m_currentProgress++;
  }
  else {
    m_currentProgress += offset;
  }
  byte percent = (float)((float)m_currentProgress / (float)m_maxProgress) * 100;
  byte width = percent * 1.24;

  m_display.DrawRect(2, DisplayArea_ProgressBar[1] + 2, width, DisplayArea_ProgressBar[3] -3, true);
  Print(String(percent) + "%", DisplayArea_Line3, OLED::Alignments::Center);
  m_display.Refresh();
}

void Display::HideProgress() {
  Clear(DisplayArea_Pages);
  m_display.Refresh();
}

void Display::PushContent() {
  m_display.PushContent();
}

void Display::PopContent() {
  m_display.PopContent();
}

void Display::Refresh() {
  m_display.Refresh();
}

bool Display::IsOn() {
  return m_display.IsOn();
}

void Display::Print(String text, const int area[4], OLED::Alignments alignment, bool largeFont) {
  m_display.DrawRect(area, true, OLED::Colors::Black);
  if (largeFont) {
    m_display.SetFont(Roboto_Light_20);
  }
  else {
    m_display.SetFont(Roboto_Light_13);
  }
  m_display.Print(area, text, alignment);
  m_display.SetFont(Roboto_Light_13);
}

void Display::DrawXBM(const int area[4], const char *xbm) {
  m_display.DrawXBM(area[0], area[1], area[2], area[3], xbm);
}

void Display::Command(String command) {
  String lowerCommand = command;
  lowerCommand.toLowerCase();

  if (lowerCommand == "off") {
    m_display.Off();
  }
  else {
    m_display.On();
  }

  int pos = lowerCommand.indexOf('=');
  if (pos != -1) {
    String value = command.substring(pos +1);
    lowerCommand = lowerCommand.substring(0, pos);

    if (lowerCommand == "mode") {
      m_mode = value;
      m_currentPage = -1;
      m_lastPageUpdate = 0;
    }
    
    else if (lowerCommand == "interval") {
      m_interval = value.toInt();
    }
    
    else if (lowerCommand == "show") {
      Values.FhemText1 = "";
      Values.FhemText2 = "";
      Values.FhemText3 = "";
      Values.FhemIcon = "";

      value += ",";
      String part = "";
      byte idx = 0;
      for (byte b = 0; b < value.length(); b++) {
        char c = value[b];
        if (c != ',') {
          part += c;
        }
        else {
          switch (idx) {
          case 0:
            Values.FhemText1 = part;
            break;
          case 1:
            Values.FhemText2 = part;
            break;
          case 2:
            Values.FhemText3 = part;
            break;
          case 3:
            Values.FhemIcon = part;
            break;
          default:
            break;
          }
          idx++;
          part = "";
        }
      }

      m_mode = "f";
      m_currentPage = -1;
      m_lastPageUpdate = 0;
    }

  }

}

void Display::UpdatePages() {
  char nextPage = '-';
  
  if (m_mode.length() > 0) {
    if (m_currentPage == -1) {
      m_currentPage = 0;
      nextPage = m_mode[m_currentPage];
    }
    else {
      m_currentPage++;
      if (m_currentPage > m_mode.length() -1) {
        m_currentPage = 0;
      }
      nextPage = m_mode[m_currentPage];
    }
  }


  if (nextPage == 'f') {
    DrawPageF(this);
  }
  else if (nextPage == 't') {
    DrawPageT(this);
  }
  else if (nextPage == 'h') {
    DrawPageH(this);
  }
  else if (nextPage == 's') {
    DrawPageS(this);
  }
  else if (nextPage == 'p') {
    DrawPageP(this);
  }
}

void Display::Handle(WSBase::Frame frame, unsigned int framesPerMinute, String version, int32_t rssi, bool wifiFlag) {
  m_wifiFlag = wifiFlag;
  if (IsConnected()) {
    if (m_offTime >= 0 && millis() > m_offTime) {
      m_display.Off();
      m_offTime = -1;
    }

    if (millis() < m_lastHeaderUpdate) {
      m_lastHeaderUpdate = 0;
    }
    if (millis() > m_lastHeaderUpdate + 1000) {
      Clear(DisplayArea_RSSI);
      String text;
      text += rssi;
      text += " dBm";
      m_display.Print(DisplayArea_RSSI, text, OLED::Alignments::Right);
      
      if (m_wifiFlag) {
        m_display.DrawXBM(DisplayArea_WifiFlag, xbm_wifi_bits);
      }
      else {
        Clear(DisplayArea_WifiFlag);
      }

      if (m_fhemFlag) {
        m_display.DrawXBM(DisplayArea_FhemFlag, xbm_fhem_bits);
      }
      else {
        Clear(DisplayArea_FhemFlag);
      }

      if (m_addonFlag) {
        m_display.DrawXBM(DisplayArea_AddonFlag, xbm_cpu_bits);
      }
      else {
        Clear(DisplayArea_AddonFlag);
      }

      m_display.Refresh();
      m_stateManager->SetDisplayStatus(IsOn() ? "on" : "off");
      m_lastHeaderUpdate = millis();
    }

    if (millis() < m_lastPageUpdate) {
      m_lastPageUpdate = 0;
    }
    if (millis() > m_lastPageUpdate + m_interval * 1000) {
      Values.Temperature = frame.Temperature;
      Values.Humidity = frame.Humidity;
      Values.Pressure = frame.Pressure;
      Values.FramesPerMinute = framesPerMinute;
      Values.Version = version;
      UpdatePages();

      m_lastPageUpdate = millis();
    }

  }
}

void Display::Clear() {
  m_display.Clear();
}
void Display::Clear(const int area[4]) {
  m_display.DrawRect(area[0], area[1], area[2], area[3], true, OLED::Colors::Black);
}



