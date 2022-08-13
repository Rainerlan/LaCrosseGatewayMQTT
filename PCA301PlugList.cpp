#include "PCA301PlugList.h"

void PCA301PlugList::Begin(word pollInterval) {
  m_pollInterval = pollInterval;
  if (m_plugs.Size() == 0) {
    String plugList = m_settingCallback("PCA301Plugs", "", false);

    String id;
    String channel;
    bool flag = false;
    plugList.trim();
    plugList += ",";
    if (plugList.length() > 2) {
      for (uint i = 0; i < plugList.length(); i++) {
        char c = plugList.charAt(i);
        if (c == ',') {
          PCA301Plug plug;
          plug.SetIdString(id);
          plug.Channel = (byte)channel.toInt();
          plug.LastPoll = 0;
          m_plugs.Put(id, plug);

          flag = false;
          id = "";
          channel = "";
        }
        else if (c == '=') {
          flag = true;
        }
        else {
          if (!flag) {
            id += c;
          }
          else {
            channel += c;
          }
        }
      }

    }
  
  }

}

void PCA301PlugList::HandleReceivedPlug(byte id[3], byte channel) {
  String idString = PCA301Plug::BuildIdString(id);
  bool writeSettings = false;

  if (!m_plugs.ContainsKey(idString)) {
    writeSettings = true;
    PCA301Plug plug;
    plug.SetIdString(idString);
    plug.Channel = channel;
    plug.LastPoll = 0;
    m_plugs.Put(idString, plug);
  }
  else {
    PCA301Plug *plug = m_plugs.GetPointer(idString);
    if (plug->Channel != channel) {
      plug->Channel = channel;
      writeSettings = true;
    }
  }

  
  if (writeSettings) {
    String data;
    for (byte b = 0; b < m_plugs.Size(); b++) {
      PCA301Plug plug = m_plugs.GetValueAt(b);
      data += plug.GetIdString();
      data += "=";
      data += plug.Channel;
      if (b < m_plugs.Size() - 1) {
        data += ",";
      }
    }
    
    m_settingCallback("PCA301Plugs", data, true);
  }

  if (m_plugs.ContainsKey(idString)) {
    PCA301Plug *plug = m_plugs.GetPointer(idString);
    plug->LastPoll = millis();
  }

}

void PCA301PlugList::Poll() {
  if (millis() < m_lastPoll) {
    m_lastPoll = 0;
  }
  if (millis() > m_lastPoll + 500) {
    for (uint i = 0; i < m_plugs.Size(); i++) {
      PCA301Plug *plug = m_plugs.GetValuePointerAt(i);
      if (millis() > plug->LastPoll + m_pollInterval * 1000 || millis() < plug->LastPoll) {
        byte payload[10];
        payload[0] = plug->Channel;
        payload[1] = 4;
        payload[2] = plug->ID[0];
        payload[3] = plug->ID[1];
        payload[4] = plug->ID[2];
        payload[5] = 0;
        payload[6] = 0xFF;
        payload[7] = 0xFF;
        payload[8] = 0xFF;
        payload[9] = 0xFF;

        if (m_sendPayloadCallback != 0) {
          String logItem = "Poll ";
          logItem += " [ID=";
          logItem += String(plug->ID[0], HEX);
          logItem += " ";
          logItem += String(plug->ID[1], HEX);
          logItem += " ";
          logItem += String(plug->ID[2], HEX);
          logItem += " Channel=";
          logItem += String(payload[0]);
          logItem += "]";
          Log(logItem);

          m_sendPayloadCallback(payload);
        }
        plug->LastPoll = millis();

        break;
      }
    }
    m_lastPoll = millis();
  }

}

byte PCA301PlugList::GetNextFreeChannel() {
  byte result = 0;
  for (int channel = 1; channel <= 128; channel++) {
    bool isFree = true;
    for (uint i = 0; i < m_plugs.Size(); i++) {
      PCA301Plug plug = m_plugs.GetValueAt(i);
      if (plug.Channel == channel) {
        isFree = false;
        break;
      }
    }
    if (isFree) {
      result = channel;
      break;
    }
  }

  return result;
}

PCA301PlugList::PCA301PlugList() {
  m_lastPoll = millis();
}

void PCA301PlugList::OnSendPayload(TCallbackFunction fp) {
  m_sendPayloadCallback = fp;
}

void PCA301PlugList::SetLogItemCallback(TLogItemCallback callback) {
  m_logItemCallback = callback;
}

void PCA301PlugList::SetSettingsCallback(TSettingsCallback callback) {
  m_settingCallback = callback;
}

bool PCA301PlugList::IsKnownPlug(byte id[3]) {
  return m_plugs.ContainsKey(PCA301Plug::BuildIdString(id));
}

void PCA301PlugList::Log(String logItem) {
  if (m_logItemCallback != NULL) {
    m_logItemCallback(logItem);
  }
}