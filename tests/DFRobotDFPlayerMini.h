#pragma once
#include <utility>
#include <vector>
#define DFPlayerError 6
struct DFRobotDFPlayerMini {
  std::vector<std::pair<int,int>> played;
  std::vector<std::pair<uint8_t,uint16_t>> messages;
  int activeMessage = -1;
  bool begin(HardwareSerial&,bool,bool){return true;}
  bool available(){
    if (activeMessage >= 0) return true;
    if (messages.empty()) return false;
    activeMessage = 0;
    return true;
  }
  uint8_t readType(){return messages.at(activeMessage).first;}
  uint16_t read(){
    uint16_t detail = messages.at(activeMessage).second;
    messages.erase(messages.begin() + activeMessage);
    activeMessage = -1;
    return detail;
  }
  void volume(int){}
  void stop(){}
  void playFolder(int folder,int track){played.emplace_back(folder,track);}
};
