#include "HardwarePageBuilder.h"

String HardwarePageBuilder::BuildSerialBridgeLine(TcpServer* serialBridge, String name) {
  String result = name + "\t";
  if (serialBridge->GetPort() != 0) {
    result += String(serialBridge->GetPort()) + "\t" + String(serialBridge->GetClientsConnected()) + " client(s) connected\n";
  }
  else {
    result += "---\n";
  }
  return result;
}

String HardwarePageBuilder::BuildRfmLine(RFMxx *rfm, String name) {
  String result = "";
  result += name + "\t";
  
  if (rfm->IsConnected()) {
    result += rfm->GetRadioName() + "\t";
    result += (String)rfm->GetFrequency() + " kHz    ";
    if(rfm->GetRadioType() == RFMxx::RadioType::RFM95W) {
      result += "LoRa SF=";
      result += String(rfm->GetSpreadingFactor());
      result += " BW=";
      result += String(rfm->GetBandwidthHz() / 1000);
      result += " kHz";
    }
    else {
      if(rfm->ToggleInterval) {
        result += "toggle: ";
        result += String(rfm->ToggleInterval) + " seconds > ";
        if(bitRead(rfm->ToggleMode, 0)) {
          result += "17.241";
        }
        if(bitRead(rfm->ToggleMode, 1)) {
          result += " / 9.579";
        }
        if(bitRead(rfm->ToggleMode, 2)) {
          result += " / 8.842";
        }
        if(bitRead(rfm->ToggleMode, 3)) {
          result += " / 20.000 ";
        }
      }
      else {
        result += "data rate: " + (String)rfm->GetDataRate() + " kbps";
      }
    }
  }
  else {
    result += "---";
  }

  return result + "\n";
}

String HardwarePageBuilder::BuildDataPortLine(DataPort *dataPort, String name) {
  String  result = "";

  result += name + "\t" + (String)(dataPort->GetPort() != 0 ? String(dataPort->GetPort()) : "---") + (String)"\t" + (String)(dataPort->IsConnected() ? "FHEM connected" : "") + (String)"\n";

  return result;
}


String HardwarePageBuilder::Build(RFMxx *rfm1, RFMxx *rfm2, RFMxx *rfm3, RFMxx *rfm4, RFMxx *rfm5, OwnSensors *ownSensors, SC16IS750 *sc16is750_1, SC16IS750 *sc16is750_2, DigitalPorts *digitalPorts, Display *display, DataPort *dataPort1, DataPort *dataPort2, DataPort *dataPort3, TcpServer *serialBridge, TcpServer *serialBridge2, TcpServer *softSerialBridge, AnalogPort *analogPort, Nextion *nextion) {
  String result = "";
  result += BuildRfmLine(rfm1, "Radio #1");
  result += BuildRfmLine(rfm2, "Radio #2");
  result += BuildRfmLine(rfm3, "Radio #3");
  result += BuildRfmLine(rfm4, "Radio #4");
  result += BuildRfmLine(rfm5, "Radio #5");

  String cvSHT75 = "";
  if (ownSensors->HasSHT75()) {
    cvSHT75 = "";
    SHT75 *sht75 = ownSensors->GetSHT75Instance();
    SHT75Value sht75Values = sht75->GetLastMeasuredValue();
    cvSHT75 += "T=" + String(sht75Values.Temperature, 1);
    cvSHT75 += " H=" + String(sht75Values.Humidity);
  }
  result += (String)"SHT75\t" + (ownSensors->HasSHT75() ? "OK\t" : "---\t") + cvSHT75 + "\n";

  String cvbme680t = "";
  if (ownSensors->HasBME680()) {
    cvbme680t = "";
    BME680 *bme680 = ownSensors->GetBME680Instance();
    BME680Value bme680Values = bme680->GetLastMeasuredValue();
    cvbme680t += "T=" + String(bme680Values.Temperature, 1);
    cvbme680t += " H=" + String(bme680Values.Humidity, 0);
    cvbme680t += " P=" + String(bme680Values.Pressure, 1);
    cvbme680t += " G=" + String(bme680Values.Gas, 0);
  }
  result += (String)"BME680\t" + (ownSensors->HasBME680() ? "OK\t" : "---\t") + cvbme680t + "\n";

  String cv280t = "";
  if (ownSensors->HasBME280()) {
    cv280t = "";
    BME280 *bme280 = ownSensors->GetBME280Instance();
    bme280_compensation cv280 = bme280->GetCompensationValues();
    BME280Value lm280 = bme280->GetLastMeasuredValue();
    cv280t += "T=" + String(lm280.Temperature, 1) ;
    cv280t += " H=" + String(lm280.Humidity);
    cv280t += " P=" + String(lm280.Pressure, 1);
    cv280t += " Calibration: T1:" + String(cv280.T1) + " T2:" + String(cv280.T2) + " T3:" + String(cv280.T3) + " H1:" + String(cv280.H1) + " H2:" + String(cv280.H2) + " H3:" + String(cv280.H3) + " H4:" + String(cv280.H4) + " H5:" + String(cv280.H5) + " H6:" + String(cv280.H6) + " P1:" + String(cv280.P1) + " P2:" + String(cv280.P2) + " P3:" + String(cv280.P3) + " P4:" + String(cv280.P4) + " P5:" + String(cv280.P5) + " P6:" + String(cv280.P6) + " P7:" + String(cv280.P7) + " P8:" + String(cv280.P8) + " P9:" + String(cv280.P9);
    cv280t += " ADC:";
    cv280t += " T=" + String(lm280.ADCT);
    cv280t += " H=" + String(lm280.ADCH);
    cv280t += " P=" + String(lm280.ADCP);
  }
  result += (String)"BME280\t" + (ownSensors->HasBME280() ? "OK\t" : "---\t") + cv280t + "\n";

  String cvP280t = "";
  if (ownSensors->HasBMP280()) {
    cvP280t = "";
    BMP280 *bmp280 = ownSensors->GetBMP280Instance();
    bmp280_compensation cvP280 = bmp280->GetCompensationValues();
    BMP280Value lmP280 = bmp280->GetLastMeasuredValue();
    cvP280t += "T=" + String(lmP280.Temperature, 1) ;
    cvP280t += " P=" + String(lmP280.Pressure, 1);
    cvP280t += " Calibration: T1:" + String(cvP280.T1) + " T2:" + String(cvP280.T2) + " T3:" + String(cvP280.T3) + " P1:" + String(cvP280.P1) + " P2:" + String(cvP280.P2) + " P3:" + String(cvP280.P3) + " P4:" + String(cvP280.P4) + " P5:" + String(cvP280.P5) + " P6:" + String(cvP280.P6) + " P7:" + String(cvP280.P7) + " P8:" + String(cvP280.P8) + " P9:" + String(cvP280.P9);
    cvP280t += " ADC:";
    cvP280t += " T=" + String(lmP280.ADCT);
    cvP280t += " P=" + String(lmP280.ADCP);
  }
  result += (String)"BMP280\t" + (ownSensors->HasBMP280() ? "OK\t" : "---\t") + cvP280t + "\n";

  String cv180t = "";
  if (ownSensors->HasBMP180()) {
    cv180t = "";
    BMP180 *bmp180 = ownSensors->GetBMP180Instance();
    bmp180_compensation cv180 = bmp180->GetCompensationValues();
    BMP180Value lm180 = bmp180->GetLastMeasuredValue();
    cv180t += "T=" + String(lm180.Temperature, 1);
    cv180t += " P=" + String(lm180.Pressure, 1);
    cv180t += " Calibration: AC1:" + String(cv180.CAC1) + " AC2:" + String(cv180.CAC2) + " AC3:" + String(cv180.CAC3) + " AC4:" + String(cv180.CAC4) + " AC5:" + String(cv180.CAC5) + " AC6:" + String(cv180.CAC6) + " B1:" + String(cv180.CB1) + " B2:" + String(cv180.CB2) + " MB:" + String(cv180.CMB) + " MC:" + String(cv180.CMC) + " MD:" + String(cv180.CMD);
    cv180t += " ADC:";
    cv180t += " T=" + String(lm180.ADCT);
    cv180t += " P=" + String(lm180.ADCP);

  }
  result += (String)"BMP180\t" + (ownSensors->HasBMP180() ? "OK\t" : "---\t") + cv180t + "\n";

  result += "DHT22\t";
  if (ownSensors->HasDHT22()) {
    result += "OK\t";
    DHTxx *dhtXX = ownSensors->GetDHTxxInstance();
    DHTxxValue cvDHT = dhtXX->GetLastMeasuredValue();
    result += "T=" + String(cvDHT.Temperature, 1);
    result += " H=" + String(cvDHT.Humidity);
  }
  else {
    result += "---\t";
  }
  result += "\n";

  result += "LM75\t";
  if (ownSensors->HasLM75()) {
    result += "OK\t";
    result += "T=" + String(ownSensors->GetLM75Instance()->GetLastMeasuredValue().Temperature, 1);
  }
  else {
    result += "---\t";
  }
  result += "\n";

  result += "BH1750\t";
  if(ownSensors->HasBH1750()) {
    result += "OK\t";
    result += "LX=" + String(ownSensors->GetBH1750Instance()->GetLastMeasuredValue());
  }
  else {
    result += "---\t";
  }
  result += "\n";
  
  result += (String)"SC16IS750 (0x90)\t" + (sc16is750_1->IsConnected() ? "OK\t" : "---\t") + (sc16is750_1->IsConnected() && sc16is750_1->IsClone() ? "Clone" : "") + "\n";
  result += (String)"SC16IS750 (0x92)\t" + (sc16is750_2->IsConnected() ? "OK\t" : "---\t") + (sc16is750_2->IsConnected() && sc16is750_2->IsClone() ? "Clone" : "") + "\n";
  result += (String)"MCP23008\t" + (digitalPorts->IsConnected() ? "OK\t\n" : "---\t\n");

  result += (String)"OLED\t";
  if (display->IsConnected()) {
    result += "OK\t";
    result += display->IsOn() ? "On" : "Off";
  }
  else {
    result += "---\t";
  }
  result += "\n";

  result += BuildDataPortLine(dataPort1, "DataPort #1");
  result += BuildDataPortLine(dataPort2, "DataPort #2");
  result += BuildDataPortLine(dataPort3, "DataPort #3");
  
  result += BuildSerialBridgeLine(serialBridge, "Serial-bridge #1");
  result += BuildSerialBridgeLine(serialBridge2, "Serial-bridge #2");
  result += BuildSerialBridgeLine(softSerialBridge, "Soft-bridge");

  result += (String)"Nextion\t";
  if (nextion->IsConnected()) {
    result += "OK\t";
  }
  else {
    result += "---\t";
  }
  result += "\n";
  
  unsigned int uAnalog = (float)analogPort->GetLastValue() * (1000.0 / (float)analogPort->GetU1023()) / 1.023;
  result += "Analog port\t" + (String)(analogPort->IsEnabled() ? "Enabled\t" : "Disabled\t") + "ADC=" + (String)analogPort->GetLastValue() + " U=" + String(uAnalog) + " mV (0 ... " + String(analogPort->GetU1023()) + " mV)";


  return result;
}
