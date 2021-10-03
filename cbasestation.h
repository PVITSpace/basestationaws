#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include "encode.h"
#include "gwradio.h"


// This code
// 1. Connects to a WiFi network
// 2. Loops:
//    a. Makes an http request to a URL
//    b. Parses the response, expecting an integer between 0 and 1000
//    c. Outputs that on PIN D0 as a voltage between 0 and 3.6V
//    d. Waits for 60 seconds


// Wifi settings are wifiMulti.addAP(ssid, password) below

#define CMD_INTERVAL 20
class CBaseStation{
  private:

  class ConsoleEcho{
  private:
    std::string str;
    std::string strtolog;
    CBaseStation *pBS;
  public:
  
  
  
    void setBS( CBaseStation *ptmp){pBS=ptmp;}
    void echo(){Serial.println(str.c_str());  strtolog+=str;  str="";}
    void logstr(const char *pstr){pBS->getEchoData(pstr);}
    void log(){pBS->getEchoData(strtolog.c_str()); strtolog="";}
    void begin(long tmp){Serial.begin(tmp);}
    void print(const char *tmp){str=str+tmp;}
   // void print(long tmp){str=str+tmp;}
    void print(std::string tmp){str=str+tmp;}
    void print(String tmp){str=str+tmp.c_str();}
  
    void println(const char *tmp=""){print(tmp);echo();}
    void println(std::string tmp){print(tmp);echo();}
    void println(String tmp){print(tmp);echo();}
} SerialB;


WiFiClientSecure client;
WiFiMulti wifiMulti;
//ConsoleEcho SerialB;

// The code calls the path on the host below and 
// expects a response with one line containing an integer
// between 0 and maxMetric.


const char* host = "9nxvzfnhrd.execute-api.us-east-1.amazonaws.com";
const char* cmdpath = "/v1/commands?latest=sent&value=false";
const char* updatecmdpath = "/v1/commands?sent=true&cid=";
const char* updateackpath = "/v1/commands?ack=true&cid=";
const char* logpath = "/v1/satlog?bid=1&rssi=0&data=";
const char* consolelogpath = "/v1/errorlog?bid=1&data=";



const int httpsPort = 443;
const int serialBaudRate = 115200;
const int delayWaitingForWifiConnection = 500;

unsigned long lastCmd = 0;

public:
  gwradio radio;

CBaseStation(){SerialB.setBS(this);}
bool isCmdTime();
void connectWifi();
void updateTransmittedCmd(const char * strdata);
void updateAckCmd(CBufferObj &bufobj);
void updateAllTransmittedCommands();
void updateAllReceivedCommands();
void insertSatData(CBufferObj &bufobj);
void getEchoData(const char * strdata);
void getLastCommand();
};
