#ifndef _WATCHDOG_h
#define _WATCHDOG_h

#include "Arduino.h"

typedef void DispatchCallbackType(String);

class Watchdog {
 private:
   unsigned long m_lastTrigger = 0;
   uint m_timeout;
   bool m_enabled;
   
 public:
   DispatchCallbackType *m_dispatchCallback;
   Watchdog();
   void Begin(DispatchCallbackType *callback);
   void Handle();
   void Command(String command);
   
};

#endif

