#ifndef _DISPLAY_PAGE_P_h
#define _DISPLAY_PAGE_P_h

#include "Arduino.h"
#include "Display.h"

void DrawPageP(Display *display) {
  display->Clear(DisplayArea_Pages);
  
  display->DrawXBM(DisplayArea_PageIcon, xbm_pressure);
  display->Print(String(display->Values.Pressure) + " hPa", DisplayArea_BigPageTextShort, OLED::Alignments::Center, true);

  display->Refresh();
}

#endif