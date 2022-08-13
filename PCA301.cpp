#include "PCA301.h"

/*
868.950 MHz / 6.631 kbps

   0  1  2  3  4  5  6  7  8  9 10 11
  ----------------------------------- 
  01 05 07 F8 92 01 AA AA AA AA 77 4A
  |  |  -------- |  ----- ----- -----  
  |  |     |     |    |     |     |------ CRC16 (Polynom 8005h)
  |  |     |     |    |     |  
  |  |     |     |    |     |------------ Accumulated consumption kWh 1/100
  |  |     |     |    |    
  |  |     |     |    |------------------ current consumption W 1/100
  |  |     |     |    
  |  |     |     |----------------------- data: 0/1 for command 5 or 1 with command 4
  |  |     |
  |  |     |----------------------------- address
  |  |
  |  |----------------------------------- command: 0x04=measure data, 0x05=switch device, 0x06=device LED, 0x11=pairing
  |
  |-------------------------------------- channel

Base -> Plug: off
Send:    01 05 03 A0 94 00 FF FF FF FF
Receive: 01 05 03 A0 94 00 AA AA AA AA  

Base -> Plug: on
Send:    01 05 03 A0 94 01 FF FF FF FF
Receive: 01 05 03 A0 94 01 AA AA AA AA

Plug local: off
Receive: 01 05 03 A0 94 01 AA AA AA AA

Plug local: on
Receive: 01 05 03 A0 94 00 AA AA AA AA

Plug in pairing mode:
Receive: 00 11 03 A0 94 AA AA AA AA AA
Send:    02 11 03 A0 94 00 FF FF FF FF
Receive: 02 04 03 A0 94 00 00 00 00 00

Base -> Plug: identify
Send:    01 06 03 A0 94 01 FF FF FF FF
Receive: ---

Base -> Plug: poll
Send:    01 04 03 A0 94 00 FF FF FF FF
Receive: 01 04 03 A0 94 01 01 F2 00 00 -> if on:  49.8W / 0.0 KWh
Receive: 01 04 03 A0 94 00 00 00 00 00 -> if off: Byte 5==0

Base -> Plug: reset
Send:    01 04 03 A0 94 01 FF FF FF FF
Receive: 01 04 03 A0 94 01 01 F2 00 00

 */


void PCA301::SendPayload(byte bytes[10], bool isRetry) {
  if (IsInitialized()) {
    byte payload[15];
    for (byte b = 0; b < 10; b++) {
      payload[b] = bytes[b];
      m_lastPayload[b] = bytes[b];
    }

    byte crc[2];
    CalculateCRC(bytes, crc);
    payload[10] = crc[0];
    payload[11] = crc[1];

    payload[12] = 0xAA;
    payload[13] = 0xAA;
    payload[14] = 0xAA;

    m_rfm->EnableReceiver(false);
    m_rfm->SendArray(payload, 15);
    m_rfm->EnableReceiver(true);

    String logItem = "TX: ";
    for (int i = 0; i < 15; i++) {
      String bt = String(payload[i], HEX);
      bt.toUpperCase();
      logItem += bt;
      logItem += " ";
    }
    logItem += GetPlugIdentifier(bytes[2], bytes[3], bytes[4], bytes[0]);
    Log(logItem);

    if (isRetry) {
      m_retries++;
    }
    else {
      m_retries = 0;
    }

    m_expectedAnswer = ExpectedAnswers::None;
    m_lastCommand = millis();
    if (m_retries <= 2) {
      if (bytes[1] == 0x05 && bytes[5] == 0x00) {
        m_expectedAnswer = ExpectedAnswers::Off;
      }
      if (bytes[1] == 0x05 && bytes[5] == 0x01) {
        m_expectedAnswer = ExpectedAnswers::On;
      }
      if (bytes[1] == 0x04) {
        m_expectedAnswer = ExpectedAnswers::Values;
      }
      Log("Expect: " + String((uint)m_expectedAnswer) + GetPlugIdentifier(bytes[2], bytes[3], bytes[4], bytes[0]));
    }

  }
  
}


void PCA301::Handle() {
  if(IsInitialized()) {
    if (millis() > m_lastCommand + 250 && m_expectedAnswer != ExpectedAnswers::None) {
      Log("Handle Resend: " + String((uint)m_expectedAnswer) + GetPlugIdentifier(m_nextAction.ID[0], m_nextAction.ID[1], m_nextAction.ID[2], m_nextAction.Channel));
      m_lastCommand = millis();
      SendPayload(m_lastPayload, true);
    }
    else if (m_nextAction.Action == ActionTypes::Poll && millis() >= m_nextAction.StartTime) {
      Log("Handle Action POLL" + GetPlugIdentifier(m_nextAction.ID[0], m_nextAction.ID[1], m_nextAction.ID[2], m_nextAction.Channel));
      m_nextAction.Action = ActionTypes::None;
      byte payload[10];
      payload[0] = m_nextAction.Channel;
      payload[1] = 0x04;
      payload[2] = m_nextAction.ID[0];
      payload[3] = m_nextAction.ID[1];
      payload[4] = m_nextAction.ID[2];
      payload[5] = 0;
      payload[6] = 0xFF;
      payload[7] = 0xFF;
      payload[8] = 0xFF;
      payload[9] = 0xFF;
      SendPayload(payload, false);
      
      
    }
    else if (m_nextAction.Action == ActionTypes::Pair && millis() >= m_nextAction.StartTime) {
      Log("Handle Action PAIR" + GetPlugIdentifier(m_nextAction.ID[0], m_nextAction.ID[1], m_nextAction.ID[2], m_nextAction.Channel));
     
      byte newChannel = m_plugList.GetNextFreeChannel();

      byte payload[10];
      payload[0] = newChannel;
      payload[1] = 0x11;
      payload[2] = m_nextAction.ID[0];
      payload[3] = m_nextAction.ID[1];
      payload[4] = m_nextAction.ID[2];
      payload[5] = 0;
      payload[6] = 0xFF;
      payload[7] = 0xFF;
      payload[8] = 0xFF;
      payload[9] = 0xFF;
      SendPayload(payload, false);

      m_nextAction.Action = ActionTypes::Poll;
      m_nextAction.Channel = newChannel;
      m_nextAction.StartTime = millis() +100;

    }
    else {
      m_plugList.Poll();
    }
  }
}


void PCA301::DecodeFrame(byte *bytes, struct PCA301::Frame *frame) {
  frame->IsValid = true;
  frame->IsFromPlug = true;
  frame->Power = 0.0;
  frame->AccumulatedPower = 0.0;

  byte crc[2];
  CalculateCRC(bytes, crc);
  if (bytes[10] != crc[0] || bytes[11] != crc[1]) {
    // wrong crc
    frame->IsValid = false;
  }

  if (bytes[1] == 6 || (bytes[6] == 0xFF && bytes[7] == 0xFF && bytes[8] == 0xFF && bytes[9] == 0xFF)){
    frame->IsFromPlug = false;
  }
  if (bytes[1] == 4 && bytes[6] == 0xAA && bytes[7] == 0xAA && bytes[8] == 0xAA && bytes[9] == 0xAA){
    frame->IsFromPlug = false;
  }

  frame->Channel = bytes[0];
  frame->Command = bytes[1];
  frame->ID[0] = bytes[2];
  frame->ID[1] = bytes[3];
  frame->ID[2] = bytes[4];
  frame->Data = bytes[5];
  frame->CRC1 = bytes[10];
  frame->CRC2 = bytes[11];
  frame->IsOn = frame->Data == 1;
  frame->IsPairingCommand = frame->Command == 0x11;
  frame->IsOnOffCommand = frame->Command == 0x05;

  if (frame->IsValid) {
    String logItem = "RX: ";
    for (int i = 0; i < PCA301::FRAME_LENGTH; i++) {
      String bt = String(bytes[i], HEX);
      bt.toUpperCase();
      logItem += bt;
      logItem += " ";
    }
    logItem += "  ";
    logItem += frame->IsFromPlug ? "(From Plug)" : "(From other sender)";
    logItem += GetPlugIdentifier(frame->ID[0], frame->ID[1], frame->ID[2], frame->Channel);
    Log(logItem);
  }


  if (frame->IsValid && frame->IsFromPlug) {

    if (frame->Command == 0x04) {
      frame->Power = (bytes[6] * 256 + bytes[7]) / 10.0;
      frame->AccumulatedPower = (bytes[8] * 256 + bytes[9]) / 100.0;
    }

    
    if (frame->Command == 0x05) {
      Log("Set Action POLL" + GetPlugIdentifier(frame->ID[0], frame->ID[1], frame->ID[2], frame->Channel));
      m_nextAction.Action = ActionTypes::Poll;
      m_nextAction.Channel = frame->Channel;
      m_nextAction.ID[0] = frame->ID[0];
      m_nextAction.ID[1] = frame->ID[1];
      m_nextAction.ID[2] = frame->ID[2];
      m_nextAction.StartTime = millis();
      
    }

    if (frame->Command == 0x11) {
      Log("Set Action PAIR" + GetPlugIdentifier(frame->ID[0], frame->ID[1], frame->ID[2], frame->Channel));
      m_nextAction.Action = ActionTypes::Pair;
      m_nextAction.Channel = frame->Channel;
      m_nextAction.ID[0] = frame->ID[0];
      m_nextAction.ID[1] = frame->ID[1];
      m_nextAction.ID[2] = frame->ID[2];
      m_nextAction.StartTime = millis() + 70;
    }


    if (m_expectedAnswer == ExpectedAnswers::Values && frame->Command == 0x04) {
      m_expectedAnswer = ExpectedAnswers::None;
    }
    if (m_expectedAnswer == ExpectedAnswers::On && frame->IsOn) {
      m_expectedAnswer = ExpectedAnswers::None;
    }
    if (m_expectedAnswer == ExpectedAnswers::Off && !frame->IsOn) {
      m_expectedAnswer = ExpectedAnswers::None;
    }

    if (frame->Command == 0x04) {
      m_plugList.HandleReceivedPlug(frame->ID, frame->Channel);

    }

  }
  
}


String PCA301::GetFhemDataString(byte *data) {
  String result = "";

  struct PCA301::Frame frame;
  DecodeFrame(data, &frame);
  if (frame.IsValid && frame.IsFromPlug && !frame.IsPairingCommand  && !frame.IsOnOffCommand) {
    if (m_plugList.IsKnownPlug(frame.ID)) {
      word p = (word)(frame.Power * 10);
      word a = (word)(frame.AccumulatedPower * 100);

      result = "OK 24 ";
      result += frame.Channel;
      result += " ";
      result += frame.Command;
      result += " ";
      result += frame.ID[0];
      result += " ";
      result += frame.ID[1];
      result += " ";
      result += frame.ID[2];
      result += " ";
      result += frame.Data;
      result += " ";
      result += (byte)(p >> 8);
      result += " ";
      result += (byte)(p & 0x00FF);
      result += " ";
      result += (byte)(a >> 8);
      result += " ";
      result += (byte)(a & 0x00FF);

      Log("to FHEM: " + result + GetPlugIdentifier(frame.ID[0], frame.ID[1], frame.ID[2], frame.Channel));
    }
  }

  return result;
}


PCA301::PCA301() {
  m_rfm = NULL;
  m_lastPoll = 0;
  m_expectedAnswer = ExpectedAnswers::None;
  m_retries = 0;
  m_doLogging = false;
}


void PCA301::SetLogItemCallback(LogItemCallbackType *callback) {
  m_logItemCallback = callback;
}

void PCA301::EnableLogging(bool enabled) {
  m_doLogging = enabled;
}

void PCA301::Begin(RFMxx *rfm, unsigned long frequency, word interval, TSettingsCallback callback) {
  Log("Initialized with " + String(frequency) + " kHz and " + String(interval) + " seconds poll", true);
  m_retries = 0;
  m_nextAction.Action = ActionTypes::None;
  m_settingsCallback = callback;
  m_lastPoll = millis();
  m_rfm = rfm;
  m_rfm->InitializePCA301();
  m_rfm->SetFrequency(frequency);
  
  m_plugList.OnSendPayload([this](byte pl[10]) {
    SendPayload(pl, false);
  });

  m_plugList.SetLogItemCallback([this] (String logItem) {
    Log(logItem);
  });

  m_plugList.SetSettingsCallback([this](String key, String value, bool write) {
    return m_settingsCallback(key, value, write);
  });

  m_plugList.Begin(interval);
}

bool PCA301::IsInitialized() {
  return m_rfm != NULL;
}

void PCA301::Log(String logItem, bool force) {
  if (m_logItemCallback != NULL) {
    if(m_doLogging) {
      m_logItemCallback(String(millis()) + ": " + logItem);
    }
    else if (force) {
      m_logItemCallback(logItem);
    }
  }
}

String PCA301::GetPlugIdentifier(byte id1, byte id2, byte id3, byte channel) {
  String result;

  result += " [ID=";
  result += String(id1, HEX);
  result += " ";
  result += String(id2, HEX);
  result += " ";
  result += String(id3, HEX);
  result.toUpperCase();
  result += "  Channel=";
  result += channel;
  result += "]";

  return result;
}

RFMxx *PCA301::GetUsedRadio() {
  return m_rfm;
}

bool PCA301::IsValidDataRate(unsigned long dataRate) {
  return dataRate == 6631ul;
}

void PCA301::CalculateCRC(byte data[], byte result[2]) {
  word rw = 0;
  int i, j;
  for (j = 0; j < PCA301::FRAME_LENGTH -2; j++) {
    rw = rw ^ ((uint16_t)data[j] << 8);
    for (i = 0; i<8; i++) {
      if (rw & 0x8000) {
        rw = (rw << 1) ^ 0x8005;
      }
      else {
        rw <<= 1;
      }
    }
  }

  result[0] = (byte)(rw >> 8);
  result[1] = (byte)(rw & 0xFF);
}

String ToHex(byte bt) {
  String result = String(bt, HEX);
  result = result.length() == 1 ? ("0" + result) : result;
  result.toUpperCase();
  return result;
}

String PCA301::AnalyzeFrame(byte *payload) {
  String result;
  struct PCA301::Frame frame;
  DecodeFrame(payload, &frame);

  // Show the raw data bytes
  result += "PCA301 [";
  for (int i = 0; i < PCA301::FRAME_LENGTH; i++) {
    result += ToHex(payload[i]) + " ";
  }
  result += "]";

  // Check CRC
  if (!frame.IsValid) {
    result += " CRC:WRONG";
  }
  else {
    result += " CRC:OK";
  }
  result.toUpperCase();
 
  result += frame.IsFromPlug ? " Plug" : " Other";

  // ID
  result += " ID:";
  result += ToHex(frame.ID[0]);
  result += ToHex(frame.ID[1]);
  result += ToHex(frame.ID[2]);

  result += " Ch:";
  result += frame.Channel;

  result += " Cmd:";
  result += frame.Command;

  // Power
  result += " W:";
  result += frame.Power;

  // Accumulated power
  result += " Acc:";
  result += frame.AccumulatedPower;

  // IsOn
  result += " IsOn:";
  result += frame.IsOn;

  // CRC
  result += " CRC:";
  result += ToHex(frame.CRC1);
  result += ToHex(frame.CRC2);

  

  return result;
}

