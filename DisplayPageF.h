#ifndef _DISPLAY_PAGE_F_h
#define _DISPLAY_PAGE_F_h

#include "Arduino.h"
#include "Display.h"

void DrawPageF(Display *display) {
  display->Clear(DisplayArea_Pages);
  
  if (display->Values.FhemIcon == "t") {
    display->DrawXBM(DisplayArea_PageIcon, xbm_temperature);
  }
  else if (display->Values.FhemIcon == "h") {
    display->DrawXBM(DisplayArea_PageIcon, xbm_humidity);
  }
  else if (display->Values.FhemIcon == "p") {
    display->DrawXBM(DisplayArea_PageIcon, xbm_pressure);
  }
  else if (display->Values.FhemIcon == "s") {
    display->DrawXBM(DisplayArea_PageIcon, xbm_system);
  }
  else if (display->Values.FhemIcon == "i") {
    display->DrawXBM(DisplayArea_PageIcon, xbm_info);
  }
  else if (display->Values.FhemIcon == "w") {
    display->DrawXBM(DisplayArea_PageIcon, xbm_warning);
  }
  else if (display->Values.FhemIcon == "e") {
    display->DrawXBM(DisplayArea_PageIcon, xbm_error);
  }

  if (display->Values.FhemText1.length() > 0 && display->Values.FhemText2.length() + display->Values.FhemText3.length() == 0) {
    display->Print(display->Values.FhemText1, display->Values.FhemIcon.length() == 0 ? DisplayArea_BigPageTextLong : DisplayArea_BigPageTextShort, OLED::Alignments::Center, true);
  }
  else {
    display->Print(display->Values.FhemText1, display->Values.FhemIcon.length() == 0 ? DisplayArea_Line1 : DisplayArea_ShortLine1);
    display->Print(display->Values.FhemText2, display->Values.FhemIcon.length() == 0 ? DisplayArea_Line2 : DisplayArea_ShortLine2);
    display->Print(display->Values.FhemText3, display->Values.FhemIcon.length() == 0 ? DisplayArea_Line3 : DisplayArea_ShortLine3);
  }
  display->Refresh();
}

#endif