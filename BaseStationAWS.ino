#include "cbasestation.h"
#include <esp_task_wdt.h>
//3 seconds WDT
#define WDT_TIMEOUT 3


unsigned long llcount=0;
CBaseStation bs;


void setup() {
  Serial.begin(115200);
  while(!Serial);
  bs.connectWifi();
  bs.radio.setup();
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0
void loop() {
  esp_task_wdt_reset();
  while(1){
    if(llcount%1000000==0) {
     Serial.print("x");
    // resetFunc();
     
   }
  }
  if(llcount%150000000==0) Serial.println(".");
  if(bs.isCmdTime())  {

    //bs.getLastCommand();   //Restarts when connecting to service has some problem
  }
  bs.radio.loop();
  bs.updateAllTransmittedCommands();
  bs.updateAllReceivedCommands();
  llcount++;
}
