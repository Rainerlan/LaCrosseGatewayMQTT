#include "Nextion.h"

Nextion::Nextion() : m_wifiServer(0) {
  m_lastUpdate = 0;
  m_isConnected = false;
}

bool Nextion::Begin(ESP8266WebServer *webServer, ESP8266SoftSerial *softSerial, unsigned long baud, bool addUnits, bool pressureDecimals) {
  m_server = webServer;
  m_softSerial = softSerial;
  m_baud = baud;
  m_addUnits = addUnits;
  m_pressureDecimals = pressureDecimals;
  m_isConnected = false;

  m_server->on("/ota/nextion", HTTP_POST, [this]() {
    m_server->sendHeader("Connection", "close");
    m_server->sendHeader("Access-Control-Allow-Origin", "*");

    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      if (fileName == "/nextion.tft") {
        Log("File: " + fileName + " Size: " + String(dir.fileSize()));
        File f = dir.openFile("r");
        UploadeFile(&f);
        f.close();
        delay(2000);
        Connect();
      }
    }
    m_server->send(200, "text/plain", m_log);
  }, [this]() {
    HTTPUpload &upload = m_server->upload();
    if (upload.status == UPLOAD_FILE_START) {
      m_log = "";
      Log("Start receiving '", false);
      Log(upload.filename, false);
      Log("'");
      String path = "/" + upload.filename;
      SPIFFS.begin();
      m_uploadFile = SPIFFS.open(path, "w");
    }

    else if (upload.status == UPLOAD_FILE_WRITE) {
      if (m_uploadFile) {
        m_uploadFile.write(upload.buf, upload.currentSize);
      }
    }
    else if (upload.status == UPLOAD_FILE_END) {
      if (m_uploadFile) {
        m_uploadFile.close();
      }
    }
    yield();
  });

  Connect();

  SendCommand("get \"NEXTION\"");
  String response = Receive(100, "NEXTION");
  m_isConnected = response.indexOf("NEXTION") != -1;

  return m_isConnected;
}

void Nextion::Connect() {
  SendCommand("get 0");
  Receive(50);
  m_softSerial->SetBaudrate(9600);
  SendCommand("baud=9600");
  Receive(50);
  SendCommand("baud=" + String(m_baud));
  Receive(50);
  m_softSerial->SetBaudrate(m_baud);
  SendCommand("get 0");
  Receive(50);

}

void Nextion::Handle(WSBase::Frame frame, StateManager *stateManager, int32_t rssi, bool fhemIsConnected, bool bridge1Connected, bool bridge2Connected) {
  if(millis() < m_lastUpdate) {
    m_lastUpdate = millis();
  }
  
  if(millis() > m_lastUpdate + 2000) {
    stateManager->Update();

    String upTimeText = stateManager->GetUpTime();
    upTimeText.trim();
    upTimeText = upTimeText.substring(0, upTimeText.lastIndexOf(' '));
    upTimeText.replace(" ", "");
 
    if (frame.HasTemperature) {
      SendCommand("LGW#temp.txt=\"" + String(frame.Temperature, 1) + (m_addUnits ? " °C\"" : "\""));
    }
    if (frame.HasHumidity) {
      SendCommand("LGW#hum.txt=\"" + String(frame.Humidity) + (m_addUnits ? " %rH\"" : "\""));
    }
    if (frame.HasPressure) {
      SendCommand("LGW#pres.txt=\"" + String(frame.Pressure, m_pressureDecimals ? 1 : 0) + (m_addUnits ? " hPa\"" : "\""));
    }

    SendCommand("LGW#rssi.txt=\"" + String(rssi) + (m_addUnits ? " dBm\"" : "\""));
    SendCommand("LGW#ip.txt=\"" + WiFi.localIP().toString() + "\"");
    SendCommand("LGW#fpm.txt=\"" + String(stateManager->GetFramesPerMinute()) + "\"");
    SendCommand("LGW#heap.txt=\"" + String(ESP.getFreeHeap()) + "\"");
    SendCommand("LGW#up.txt=\"" + upTimeText + "\"");
    SendCommand("LGW#ver.txt=\"" + stateManager->GetVersion() + "\"");

    SendCommand("vis LGW#wifi," + String(WiFi.isConnected() ? "1" : "0"));
    SendCommand("vis LGW#fhem," + String(fhemIsConnected ? "1" : "0"));
    SendCommand("vis LGW#cpu1," + String(bridge1Connected ? "1" : "0"));
    SendCommand("vis LGW#cpu2," + String(bridge2Connected ? "1" : "0"));

    Receive(1);
    m_lastUpdate = millis();
  }
  
}

void Nextion::SendCommand(String command) {
  while (m_softSerial->Available()) {
    m_softSerial->Read();
  }

  m_softSerial->Print(command);
  m_softSerial->Write(0xFF);
  m_softSerial->Write(0xFF);
  m_softSerial->Write(0xFF);
}

String Nextion::Receive(unsigned int timeout, String waitFor) {
  String result;
  uint8_t c = 0;
  long start;
  start = millis();
  while (millis() - start <= timeout) {
    while (m_softSerial->Available()) {
      c = m_softSerial->Read();
      if (c == 0) {
        continue;
      }
      result += (char)c;
    }
    if (waitFor.length() > 0 && result.endsWith(waitFor)) {
      break;
    }
  }

  return result;
}

String Nextion::Receive(unsigned int timeout, byte waitFor) {
  return Receive(timeout, String((char)waitFor));
}

void Nextion::UploadeFile(File *file) {
  unsigned int baud = 57600;

  if (m_progressCallback) {
    m_progressCallback(1, 0, file->size(), "Flashing Nextion");
  }
  Log("Starting upload");

  SendCommand("");
  Receive(100);
  SendCommand("whmi-wri " + String(file->size()) + "," + String(baud) + ",0");
  m_softSerial->SetBaudrate(baud);
  delay(100);

  if (Receive(500, 0x05).indexOf(0x05) != -1) {
    Log("Nextion is now in upload Mode");

    unsigned long trigger = file->size() / 50;
    char *buffer = new char[512];
    unsigned long done = 0;
    unsigned long handledBytes = 0;
    while (file->available()) {
      size_t size = file->readBytes(buffer, 512);
      done += size;
      for (word i = 0; i < size; i++) {
        m_softSerial->Write(buffer[i]);
        handledBytes++;
      }

      if (done % 4096 == 0) {
        delay(100);
        Receive(500, 0x05);

        if (m_progressCallback) {
          m_progressCallback(2, handledBytes, 0, "");
          handledBytes = 0;
        }

      }

    }

    free(buffer);
    Receive(500, 0x05);

  }
  else {
    Log(":-) could not start upload mode");
  }

  Log("Upload finished");
  if (m_progressCallback) {
    m_progressCallback(3, 0, 0, "");
  }
}


void Nextion::Log(String text, bool newLine) {
  m_log += text + (newLine ? "\n" : "");
}

bool Nextion::IsConnected() {
  return m_isConnected;
}

void Nextion::SetProgressCallback(ProgressCallbackType * callback) {
  m_progressCallback = callback;
}

void Nextion::ShowProgress(unsigned int maxValue, String message) {
  m_maxProgress = maxValue;
  m_currentProgress = 0;
  
  SendCommand("page LGW#prog");
  SendCommand("vis LGW#pbar,1");
  SendCommand("vis LGW#ptext,1");
  SendCommand("LGW#pbar.val=0");
  SendCommand("LGW#ptext.txt=\"" + message + "\"");
  
  Receive(1);
}

void Nextion::MoveProgress(unsigned int offset) {
  if (offset == 0) {
    m_currentProgress++;
  }
  else {
    m_currentProgress += offset;
  }
  
  SendCommand("LGW#pbar.val=" + String(m_currentProgress * 100 / m_maxProgress));

  Receive(1);
}

void Nextion::HideProgress() {
  SendCommand("page LGW#main");
  
  Receive(1);
}

void Nextion::Print(String data) {
  SendCommand("page LGW#prog");
  SendCommand("vis LGW#pbar,0");
  SendCommand("vis LGW#ptext,0");
  SendCommand("LGW#info.txt=\"" + data + "\"");
  
  Receive(1);
}





