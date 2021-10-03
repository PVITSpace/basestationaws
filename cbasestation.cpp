#include "cbasestation.h"


bool CBaseStation::isCmdTime(){
     if(millis() - lastCmd >= (unsigned long)CMD_INTERVAL * (unsigned long)1000)  {
         lastCmd=millis();
         return true;
     }
     return false;
}

void CBaseStation::connectWifi() {
  Serial.begin(serialBaudRate);

  Serial.print("Starting BaseStation                          -----------------             Connecting to wifi: ");
  // You can add the id and password for several WiFi networks
  // it will connect to the first one it sees.
  wifiMulti.addAP("Truffle1", "Alexander1");
  wifiMulti.addAP("Cabana", "alexander");

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(delayWaitingForWifiConnection);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected to ");
  Serial.print(WiFi.SSID());
  Serial.print(" with IP ");
  Serial.println(String(WiFi.localIP()));
}


void CBaseStation::insertSatData(CBufferObj &bufobj){

  String str=bufobj.str.c_str();
  str=urlencode(str);
  Serial.println("---------------------------------------------------");
  Serial.print("Inserting Data:<");
  Serial.print(str);
  Serial.println(">");
  Serial.println("---------------------------------------------------");
  client.setTimeout(10000);
  if (!client.connect(host, httpsPort)) {
    Serial.println(F("Connection failed"));
  }
  else{
    client.print(
      String("GET ")+logpath+str+" HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "User-Agent: SeaKing\r\n" +
      "Connection: close\r\n\r\n"
    );
  // Disconnect

   Serial.println(
      String("GET ")+logpath+str+" HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "User-Agent: SeaKing\r\n" +
      "Connection: close\r\n\r\n"
    );
  client.stop();
  }
}


void CBaseStation::updateAllReceivedCommands(){
  if (!radio.ReceivedList.size()) return;
  int count=0;
  Serial.println("updateAllReceivedCommands()  {");
  while(radio.ReceivedList.size()){
      CBufferObj bufobj=radio.ReceivedList.front();
      if (bufobj.isAck()) {
        updateAckCmd(bufobj);
        }
        else{ 
          if(bufobj.str.length()>1)  insertSatData(bufobj);
          else {  //Its a stream  Don't know how to add yet
            
          }
      }

      
      radio.ReceivedList.pop_front();
      count++;
      if(count>10) break;
    }
   Serial.println("}END    updateAllReceivedCommands()");  
  }


void CBaseStation::updateAllTransmittedCommands(){
  int count=0;
  while(radio.TransmittedList.size()){
     std::string str=radio.TransmittedList.front().str;
      StaticJsonDocument<500> doc;
       
      DeserializationError error = deserializeJson(doc, str);

    // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      } 
      else{
        const char* cidstr=doc["cid"];
        updateTransmittedCmd(cidstr);
      }

      radio.TransmittedList.pop_front();
      count++;
      if(count>10) break;
    }
  
  }



void CBaseStation::updateTransmittedCmd(const char * strdata){
  
  
  Serial.println();
  Serial.print("Transmit Update: ");
  Serial.println(strdata);

  String str=strdata;
  str=urlencode(str);

  client.setTimeout(10000);
  if (!client.connect(host, httpsPort)) {
    Serial.println(F("Connection failed"));
  }
  else{
    client.print(
      String("GET ")+updatecmdpath+str+" HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "User-Agent: SeaKing\r\n" +
      "Connection: close\r\n\r\n"
    );

      // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }
  // Disconnect
  client.stop();
  }
}


void CBaseStation::updateAckCmd(CBufferObj &bufobj){
  std::string str=bufobj.str.c_str();
  std::string cid;
  int len=str.length();
  if (len<4) return;

  if ((str.substr(0,4)=="ACK:")||(str.substr(0,4)=="ack:")||(str.substr(0,4)=="Ack:")) {
    
  }
  else return;

  for(int count=4;count<len;count++){
    char c;
    c=str[count];
    if(c=='&') break;
    cid+=c; 
  }


  Serial.println();
  Serial.print("Transmit Update: ");
  Serial.println(cid.c_str());

  client.setTimeout(10000);
  if (!client.connect(host, httpsPort)) {
    Serial.println(F("Connection failed"));
  }
  else{
    client.print(
      String("GET ")+updateackpath+String(cid.c_str())+" HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "User-Agent: SeaKing\r\n" +
      "Connection: close\r\n\r\n"
    );

      // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }
  // Disconnect
  client.stop();
  }
}



void CBaseStation::getEchoData(const char * strdata=""){
 String str=strdata;
 str=urlencode(str);
  client.setTimeout(10000);
  if (!client.connect(host, httpsPort)) {
    Serial.println(F("Connection failed"));
  }
  else{
    client.print(
      String("GET ")+consolelogpath+str+" HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "User-Agent: SeaKing\r\n" +
      "Connection: close\r\n\r\n"
    );
  // Disconnect
  client.stop();
  }
}

void CBaseStation::getLastCommand() {
  long counter=0,endt=0,startt=millis();
  StaticJsonDocument<500> doc;
  client.setTimeout(10000);
  if (!client.connect(host, httpsPort)) {
    Serial.println(F("Connection failed"));
    connectWifi();
  }
  else{
    client.print(
      String("GET ")+cmdpath+" HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "User-Agent: SeaKing\r\n" +
      "Connection: close\r\n\r\n"
    );


    while (client.connected()) {
      counter++;
        // Skip HTTP headers
      char endOfHeaders[] = "\r\n\r\n";
      if (!client.find(endOfHeaders)) {  //May return nothing
        return;
      }
   //   Serial.print("Data:");
      String line = client.readStringUntil('\n');
      if(line.length()<3) {
        //Serial.print(".");
        break;
      }
      line=line.substring(1,line.length()-1);  //Gets rid of []   This JSon doesnt like it

  Serial.print("line: ");

      DeserializationError error = deserializeJson(doc, line);

    // Test if parsing succeeds.
    if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }  
    else
      {
    
        if(line.length()>2)  {
          CBufferObj bufobj(std::string(line.c_str()));
          radio.TransmitList.push_back(bufobj);  
        }
     
      }
      if (line == "\r") {
       // break;
      }
    }

  // Disconnect

    endt=millis();
  Serial.println();
  Serial.print(counter);
  Serial.print("Get Time: ");
  Serial.println((endt-startt)/1000.0);
  client.stop();
  } 


}
