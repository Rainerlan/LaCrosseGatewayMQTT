#include "OwnSensors.h"

OwnSensors::OwnSensors() {
  m_logItemCallback = nullptr;
  m_hasBMP180 = false;
  m_hasBMP280 = false;
  m_hasBME280 = false;
  m_hasBME680 = false;
  m_hasDHT22 = false;
  m_hasLM75 = false;
  m_hasSHT75 = false;
  m_hasBH1750 = false;
  m_lastMeasurement = 0;
  m_ID = 0;
}

void OwnSensors::SetLogItemCallback(OwnSensorsLogItemCallbackType callback) {
  m_logItemCallback = callback;
}

void OwnSensors::Log(String logItem) {
  if(m_logItemCallback != nullptr) {
    m_logItemCallback(logItem);
  }
}

bool OwnSensors::TryInitialize(bool tryDHT, bool trySHT75, bool sendDecimals) {
  m_sendDecimals = sendDecimals;

  SetAltitudeAboveSeaLevel(0);
  
  m_hasBME680 = m_bme680.TryInitialize(0x76);

  if (!m_hasBME680) {
    m_hasBME280 = m_bme280.TryInitialize(0x76);
    if (!m_hasBME280) {
      m_hasBMP280 = m_bmp280.TryInitialize(0x76);
    }
  }
  
  m_hasBMP180 = m_bmp.TryInitialize();

  if (tryDHT) {
    while (millis() < 2000) {
      delay(10);
    }
    m_hasDHT22 = m_dht.TryInitialize(0);
  }
  
  if (trySHT75 && !m_hasDHT22) {
    m_hasSHT75 = m_sht75.TryInitialize(D3, D4);
  }
  
  m_hasLM75 = m_lm75.TryInitialize(0x4F);

  m_hasBH1750 = m_bh1750.TryInitialize(0x23);

  return m_hasSHT75 || m_hasBMP180 || m_hasBMP280 || m_hasBME280 || m_hasBME680 || m_hasDHT22 || m_hasLM75 || m_hasBH1750;
}

bool OwnSensors::HasBMP180() {
  return m_hasBMP180;
}

bool OwnSensors::HasBMP280() {
  return m_hasBMP280;
}

bool OwnSensors::HasBME280() {
  return m_hasBME280;
}

bool OwnSensors::HasBME680() {
  return m_hasBME680;
}

bool OwnSensors::HasDHT22() {
  return m_hasDHT22;
}

bool OwnSensors::HasLM75() {
  return m_hasLM75;
}

bool OwnSensors::HasSHT75() {
  return m_hasSHT75;
}

bool OwnSensors::HasBH1750() {
  return m_hasBH1750;
}

void OwnSensors::SetAltitudeAboveSeaLevel(int altitude) {
  m_bmp.SetAltitudeAboveSeaLevel(altitude);
  m_bme280.SetAltitudeAboveSeaLevel(altitude);
  m_bmp280.SetAltitudeAboveSeaLevel(altitude);
  m_bme680.SetAltitude(altitude);
}

void OwnSensors::SetCorrections(String corrT, String corrH) {
  if (corrT.indexOf('%') != -1) {
    m_corrTM = CorrectionMode::percent;
    corrT.replace("%", "");
    m_corrTV = corrT.toFloat() / 100.0;
  }
  else {
    m_corrTM = CorrectionMode::offset;
    m_corrTV = corrT.toFloat();
  }

  if (corrH.indexOf('%') != -1) {
    m_corrHM = CorrectionMode::percent;
    corrH.replace("%", "");
    m_corrHV = corrH.toFloat() / 100.0;
  }
  else {
    m_corrHM = CorrectionMode::offset;
    m_corrHV = corrH.toFloat();
  }

}

void OwnSensors::SetID(byte id) {
  m_ID = id;
}

BME280* OwnSensors::GetBME280Instance(){
  return &m_bme280;
}

BMP280* OwnSensors::GetBMP280Instance(){
  return &m_bmp280;
}

BME680* OwnSensors::GetBME680Instance() {
  return &m_bme680;
}

BMP180* OwnSensors::GetBMP180Instance() {
  return &m_bmp;
}

DHTxx* OwnSensors::GetDHTxxInstance() {
  return &m_dht;
}

LM75 * OwnSensors::GetLM75Instance(){
  return &m_lm75;
}

SHT75 * OwnSensors::GetSHT75Instance() {
  return &m_sht75;
}

BH1750 * OwnSensors::GetBH1750Instance() {
  return &m_bh1750;
}

WSBase::Frame OwnSensors::GetDataFrame() {
  return m_data;
}

void OwnSensors::Handle() { 
  if(m_hasBME680) {
    m_bme680.Handle();
  }
}

void OwnSensors::Measure() {
  m_data.ID = m_ID;
  m_data.CRC = 0;
  m_data.LowBatteryFlag = false;
  m_data.NewBatteryFlag = false;
  m_data.ErrorFlag = false;
  m_data.IsValid = false;
  m_data.HasHumidity = false;
  m_data.HasPressure = false;
  m_data.HasRain = false;
  m_data.HasTemperature = false;
  m_data.HasWindDirection = false;
  m_data.HasWindGust = false;
  m_data.HasWindSpeed = false;
  m_data.HasGas = false;
  m_data.HasIlluminance = false;
  m_data.HasDebug = false;

  float bmeTemperature = 0.0, bmpTemperature = 0.0, dhtTemperature = 0.0, lm75Temperature = 0.0, sht75Temperature = 0.0;
  int bmeHumidity = 0, dhtHumidity = 0, sht75Hunidity = 0;
  float bmePressure = 0.0, bmpPressure = 0.0;
  uint32_t bmeGas = 0;

  if (m_hasSHT75) {
    sht75Temperature = round(m_sht75.GetTemperature() * 10.0) / 10.0;
    sht75Hunidity = round(m_sht75.GetHumidity());
  }
  if (m_hasBME280) {
    bmeTemperature = m_bme280.GetTemperature();
    bmePressure = m_bme280.GetPressure();
    bmeHumidity = m_bme280.GetHumidity();
  }
  if (m_hasBMP280) {
    bmpTemperature = m_bmp280.GetTemperature();
    bmpPressure = m_bmp280.GetPressure();
  }
  if (m_hasBME680) {
    bmeTemperature = m_bme680.GetTemperature();
    bmePressure = m_bme680.GetPressure();
    bmeHumidity = m_bme680.GetHumidity();
    bmeGas = m_bme680.GetGas();
    m_bme680.Handle();
  }
  if (m_hasBMP180) {
    bmpTemperature = m_bmp.GetTemperature();
    bmpPressure = m_bmp.GetPressure();
  }
  if (m_hasDHT22 && m_dht.TryMeasure()) {
    dhtTemperature = m_dht.GetTemperature();
    dhtHumidity = m_dht.GetHumidity();
  }
  if (m_hasLM75) {
    lm75Temperature = m_lm75.GetTemperature();
  }
  
  if (m_hasBME280) {
    m_data.HasPressure = true;
    m_data.HasTemperature = true;
    m_data.HasHumidity = true;
    m_data.Temperature = bmeTemperature;
    m_data.Pressure = bmePressure;
    m_data.Humidity = bmeHumidity;
    m_data.IsValid = true;
  }

  if (m_hasBME680) {
    m_data.HasPressure = true;
    m_data.HasTemperature = true;
    m_data.HasHumidity = true;
    m_data.HasGas = true;
    m_data.Temperature = bmeTemperature;
    m_data.Pressure = bmePressure;
    m_data.Humidity = bmeHumidity;
    m_data.Gas = bmeGas;
    m_data.IsValid = true;
  }

  if (!m_hasBME280 && !m_hasBME680 && (m_hasBMP180 || m_hasBMP280)) {
    m_data.HasPressure = true;
    m_data.HasTemperature = true;
    m_data.Temperature = bmpTemperature;
    m_data.Pressure = bmpPressure;
    m_data.IsValid = true;
  }

  if (!m_hasBME280 && !m_hasBME680 && m_hasDHT22) {
    m_data.HasTemperature = true;
    m_data.HasHumidity = true;
    m_data.Humidity = dhtHumidity;
    m_data.Temperature = dhtTemperature;
    m_data.IsValid = true;
  }

  if (!m_hasBME280 && !m_hasBME680 && !m_hasBMP180 && !m_hasDHT22 && m_hasLM75) {
    m_data.HasTemperature = true;
    m_data.Temperature = lm75Temperature;
    m_data.IsValid = true;
  }

  if (m_hasSHT75) {
    m_data.HasTemperature = true;
    m_data.HasHumidity = true;
    m_data.Temperature = sht75Temperature;
    m_data.Humidity = sht75Hunidity;
    m_data.IsValid = true;
  }

  if(m_hasBH1750) {
    m_data.HasIlluminance = true;
    m_data.Illuminance = m_bh1750.GetIlluminance();
    m_data.IsValid = true;
  }

  if (m_data.HasTemperature) {
    m_data.Temperature = m_corrTM == CorrectionMode::offset ? m_data.Temperature + m_corrTV : m_data.Temperature + m_data.Temperature * m_corrTV;
  }
  if (m_data.HasHumidity) {
    m_data.Humidity = m_corrHM == CorrectionMode::offset ? m_data.Humidity + m_corrHV : m_data.Humidity + m_data.Humidity * m_corrHV;
  }

}

String OwnSensors::GetFhemDataString(){
  String fhemString = "";
   
  if (m_data.IsValid) {
    fhemString = BuildFhemDataString(&m_data, 4);
  }

  return fhemString;
}



 
