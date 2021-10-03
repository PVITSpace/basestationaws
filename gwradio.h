#pragma once
#include <RadioLib.h>
#include <list>
#include "bufferobj.h"

// pin mapping

// ESP32 with Ebyte module
//#define SCK                       18
//#define MISO                      19
//#define MOSI                      23
/*#define NSS                       25
#define DIO1                      14
#define BUSY                      27
#define RXEN                      12
#define TXEN                      13
#define NRST                      26*/

// Arduino Uno
#define NSS                       10
#define DIO1                      2
#define BUSY                      9
#define RXEN                      A4
#define TXEN                      A5
#define NRST                      3


//TTGO Beam
#define I2C_SDA         21
#define I2C_SCL         22

#define LORA_SCK        5
#define LORA_MISO       19
#define LORA_MOSI       27
#define LORA_SS         18
#define LORA_DIO0       26
#define LORA_DIO1       33
#define LORA_DIO2       32
#define LORA_RST        23

// ESP32 single-channel gateway using the built-in LoRa radio  
//Board ESP#@ Dev Module
//Upload Speed 921600
//CPU Frequeency  80
//Flash Frequency 80
//Flash Mode QIO
//Flash Size 4MB
//Partion Default 4MB with spiffs
//PSRAM Disabled
//#define SCK                       14
//#define MISO                      12
//#define MOSI                      13
#define NSS_BUILTIN               16
#define DIO0_BUILTIN              26
#define NRST_BUILTIN             -1// RADIOLIB_PIN_UNUSED
#define DIO1_BUILTIN              33

// configuration
//#define USE_E22_900M30S           // comment out when E22-900M30S is NOT being used
#define USE_ESP32_GATEWAY         // comment out when ESP32 single-channel gateway is NOT being used
#define USE_TTGO
#define PING_INTERVAL             2       // seconds, how much time to wait between PINGs sent from this node

#define FREQUENCY_900M30S         433.0//915.0   // MHz carrier, for E22-900M30S
#define FREQUENCY_400M30S         433.0   // MHz carrier, for E22-400M30S
#define FREQUENCY_GATEWAY         433.0   // MHz carrier, for ESP32 single-channel gateway
#define BANDWIDTH                 125.0   // kHz dual-sideband
#define SPREADING_FACTOR          9       // 2^9 chips
#define CODING_RATE               7       // 4/7 coding
#define SYNC_WORD                 0x12    // private network
#define OUTPUT_POWER              14      // +14 dBm
#define CURRENT_LIMIT             120.0    // mA  was 60
#define PREAMBLE_LENGTH           8       // symbols
#define TCXO_VOLTAGE              2.4     // V

// debug-only macros
#define APP_DEBUG
#ifdef APP_DEBUG
  #define APP_DEBUG_PORT         Serial
  #define APP_DEBUG_BEGIN(...) { APP_DEBUG_PORT.begin(__VA_ARGS__); }
  #define APP_DEBUG_WRITE(...) { APP_DEBUG_PORT.write(__VA_ARGS__); }
  #define APP_DEBUG_PRINT(...) { APP_DEBUG_PORT.print(__VA_ARGS__); }
  #define APP_DEBUG_PRINTLN(...) { APP_DEBUG_PORT.println(__VA_ARGS__); }
#else
  #define APP_DEBUG_BEGIN(...) {}
  #define APP_DEBUG_WRITE(...) {}
  #define APP_DEBUG_PRINT(...) {}
  #define APP_DEBUG_PRINTLN(...) {}
#endif

class gwradio{

#if defined(USE_TTGO)
  SX1278 lora = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_DIO1);
#elif defined(USE_ESP32_GATEWAY)
  //SPIClass spi(HSPI);
  //RFM96 lora = new Module(NSS_BUILTIN, DIO0_BUILTIN, NRST_BUILTIN, DIO1_BUILTIN, spi);
  RFM96 lora = new Module(NSS_BUILTIN, DIO0_BUILTIN, NRST_BUILTIN, DIO1_BUILTIN);
 // SX1280 lora = new Module(25, 27, 23, 19);
#elif defined(USE_E22_900M30S)
  SX1262 lora = new Module(NSS, DIO1, NRST, BUSY);
#else
  SX1268 lora = new Module(NSS, DIO1, NRST, BUSY);
#endif


    int transmissionState = ERR_NONE;  // save transmission state between loops
    bool transmitting = false;  // flag to indicate if we're currently sending ping packet or just listening
    unsigned long lastPing = 0;// timestamp of the last PING packet
    bool bAck=false;
    bool bValid=false;
 
  public:
    std::list<CBufferObj> TransmitList;
    std::list<CBufferObj> TransmittedList;
    std::list<CBufferObj> ReceivedList;
    gwradio(){}
    void setRfMode(bool transmit);// function to set RF mode to transmit or receive
    void setup();
    bool isTransmitTime();
    void TransmitCmd();
    void TransmitPacket(const char *buf, int len);
    void SetRadioReceive();
    void ReceivedPacket();
    void loopRadio();
    void loop();
};
