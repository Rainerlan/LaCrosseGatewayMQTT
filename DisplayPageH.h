#ifndef _DISPLAY_PAGE_H_h
#define _DISPLAY_PAGE_H_h

#include "Arduino.h"
#include "Display.h"

void DrawPageH(Display *display) {
  display->Clear(DisplayArea_Pages);
  
  display->DrawXBM(DisplayArea_PageIcon, xbm_humidity);
  display->Print(String(display->Values.Humidity) + " %rH", DisplayArea_BigPageTextShort, OLED::Alignments::Center, true);

  display->Refresh();
}

#endif