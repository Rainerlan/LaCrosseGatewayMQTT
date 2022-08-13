#ifndef _DISPLAY_AREAS_h
#define _DISPLAY_AREAS_h

const int DisplayArea_Header[] PROGMEM = {
  0, 0, 128, 17
};

const int DisplayArea_WifiFlag[] PROGMEM = {
  0, 0, 16, 16
};

const int DisplayArea_FhemFlag[] PROGMEM = {
  24, 0, 16, 16
};

const int DisplayArea_AddonFlag[] PROGMEM = {
  48, 0, 16, 16
};

const int DisplayArea_RSSI[] PROGMEM = {
  65, 0, 63, 15
};

const int DisplayArea_Pages[] PROGMEM = {
  0, 18, 128, 46
};

const int DisplayArea_Line1[] PROGMEM = {
  0, 18, 128, 16
};

const int DisplayArea_ShortLine1[] PROGMEM = {
  37, 18, 91, 16
};

const int DisplayArea_Line2[] PROGMEM = {
  0, 33, 128, 16
};

const int DisplayArea_ShortLine2[] PROGMEM = {
  37, 33, 91, 16
};

const int DisplayArea_Line3[] PROGMEM = {
  0, 48, 128, 16
};

const int DisplayArea_ShortLine3[] PROGMEM = {
  37, 48, 91, 16
};

const int DisplayArea_BigPageTextShort[] PROGMEM = {
  37, 30, 91, 22
};

const int DisplayArea_BigPageTextLong[] PROGMEM = {
  0, 30, 128, 22
};

const int DisplayArea_LED[] PROGMEM = {
  115, 1, 14, 14
};

const int DisplayArea_ProgressBar[] PROGMEM = {
  0, 34, 128, 14
};

const int DisplayArea_PageIcon[] PROGMEM = {
  0, 26, 32, 32
};

#endif