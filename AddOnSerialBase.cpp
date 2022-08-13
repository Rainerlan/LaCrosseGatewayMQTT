#include "AddOnSerialBase.h"

AddOnSerialBase::AddOnSerialBase(SC16IS750 *expander, byte dtrPort, String hexFilename) {
  m_logItemCallback = nullptr;
  m_expander = expander;
  m_dtrPort = dtrPort;
  m_doLogging = false;
  m_isFlashing = false;
  m_progressCallback = nullptr;
  m_hexFilename = hexFilename;
}

void AddOnSerialBase::SetLogItemCallback(LogItemCallbackType callback) {
  m_logItemCallback = callback;
}

void AddOnSerialBase::Begin() {
  m_expander->PinMode(m_dtrPort, OUTPUT);
  m_expander->DigitalWrite(m_dtrPort, HIGH);

  m_server->on(("/ota/" + m_hexFilename).c_str(), HTTP_POST, [this]() {
    m_server->sendHeader("Connection", "close");
    m_server->sendHeader("Access-Control-Allow-Origin", "*");

    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      if (fileName == "/" + m_hexFilename) {
        Log("File: " + fileName + " Size: " + String(dir.fileSize()));
        File f = dir.openFile("r");
        FlashFile(&f);
        f.close();
      }
    }

    m_server->send(200, "text/plain", m_log);
    m_doLogging = false;
  }, [this]() {
    HTTPUpload &upload = m_server->upload();
    if (upload.status == UPLOAD_FILE_START) {
      m_doLogging = true;
      m_log = "";
      Log("Start receiving '", false);
      Log(upload.filename, false);
      Log("'");
      ////Serial.println("==== STOP ====");
      ////m_server->stop();
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
      ////m_server->begin();
      ////Serial.println("==== BEGIN ====");
    }
    yield();
  });


}

void AddOnSerialBase::SetBaudrate(unsigned long baudrate) {
  m_expander->SetBaudrate(baudrate);
}

void AddOnSerialBase::Log(String text, bool newLine) {
  if (m_doLogging) {
    m_log += text + (newLine ? "\n" : "");
    if (m_logItemCallback) {
      m_logItemCallback(text, newLine);
    }
  }
}

void AddOnSerialBase::Reset() {
  m_expander->DigitalWrite(m_dtrPort, LOW);
  delay(50);
  m_expander->DigitalWrite(m_dtrPort, HIGH);
}

// --- OTA UPDATE ----------------------------------------------------------------
void AddOnSerialBase::BeginFlash() {
  m_originalBaudRate = m_expander->GetBaudrate();
  m_expander->SetBaudrate(57600);

  while (m_expander->Available()) {
    m_expander->Read();
  }

  // Send sync
  Log("Sending sync");
  SendBytes(2, 0x30, 0x20);
  delay(250);
  Receive();

  // Enter program mode 
  Log("Enter program mode");
  SendBytes(2, 'P', 0x20);
  delay(50);
  Receive();
}

void AddOnSerialBase::Receive() {
  byte count = m_expander->Available();
  ////Log("<- ", false);
  for (int i = 0; i < count; i++) {
    m_expander->Read();
    ////byte bt = m_expander->Read();
    ////Log("[ " + String(bt, 16) + "] ", false);
  }
  ////Log("");
}

void AddOnSerialBase::SendBytes(byte count, ...) {
  byte data[count];
  va_list parameters;
  va_start(parameters, count);
  for (int i = 0; i < count; i++) {
    byte bt = (byte)va_arg(parameters, int);
    data[i] = bt;
  }
  SendBytes(count, data);
  va_end(parameters);
}

void AddOnSerialBase::SendBytes(byte count, byte *data) {
  ////Log("-> ", false);
  for (int i = 0; i < count; i++) {
    ////byte bt = data[i];
    m_expander->Write(data[i]);
    ////String ch = " ";
    ////if (data[i] >= 0x30 && data[i] <= 0x7A) {
    ////ch = String((char)bt);
    ////}
    ////Log("[" + ch + " " + String(bt, 16) + "] ", false);
  }
  ////Log("");
}

void AddOnSerialBase::FlashFile(File *file) {
  if (m_progressCallback) {
    m_progressCallback(1, 0, file->size(), "Flashing Addon");
  }
  Log("Starting flash");
  m_isFlashing = true;
  Reset();
  delay(200);
  BeginFlash();
  SendFile(file);
  FinishFlash();
  m_isFlashing = false;
  Log("Flash finished");
  if (m_progressCallback) {
    m_progressCallback(3, 0, 0, "");
  }
}

void AddOnSerialBase::FinishFlash() {
  Log("Leave Program Mode");
  SendBytes(2, 'Q', 0x20);
  delay(50);
  Receive();

  m_expander->SetBaudrate(m_originalBaudRate);
}

void AddOnSerialBase::SendFile(File *file) {
  String line;
  bool goOn = true;

  byte packet[128];
  byte packetSize = 0;
  m_currentPage = 0;
  word totalSize = 0;

  unsigned long trigger = file->size() / 20;
  unsigned long handledBytes = 0;
  while (goOn) {
    line = file->readStringUntil(10);
    if (line.length() > 0) {
      if (m_progressCallback) {
        handledBytes += line.length();
        if (handledBytes >= trigger) {
          m_progressCallback(2, handledBytes, 0, "");
          handledBytes = 0;
        }
      }

      if (line.length() > 7 && line.substring(7, 9) == "00") {
        byte len = (byte)strtol(line.substring(1, 3).c_str(), NULL, HEX);
        String payloadString = line.substring(9, 9 + (len + 1) * 2);

        for (byte i = 0; i < len * 2; i += 2) {
          String oneByteString = payloadString.substring(i, i + 2);
          byte oneByte = (byte)strtol(oneByteString.c_str(), NULL, HEX);

          packet[packetSize] = oneByte;
          packetSize++;
          totalSize++;
          if (packetSize == 128) {
            SendPage(packet, packetSize);
            packetSize = 0;
          }
        }
      }
    }
    else {
      if (packetSize > 0) {
        SendPage(packet, packetSize);
      }
      goOn = false;
    }
  }
  Log("Binary size is:", false);
  Log(String(totalSize));
}

void AddOnSerialBase::SendPage(byte packet[128], byte size) {
  word dummy = m_currentPage * 64;
  SendBytes(4, 'U', (byte)(dummy & 0xFF), (byte)(dummy >> 8), 0x20);
  delay(10);
  Receive();

  byte data[256];
  byte pos = 0;
  data[pos++] = 'd';
  data[pos++] = 0;
  data[pos++] = size;
  data[pos++] = 'F';

  for (int i = 0; i < size; i++) {
    data[pos++] = packet[i];
  }

  data[pos++] = 0x20;
  SendBytes(pos, data);
  delay(10);
  Receive();

  m_currentPage++;

}

void AddOnSerialBase::SetProgressCallback(ProgressCallbackType * callback) {
  m_progressCallback = callback;
}
