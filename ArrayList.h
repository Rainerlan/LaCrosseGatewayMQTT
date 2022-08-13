#ifndef _ARRAYLIST_H
#define _ARRAYLIST_H

#include "Arduino.h"

template<typename V, unsigned int capacity>
class ArrayList {
  public:
    ArrayList();
    ~ArrayList();
    unsigned int Size();
    void Put(V value);
    V Get(int pos);
    void Remove(int pos);
    void Dump();
    void Clear();
    int GetCapacity();
    void FromString(String data, char delimiter, char blockSign);
    
  protected:
    V m_values[capacity];
    int m_position;
    int m_capacity;

};

template<typename V, unsigned int capacity>
int ArrayList<V, capacity>::GetCapacity() {
  return m_capacity;
}

template<typename V, unsigned int capacity>
ArrayList<V, capacity>::ArrayList() {
  m_capacity = capacity;
  Clear();
}

template<typename V, unsigned int capacity>
void ArrayList<V, capacity>::Clear() {
  m_position = 0;
}


template<typename V, unsigned int capacity>
ArrayList<V, capacity>::~ArrayList() {
}

template<typename V, unsigned int capacity>
unsigned int ArrayList<V, capacity>::Size() {
  return m_position;
}

template<typename V, unsigned int capacity>
void ArrayList<V, capacity>::Put(V value) {
  if (m_position +1 < capacity) {
    m_values[m_position] = value;
    m_position++;
  }
}

template<typename V, unsigned int capacity>
V ArrayList<V, capacity>::Get(int pos) {
  if (pos <= m_position) {
    return m_values[pos];
  }
}

template<typename V, unsigned int capacity>
void ArrayList<V, capacity>::Remove(int index) {
  for (int i = index; i < capacity - 1; i++) {
    m_values[i] = m_values[i + 1];
  }
  m_position--;
}

template<typename V, unsigned int capacity>
void ArrayList<V, capacity>::Dump() {
  for (unsigned int i = 0; i < Size(); i++) {
    Serial.print(m_values[i]);
    Serial.println();
  }
}

template<typename V, unsigned int capacity>
void ArrayList<V, capacity>::FromString(String data, char delimiter, char blockSign) {
  String part;
  bool inBlock = false;
  for (uint i = 0; i < data.length(); i++) {
    if (data[i] == blockSign) {
      inBlock = !inBlock;
    }
    if (i == data.length() - 1) {
      part += data[i];
      Put(part);
    }
    else if (data[i] == delimiter && !inBlock) {
      part += data[i];
      Put(part);
      part = "";
    }
    else {
      part += data[i];
    }
  }


}



#endif

