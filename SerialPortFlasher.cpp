#include "SerialPortFlasher.h"


SerialPortFlasher::SerialPortFlasher() {
  ////m_isUploading = false;
  m_state = State::Finished;
}

void SerialPortFlasher::Begin() {
  m_receivedSize = 0;
  m_lastReception = millis();
  ////m_isUploading = true;
  m_sizeBuffer = "";
  m_state = State::GetSize;

  uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  Update.begin(maxSketchSpace);
}

////void SerialPortFlasher::End() {
////  m_isUploading = false;
////}

bool SerialPortFlasher::IsUploading() {
  return m_state != State::Finished;
}

/*
Protocol                                               
-------------------------------------------------------
-> 418320S         file size                           
<- 1

-> b               b: Byte = nbr. of data bytes        
-> bbbbb ... b     data bytes                          
<- C               C: byte with checksum

-> b               b: Byte = nbr. of data bytes        
-> bbbbb ... b     data bytes
<- b               b: byte with checksum

...

-> b               b: Byte = nbr. of data bytes        
-> bbbbb ... b     data bytes
<- b               b: byte with checksum

<- Finish Message

*/
void SerialPortFlasher::Add(byte b) {
  m_lastReception = millis();

  if(m_state == State::GetSize) {
    if((char)b == 'S') {
      m_expectedSize = m_sizeBuffer.toInt();
      m_state = State::GetBlockSize;
      Serial.print(1);
    }
    else {
      m_sizeBuffer += (char)b;
    }
  }
  else if(m_state == State::GetBlockSize) {
    m_blockSize = b;
    m_state = State::GetBytes;
    m_dataBufferPointer = 0;
    m_checksum = 0;
  }
  else if(m_state == State::GetBytes) {
    m_dataBuffer[m_dataBufferPointer++] = b;
    m_checksum += b;
    m_receivedSize++;
    if(m_dataBufferPointer == m_blockSize) {
      Update.write(m_dataBuffer, m_blockSize);
      m_state = State::GetBlockSize;
      Serial.print(m_checksum);
      
      if(m_receivedSize == m_expectedSize) {
        m_state = State::Finished;
        Serial.print("Finished");
        Serial.println("Update.end: " + String(Update.end(true)));
        Serial.print("Error: ");
        Update.printError(Serial);
        ESP.restart();
      }
      
    }
  }
  
  //// m_receivedSize++;
  ////if (Update.write(&b, 1) != 1) {
  ////  Serial.println(Update.getError());
  ////}

  ////Serial.println(b, HEX);

  ////if (m_receivedSize % 5000 == 0) {
  ////  Serial.println(String(m_receivedSize));
  ////}

  ////Serial.print(b);
  ////Serial.print(".");
}

void SerialPortFlasher::Handle() {
  if (IsUploading() && millis() > m_lastReception + 2000) {
    Serial.println("ERROR: timeout");
    m_state = State::Finished;
    
    ////Serial.println("Finished: " + String(m_receivedSize));
    
    ////delay(100);
    ////Serial.println("Update.end: " + String(Update.end(true)));
    ////Serial.print("Error: ");
    ////Update.printError(Serial);

    ////Serial.println("restart");
    ////ESP.restart();
  }
}

