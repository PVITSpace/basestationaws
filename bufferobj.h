#pragma once


namespace std {
  #ifndef to_string
  inline string to_string(int _Val)
  {    // convert int to string
      char _Buf[256];
      sprintf(_Buf, "%d", _Val);
      return (string(_Buf));
  }

   inline string to_string(long _Val)
  {    // convert int to string
      char _Buf[256];
      sprintf(_Buf, "%D", _Val);
      return (string(_Buf));
  }

    inline string to_string(float _Val)
  {    // convert int to string
      char _Buf[256];
      sprintf(_Buf, "%f", _Val);
      return (string(_Buf));
  }
  #endif




  #ifndef stoi
  inline int stoi(const char * str)
  {
    return String(str).toInt();
  }

    inline float stof(const char * str)
  {
    return String(str).toFloat();
  }

   inline int stoi(string str)
  {
     return String(str.c_str()).toInt();
  }

    inline float stof(string str)
  {
    return String(str.c_str()).toFloat();
  }
  #endif
}


struct CBufferObj {
  byte buffer[500];
  int len=0;
  std::string str;

  CBufferObj(const byte * buf,int l){
    memcpy(buffer,buf,l);
    len=l; 
  }

  CBufferObj(std::string s){
   str=s;
  }

  bool isAck(){
    if (str.length()<4) return false;
    if ((str.substr(0,4)=="ACK:")||(str.substr(0,4)=="ack:")||(str.substr(0,4)=="Ack:")) return true;
  }
};
