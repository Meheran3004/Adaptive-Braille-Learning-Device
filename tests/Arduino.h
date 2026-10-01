#pragma once
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <vector>
constexpr int HIGH=1, LOW=0, INPUT_PULLUP=2, OUTPUT=3, SERIAL_8N1=0;
inline unsigned long fakeNow=0;
inline int pins[40];
inline unsigned long millis(){return fakeNow;}
inline void pinMode(int,int){}
inline int digitalRead(int p){return pins[p];}
inline void digitalWrite(int p,int v){pins[p]=v;}
inline void randomSeed(unsigned long){}
inline long nextRandom=0;
inline long random(long limit){return limit > 0 ? nextRandom % limit : -1;}
class String {
  std::string value;
public:
  String(const char* s=""):value(s){}
  unsigned int length() const{return value.size();}
  char operator[](unsigned int n) const{return value.at(n);}
  void trim(){
    auto first=value.find_first_not_of(" \r\n\t");
    if(first==std::string::npos){value.clear();return;}
    value=value.substr(first,value.find_last_not_of(" \r\n\t")-first+1);
  }
  void toLowerCase(){std::transform(value.begin(),value.end(),value.begin(),[](unsigned char c){return std::tolower(c);});}
};
struct SerialStub {
  void begin(int){}
  template<class T> void print(T){}
  template<class T> void println(T){}
  void println(){}
};
inline SerialStub Serial;
