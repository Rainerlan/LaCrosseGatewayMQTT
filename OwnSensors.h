#ifndef _OWNSENSORS_h
#define _OWNSENSORS_h

#include <functional>
#include "WSBase.h"
#include "BMP180.h"
#include "BMP280.h"
#include "BME280.h"
#include "BME680.h"
#include "DHTxx.h"
#include "LM75.h"
#include "SHT75.h"
#include "BH1750.h"

typedef std::function<void(String)> OwnSensorsLogItemCallbackType;

class OwnSensors : public WSBase {
public: 
  OwnSensors();
  void SetLogItemCallback(OwnSensorsLogItemCallbackType callback);
  bool TryInitialize(bool tryDHT, bool trySHT75, bool sendDecimals);
  bool HasBMP180();
  bool HasBMP280();
  bool HasBME280();
  bool HasBME680();
  bool HasDHT22();
  bool HasLM75();
  bool HasSHT75();
  bool HasBH1750();
  WSBase::Frame GetDataFrame();
  void Measure();
  void Handle();
  String GetFhemDataString();
  void SetAltitudeAboveSeaLevel(int altitude);
  void SetCorrections(String corrT, String corrH);
  void SetID(byte id);
  BME280 *GetBME280Instance();
  BMP280 *GetBMP280Instance();
  BME680 *GetBME680Instance();
  BMP180 *GetBMP180Instance();
  DHTxx *GetDHTxxInstance();
  LM75 *GetLM75Instance();
  SHT75 *GetSHT75Instance();
  BH1750 *GetBH1750Instance();

protected:
  OwnSensorsLogItemCallbackType m_logItemCallback;
  enum CorrectionMode {
    offset,
    percent
  };

  byte m_ID;
  WSBase::Frame m_data;
  bool m_hasBMP180;
  bool m_hasBMP280;
  bool m_hasBME280;
  bool m_hasBME680;
  bool m_hasDHT22;
  bool m_hasLM75;
  bool m_hasSHT75;
  bool m_hasBH1750;
  float m_corrTV;
  float m_corrHV;
  CorrectionMode m_corrTM;
  CorrectionMode m_corrHM;
  
  BMP180 m_bmp;
  BMP280 m_bmp280;
  BME280 m_bme280;
  BME680 m_bme680;
  DHTxx m_dht;
  LM75 m_lm75;
  SHT75 m_sht75;
  BH1750 m_bh1750;
  
  unsigned long m_lastMeasurement;

  void Log(String logItem);
};


#endif

