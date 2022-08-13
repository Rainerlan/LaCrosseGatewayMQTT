#ifndef __SHT75_H__
#define __SHT75_H__

#include <Arduino.h>

typedef struct {
  float CC1;
  float CC2;
  float CC3;
  float CD1;
  float CD2;
  float CT1;
  float CT2;
} sht75Compensation;

typedef struct {
  float Temperature;
  int Humidity;
} SHT75Value;

class SHT75 {
  public:
    SHT75();
    bool TryInitialize(byte dataPin, byte clockPin);
    bool IsInitialized();
    float GetHumidity();
    float GetTemperature();
    SHT75Value GetLastMeasuredValue();
    
  private:
    byte m_dataPin;
    byte m_clockPin;
    bool m_isInitialized;
    SHT75Value m_lastValue;
    sht75Compensation m_compensation;
    uint8_t ShiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
    void ShiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
    bool Command(int command);
    bool WaitForAck();
    word ReadWord(); 
    void SuppressCRC();
};

#endif
