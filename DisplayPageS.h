#ifndef _DISPLAY_PAGE_S_h
#define _DISPLAY_PAGE_S_h

#include "Arduino.h"
#include "Display.h"

void DrawPageS(Display *display) {
  display->Clear(DisplayArea_Pages);
  
  display->DrawXBM(DisplayArea_PageIcon, xbm_system);
  display->Print("LGW V " + display->Values.Version, DisplayArea_ShortLine1);
  display->Print("FPM: " + String(display->Values.FramesPerMinute), DisplayArea_ShortLine2);
  display->Print("Heap: " + String(ESP.getFreeHeap()), DisplayArea_ShortLine3);


  display->Refresh();
}

#endif