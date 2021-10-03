#include "gwradio.h"
#include "encode.h"


volatile bool operationDone = false;  // flag to indicate that a packet was sent or received
volatile bool enableInterrupt = true;  // disable interrupt when it's not needed


void setFlag(void) {  // this function is called when a complete packet is transmitted
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }

  // we sent or received a packet, set the flag
  operationDone = true;
}

// function to set RF mode to transmit or receive
void gwradio::setRfMode(bool transmit) {
  #if !defined(USE_ESP32_GATEWAY)
  if(transmit) {
    digitalWrite(RXEN, LOW);
    digitalWrite(TXEN, HIGH);
  } else {
    digitalWrite(RXEN, HIGH);
    digitalWrite(TXEN, LOW);
  }
  delay(20);  //100
  #endif
}

void gwradio::setup() {
  
  pinMode(RXEN, OUTPUT);
  pinMode(TXEN, OUTPUT);

  // initialize the radio
  Serial.print(F("Initializing ... "));

  #if defined(USE_TTGO)
    int state = lora.begin();
  #elif defined(USE_ESP32_GATEWAY)
  //  int state = lora.begin(FREQUENCY_GATEWAY, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SYNC_WORD, OUTPUT_POWER, CURRENT_LIMIT, PREAMBLE_LENGTH);
    int state = lora.begin();
  #elif defined(USE_E22_900M30S)
    int state = lora.begin(FREQUENCY_900M30S, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SYNC_WORD, OUTPUT_POWER, CURRENT_LIMIT, PREAMBLE_LENGTH, TCXO_VOLTAGE);
  #else
    int state = lora.begin(FREQUENCY_400M30S, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SYNC_WORD, OUTPUT_POWER, CURRENT_LIMIT, PREAMBLE_LENGTH, TCXO_VOLTAGE);
  #endif
  if(state == ERR_NONE) {
    Serial.println(F("success!"));
    bValid=true;
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);    
  }

  // set the function that will be called when packet transmission or reception is finished
  #if defined(USE_ESP32_GATEWAY)
    lora.setDio0Action(setFlag);
  #else
    lora.setDio1Action(setFlag);
  #endif

  // set receive mode
  setRfMode(false);
  lora.startReceive();
}

// check if it's time to transmit a ping packet

bool gwradio::isTransmitTime(){
     if(millis() - lastPing > (unsigned long)PING_INTERVAL * (unsigned long)1000)  {
        return true;
     }
     return false;
}

void gwradio::TransmitPacket(const char *buf, int len){

    // set mode to transmission
    setRfMode(true);
    transmitting = true;

    // start transmitting
    Serial.print(F("Sending "));
   // transmissionState = lora.startTransmit(str);
    transmissionState = lora.startTransmit(buf,len);
   
    // save timestamp
    lastPing = millis();
}

void gwradio::SetRadioReceive(){
    setRfMode(false);
      transmitting = false;
      Serial.print(F("Starting to listen ... "));
      int state = lora.startReceive();
      if (state == ERR_NONE) {
        // packet was successfully sent
        Serial.println(F(" Started successfully!"));
 
      } else {
        Serial.print(F(" Failed to start reception, code "));
        Serial.println(state);
 
      }
}

//Packet received.  Place in Rcvd queue.
//-------------------------------- Not adding image stream data yet

#define STREAMHEADERLEN 15
void gwradio::ReceivedPacket(){
       Serial.println();
       Serial.println(F("---------------------------------------------------------------------------------  Received packet!"));

      byte buffer[500]={NULL};
      byte apibuffer[500]={NULL};
      int apibufferlen=0;
      //int state = lora.readData(str);
      int len=lora.getPacketLength();
      int state = lora.readData(buffer,len);

      if((state == ERR_NONE)&&(len>0)) {
       // print RSSI (Received Signal Strength Indicator)
        Serial.print(F("RSSI:\t\t"));
        Serial.print(lora.getRSSI());
        Serial.println(F(" dBm"));
 
        // print SNR (Signal-to-Noise Ratio)
        Serial.print(F("SNR:\t\t"));
        Serial.print(lora.getSNR());
        Serial.println(F(" dB"));
        std::string ss="table:chunks~cid:";
        if ((char)buffer[0]=='X'){
           Serial.println(F("-----------------------------------Received packet Streamer! -----------------------"));
          if(len<STREAMHEADERLEN)   Serial.println(F("Received packet ERROR - Streamer!"));
          int count=0;
          for (count=1;count<STREAMHEADERLEN;count++){
            char c=(char)buffer[count];
            if (c=='B'){
              ss+="~HASMORE:0~BLOCK:";
              break;
            }
             if (c=='b'){
              ss+="~HASMORE:1~BLOCK:";
              break;
            }
            ss+=c;
          }
          for (count+=1;count<STREAMHEADERLEN;count++){
            char c=(char)buffer[count];
            if (c!=' ') ss+=c;
          }

           Serial.print(F("------------------- Data:\t\t"));
           Serial.println(ss.c_str());

       

           CBufferObj bb(buffer+STREAMHEADERLEN,len-STREAMHEADERLEN);
           bb.str=ss;
           ReceivedList.push_back(bb);
        }
        else {
          std::string str; 
          for (int count=0;count<len;count++) str+=buffer[count];   
     
          str=str+"&rssi="+std::to_string(lora.getRSSI())+"&snr="+std::to_string(lora.getSNR());
     
          Serial.print(F("Data:\t\t"));
          Serial.println(str.c_str());
                   
          //ReceivedList.push_back(str.c_str());
           CBufferObj bb(str.c_str());
           ReceivedList.push_back(bb);
           }  
      } else if (state == ERR_CRC_MISMATCH) {
        // packet was received, but is malformed
        Serial.println(F("CRC error!"));
 
      } else {
        // some other error occurred
        Serial.print(F("failed, code "));
        Serial.println(state);
 
      }
      lastPing = millis();

}

void gwradio::TransmitCmd(){
    if (TransmitList.size()<1) return;
    CBufferObj bufobj=TransmitList.front();
    std::string str=bufobj.str;
    std::string cmdstr=convertStringToCommand(str);
    TransmitPacket(cmdstr.c_str(),cmdstr.length());
    TransmitList.pop_front();

    TransmittedList.push_back(bufobj);
    Serial.println();
    Serial.print(F("Transmitting Cmd"));

  }


void gwradio::loopRadio(){

   // check if it's time to transmit a ping packet or what is in queue

  
  if (isTransmitTime()) {
    
   if(bAck){
  //  TransmitPacket();
    bAck=false;
    }
   else TransmitCmd();
    
  }
 
  // check if the previous operation finished     this is set by interrupt
  if(operationDone) {
    // disable the interrupt service routine while processing the data
    enableInterrupt = false;

    // check which operation finished
    if(transmitting) {
      // packet transmission finished, check the result
      if (transmissionState == ERR_NONE) {
        // packet was successfully sent
        Serial.println(F(" ... Transmission finished!"));
 
      } else {
        Serial.print(F("failed, code "));
        Serial.println(transmissionState);
      }

      // set mode to reception
      SetRadioReceive();
   
    } else {
      // received packet, read the data

      ReceivedPacket();
      SetRadioReceive();

    }

    operationDone = false;       // reset flag
    enableInterrupt = true;      // we're ready to send more packets, enable interrupt service routine
  }
}


void gwradio::loop() {
  if(bValid)  loopRadio();
}
