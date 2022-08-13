#include "PCA301Plug.h"


PCA301Plug::PCA301Plug() {

}

String PCA301Plug::GetIdString() {
  return BuildIdString(ID);
}

String PCA301Plug::BuildIdString(byte id[3]) {
  char result[6];
  sprintf(result, "%02X%02X%02X", id[0], id[1], id[2]);
  return result;
}

void PCA301Plug::SetIdString(String id) {
  long l = strtol(id.c_str(), NULL, 16);
  ID[0] = l >> 16;
  ID[1] = l >> 8 & 0xFF;
  ID[2] = l & 0xFF;
}