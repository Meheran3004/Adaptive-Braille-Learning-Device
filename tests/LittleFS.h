#pragma once
inline std::vector<String> fileLines;
struct File {
  unsigned int cursor=0;
  explicit operator bool() const{return true;}
  bool available(){return cursor<fileLines.size();}
  String readStringUntil(char){return fileLines.at(cursor++);}
  void close(){}
};
struct Filesystem {
  bool begin(bool){return true;}
  File open(const char*,const char*){return File();}
};
inline Filesystem LittleFS;
