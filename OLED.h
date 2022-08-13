#ifndef _OLED_h
#define _OLED_h

#include <Arduino.h>
#include <Wire.h>
#include "OLEDFonts.h"

#define OLED_BUFFER_SIZE 128 * 64 / 8

class OLED {

public:
  enum Controllers {
    SSD1306,
    SH1106
  };
  enum Orientations {
    Normal,
    UpsideDown
  };
  enum Alignments {
    Left,
    Center,
    Right
  };

  enum Colors {
    White,
    Black,
    Invert
  };

private:
   Controllers m_controller;
   int m_address;
   byte m_buffer[OLED_BUFFER_SIZE];
   byte m_stack[OLED_BUFFER_SIZE];
   const char *m_font = Roboto_Light_11;
   bool m_isOn;
   Orientations m_orientation;

public:
  OLED();
  bool Begin(byte address, Orientations orientation, OLED::Controllers controller = OLED::Controllers::SSD1306);
  void On();
  void Off();
  void Clear();
  void Refresh();
  void SetContrast(byte contrast);
  bool Command(byte command);
  void SetFont(const char *fontData);
  void Print(const int area[4], String text, Alignments alignment = Alignments::Left);
  void Print(int x, int y, String text, Alignments alignment = Alignments::Left);
  void DrawXBM(const int area[4], const char *xbm);
  void DrawXBM(int x, int y, int width, int height, const char *xbm);
  void SetPixel(int x, int y, Colors color);
  void DrawRect(const int area[4], bool fill = false, Colors color = Colors::White);
  void DrawRect(int x, int y, int width, int height, bool fill = false, Colors color = Colors::White);
  void DrawBitmap(int x, int y, int width, int height, const char *bitmap);
  int GetStringWidth(String text);
  void PushContent();
  void PopContent();
  bool IsOn();

};

#define CHARGEPUMP 0x8D
#define COLUMNADDR 0x21
#define COMSCANDEC 0xC8
#define COMSCANINC 0xC0
#define DISPLAYALLON 0xA5
#define DISPLAYALLON_RESUME 0xA4
#define DISPLAYOFF 0xAE
#define DISPLAYON 0xAF
#define EXTERNALVCC 0x1
#define INVERTDISPLAY 0xA7
#define MEMORYMODE 0x20
#define NORMALDISPLAY 0xA6
#define PAGEADDR 0x22
#define SEGREMAP 0xA0
#define SETCOMPINS 0xDA
#define SETCONTRAST 0x81
#define SETDISPLAYCLOCKDIV 0xD5
#define SETDISPLAYOFFSET 0xD3
#define SETHIGHCOLUMN 0x10
#define SETLOWCOLUMN 0x00
#define SETMULTIPLEX 0xA8
#define SETPRECHARGE 0xD9
#define SETSEGMENTREMAP 0xA1
#define SETSTARTLINE 0x40
#define SETVCOMDETECT 0xDB
#define SWITCHCAPVCC 0x2

#endif