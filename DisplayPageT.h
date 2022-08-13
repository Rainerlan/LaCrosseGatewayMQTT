#ifndef _DISPLAY_PAGE_T_h
#define _DISPLAY_PAGE_T_h

#include "Arduino.h"
#include "Display.h"
#include "HTML.h"

void DrawPageT(Display *display) {
  display->Clear(DisplayArea_Pages);
  
  display->DrawXBM(DisplayArea_PageIcon, xbm_temperature);
  display->Print(String(display->Values.Temperature, 1) + HTML::UTF8ToASCII(" °C"), DisplayArea_BigPageTextShort, OLED::Alignments::Center, true);

  display->Refresh();
}

#endif