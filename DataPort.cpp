#include "DataPort.h"


DataPort::DataPort() : m_server(0) {
  m_logItemCallback = nullptr;
  m_ConnectCallback = nullptr;
}

void DataPort::Begin(uint port) {
  m_enabled = true;
  m_port = port;

  m_server = WiFiServer(port);
  m_server.begin();
  m_server.setNoDelay(true);
  m_isConnected = false;
}

void DataPort::SetLogItemCallback(LogItemCallbackType *callback) {
  m_logItemCallback = callback;
}

void DataPort::SetConnectCallback(ConnectCallbackType * callback){
  m_ConnectCallback = callback;
}

uint DataPort::GetPort() {
  return m_port;
}

bool DataPort::IsConnected() {
  return m_isConnected;
}

void DataPort::AddPayload(String payload) {
  if (m_enabled &&  m_queue.Count() <= 10) {
    m_queue.Push(payload);
  }
}

void DataPort::Send(String data) {
  if (m_enabled && m_client && !m_client.available()) {
    m_client.println(data);    
  }
}

bool DataPort::Handle(CommandCallbackType* callback) {
  bool result = false;
  
  if(m_enabled) {
    if (!m_client.connected()) {
      if (m_isConnected) {
        String logItem = "DataPort: #";
        logItem += m_port;
        logItem += " Client disconnected";
        m_logItemCallback (logItem);
        m_client.stop();
        m_isConnected = false;
        if (m_ConnectCallback) {
          m_ConnectCallback(false);
        }
      }
    }
    if (!m_client) {
      m_client = m_server.available();
      if (m_client) {
        m_client.setNoDelay(true);
        m_client.setTimeout(10);
        m_initialized = false;
        m_lastMillis = millis() + 1000;

        m_isConnected = true;
        if (m_logItemCallback) {
          String logItem = "DataPort: #";
          logItem += m_port;
          logItem += " Client connected, IP=";
          logItem += m_client.remoteIP().toString();
          logItem += " Port=";
          logItem += m_client.remotePort();
          m_logItemCallback(logItem);
          if (m_ConnectCallback) {
            m_ConnectCallback(true);
          }
        }

      }
    }
    else {
      if (m_client.available()) {
        String request = m_client.readString();
        if (callback) {
          if (m_logItemCallback != NULL) {
            String logItem = "DataPort: #";
            logItem += m_port;
            logItem += " received '";
            request.replace("\n", " ");
            logItem += request;
            logItem += "'";
            m_logItemCallback(logItem);
          }

          String result = callback(request);
          if (result.length() > 0) {
            Send(result);
          }
        }
        m_client.flush();
      }
      else {
        // After 50 days, millis() will overflow to zero 
        if (millis() < m_lastMillis) {
          m_lastMillis = 0;
        }
        if (millis() > m_lastMillis + 125) {
          if (!m_initialized ) {
            if (callback != NULL) {
              m_client.println("");
              delay(1000);
              callback("v");
            }
            m_initialized = true;
          }
          else {
            while (!m_queue.IsEmpty()) {
              String pl = m_queue.Pop();
              Send(pl);
              delay(10);
            }
            result = true;
          }

          m_lastMillis = millis();

        }

      }

    }
  }
  
  return result;
}

