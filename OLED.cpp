#include "OLED.h"

OLED::OLED() {
  m_address = 0;
}


bool OLED::Begin(byte address, Orientations orientation, OLED::Controllers controller) {
  bool exists = false;
  m_address = address;
  m_controller = controller;
  m_orientation = orientation;

  Wire.beginTransmission(m_address);
  if (Wire.endTransmission() == 0) {
    exists = true;
  }

  if (exists) {
    Command(DISPLAYOFF);
    Command(NORMALDISPLAY);
    Command(SETDISPLAYCLOCKDIV);
    Command(0x80);
    Command(SETMULTIPLEX);
    Command(0x3F);
    Command(SETDISPLAYOFFSET);
    Command(0x00);
    Command(SETSTARTLINE | 0x00);
    Command(CHARGEPUMP);
    Command(0x14);
    Command(MEMORYMODE);
    Command(0x00);
    Command(SEGREMAP);
    Command(COMSCANINC);
    Command(SETCOMPINS);
    Command(0x12);
    Command(SETCONTRAST);
    Command(0xCF);
    Command(SETPRECHARGE);
    Command(0xF1);
    Command(SETVCOMDETECT);
    Command(0x40);
    Command(DISPLAYALLON_RESUME);
    Command(NORMALDISPLAY);
    Command(0x2e);
    Command(DISPLAYON);

    m_isOn = true;

    if (m_orientation == Orientations::UpsideDown) {
      Command(0xA0 | 0x1);
      Command(0xC8);
    }
  }

  return exists;
}

void OLED::On() {
  Command(0xaf);
  m_isOn = true;
}

void OLED::Off() {
  Command(0xae);
  m_isOn = false;
}

void OLED::SetContrast(byte contrast) {
  Command(0x81);
  Command(contrast);
}

void OLED::Clear() {
  for (unsigned int i = 0; i < (128 * 64 / 8); i++) {
    m_buffer[i] = 0;
  }
}

bool OLED::Command(byte command) {
  noInterrupts();
  Wire.beginTransmission(m_address);
  Wire.write(0x80);
  Wire.write(command);
  byte res = Wire.endTransmission();
  interrupts();
  if (res != 0) {
    Serial.println("ERROR (Command): " + String(res));
  }
  return res == 0;
}

void OLED::Refresh() {
  int pos = 0;
  for (byte i = 0; i < 8; i++) {
    Command(0xB0 + i);

    if (m_controller == OLED::Controllers::SSD1306) {
      Command(0);
    }
    else {
      Command(2 & 0xF);
    }
    Command(0x10);

    for (byte j = 0; j < 8; j++) {
      Wire.beginTransmission(m_address);
      Wire.write(0x40);
      for (byte k = 0; k < 16; k++) {
        Wire.write(m_buffer[pos++]);
      }
      Wire.endTransmission();
    }
  }
  
}

void OLED::SetPixel(int x, int y, Colors color) {
  if (x >= 0 && x < 128 && y >= 0 && y < 64) {
    switch (color) {
    case Colors::White:  m_buffer[x + (y / 8) * 128] |= (1 << (y & 7)); break;
    case Colors::Black:  m_buffer[x + (y / 8) * 128] &= ~(1 << (y & 7)); break;
    case Colors::Invert: m_buffer[x + (y / 8) * 128] ^= (1 << (y & 7)); break;
    }
  }
}

void OLED::Print(const int area[4], String text, Alignments alignment) {
  int x = area[0];

  switch (alignment) {
  case OLED::Center:
    x += area[2] / 2;
    break;

  case OLED::Right:
    x += area[2];
    break;

  default:
    break;
  }

  Print(x, area[1], text, alignment);
}

void OLED::Print(int x, int y, String text, Alignments alignment) {
  byte currentByte;
  int charX, charY;
  int currentBitCount;
  int charCode;
  int currentCharWidth;
  int currentCharStartPos;
  int cursorX = 0;
  int numberOfChars = pgm_read_byte(m_font + 3);
  int charHeight = pgm_read_byte(m_font + 1);
  int currentCharByteNum = 0;
  int startX = 0;
  int startY = y;

  if (alignment == Alignments::Left) {
    startX = x;
  }
  else if (alignment == Alignments::Center) {
    int width = GetStringWidth(text);
    startX = x - width / 2;
  }
  else if (alignment == Alignments::Right) {
    int width = GetStringWidth(text);
    startX = x - width;
  }

  for (uint j = 0; j < text.length(); j++) {
    charCode = text.charAt(j) - 0x20;
    currentCharWidth = pgm_read_byte(m_font + 4 + charCode);
    currentCharStartPos = numberOfChars + 4;

    for (int m = 0; m < charCode; m++) {
      currentCharStartPos += pgm_read_byte(m_font + 4 + m)  * charHeight / 8 + 1;
    }

    currentCharByteNum = ((charHeight * currentCharWidth) / 8) + 1;
    for (int i = 0; i < currentCharByteNum; i++) {
      currentByte = pgm_read_byte(m_font + currentCharStartPos + i);
      for (int bit = 0; bit < 8; bit++) {
        currentBitCount = i * 8 + bit;
        charX = currentBitCount % currentCharWidth;
        charY = currentBitCount / currentCharWidth;

        if (bitRead(currentByte, bit)) {
          SetPixel(startX + cursorX + charX, startY + charY, Colors::White);
        }

      }
    }
    cursorX += currentCharWidth;

  }
}

int OLED::GetStringWidth(String text) {
  int stringWidth = 0;
  char charCode;
  for (uint i=0; i < text.length(); i++) {
    charCode = text.charAt(i)-0x20;
    stringWidth += pgm_read_byte(m_font + 4 + charCode);
  }
  return stringWidth;
}

void OLED::PushContent() {
  for (int i = 0; i < OLED_BUFFER_SIZE; i++) {
    m_stack[i] = m_buffer[i];
  }
}

void OLED::PopContent() {
  for (int i = 0; i < OLED_BUFFER_SIZE; i++) {
    m_buffer[i] = m_stack[i];
  }
  Refresh();
}

bool OLED::IsOn() {
  return m_isOn;
}

void OLED::SetFont(const char *fontData) {
  m_font = fontData;
}



void OLED::DrawBitmap(int x, int y, int width, int height, const char *bitmap) {
  for (int i = 0; i < width * height / 8; i++ ){
    unsigned char charColumn = 255 - pgm_read_byte(bitmap + i);
    for (int j = 0; j < 8; j++) {
      int targetX = i % width + x;
      int targetY = (i / (width)) * 8 + j + y;
      if (bitRead(charColumn, j)) {
        SetPixel(targetX, targetY, Colors::White);
      }
    }
  }
}

void OLED::DrawRect(const int area[4], bool fill, Colors color) {
  DrawRect(area[0], area[1], area[2], area[3], fill, color);
}

void OLED::DrawRect(int x, int y, int width, int height, bool fill, Colors color) {
  if (color == OLED::Colors::White) {
    width -= 1;
    height -= 1;
  }
  if (fill) {
    for (int i = x; i < x + width; i++) {
      for (int j = y; j < y + height; j++) {
        SetPixel(i, j, color);
      }
    }
  }
  else {
    for (int i = x; i < x + width; i++) {
      SetPixel(i, y, color);
      SetPixel(i, y + height, color);
    }
    for (int i = y; i < y + height; i++) {
      SetPixel(x, i, color);
      SetPixel(x + width, i, color);
    }
  }
}

void OLED::DrawXBM(const int area[4], const char *xbm) {
  DrawXBM(area[0], area[1], area[2], area[3], xbm);
}

void OLED::DrawXBM(int x, int y, int width, int height, const char *xbm) {
  if (width % 8 != 0) {
    width =  ((width / 8) + 1) * 8;
  }
  for (int i = 0; i < width * height / 8; i++ ){
    byte charColumn = pgm_read_byte(xbm + i);
    for (int j = 0; j < 8; j++) {
      int targetX = (i * 8 + j) % width + x;
      int targetY = (8 * i / (width)) + y;
      if (bitRead(charColumn, j)) {
        SetPixel(targetX, targetY, Colors::White);
      }
    }
  }
}

