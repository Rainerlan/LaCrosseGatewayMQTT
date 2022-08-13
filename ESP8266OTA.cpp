#include "ESP8266OTA.h"


ESP8266OTA::ESP8266OTA() {
  m_log = "";
}


void ESP8266OTA::Log(String text, bool newLine) {
  m_log += text + (newLine ? "\n" : "");
  Serial.print(text + (newLine ? "\r\n" : ""));
}

static ESP8266WebServer *webServer;
void ESP8266OTA::Begin(ESP8266WebServer *server) {
  m_server = server;
  m_log = "";
  webServer = server;
  // Upload 
  m_server->on("/ota/firmware.bin", HTTP_POST, [this]() {
    m_server->sendHeader("Connection", "close");
    m_server->sendHeader("Access-Control-Allow-Origin", "*");
    Log("");
    Log(Update.hasError() ? "ERROR: OTA update failed" : "OTA update finished");
    m_server->send(200, "text/plain", m_log);
    delay(1000);
    ESP.restart();
  }, [this]() {
    HTTPUpload &upload = m_server->upload();
    if (upload.status == UPLOAD_FILE_START){
      m_log = "";
      Log("Start receiving '", false);
      Log(upload.filename, false);
      Log("'");
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)){
        Update.printError(Serial);
        Log("ERROR: UPLOAD_FILE_START");
      }
    }
    else if (upload.status == UPLOAD_FILE_WRITE){
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize){
        Update.printError(Serial);
        Log("ERROR: UPLOAD_FILE_WRITE");
      }
    }
    else if (upload.status == UPLOAD_FILE_END){
      if (Update.end(true)) { 
        Log("Firmware size: ", false);
        Log(String(upload.totalSize));
        Log("Rebooting ESP8266 ...");
      }
      else {
        Update.printError(Serial);
        Log("ERROR: UPLOAD_FILE_END");
      }
    }
    yield();
  });

  // Pull
  static uint lastOtaProgress;
  ArduinoOTA.onStart([]() {
    lastOtaProgress = 0;
    Serial.println("Starting OTA update");
    webServer->stop();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println();
    Serial.println("OTA update finished");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (progress / (total / 100) == lastOtaProgress + 2) {
      Serial.print(".");
      lastOtaProgress += 2;
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.setPort(8266);
  ArduinoOTA.begin();
}

void ESP8266OTA::Handle() {
  ArduinoOTA.handle();
}

