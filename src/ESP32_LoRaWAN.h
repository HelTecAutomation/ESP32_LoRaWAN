#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include "SPI.h"
#include "board.h"
#include "Commissioning.h"
#include "Mcu.h"
#include "utilities.h"
#include "board-config.h"
#include "LoRaMac.h"
#include "Commissioning.h"
#include "rtc-board.h"
#include "delay.h"
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

enum eDeviceState
{
    DEVICE_STATE_INIT,
    DEVICE_STATE_JOIN,
    DEVICE_STATE_SEND,
    DEVICE_STATE_CYCLE,
    DEVICE_STATE_SLEEP
};


#define APP_TX_DUTYCYCLE_RND          1000

class LoRaWanClass{
public:
  void init(DeviceClass_t classMode,LoRaMacRegion_t region);
  void join();
  void send(DeviceClass_t classMode);
  void cycle(uint32_t dutyCycle);
  void sleep(DeviceClass_t classMode,uint8_t debugLevel);
  void displayJoining();
  void displayJoined();
  void displaySending();
  void displayAck();
  void displayMcuInit();
  void generateDeveuiByChipID();
};

extern enum eDeviceState deviceState;
extern uint8_t appPort;
extern uint32_t txDutyCycleTime;
extern uint8_t appData[LORAWAN_APP_DATA_MAX_SIZE];
extern uint8_t appDataSize;
extern uint32_t txDutyCycleTime;
extern bool overTheAirActivation;
extern LoRaMacRegion_t loraWanRegion;
extern bool loraWanAdr;
extern bool isTxConfirmed;
extern uint32_t appTxDutyCycle;
extern uint8_t confirmedNbTrials;
extern DeviceClass_t  loraWanClass;
extern uint8_t DevEui[];
extern uint8_t AppEui[];
extern uint8_t AppKey[];
extern uint8_t NwkSKey[];
extern uint8_t AppSKey[];
extern uint32_t DevAddr;
extern uint8_t idDisplayJoined;
extern uint8_t ifDisplayAck;
extern uint16_t userChannelsMask[6];

extern LoRaWanClass LoRaWAN;

#if defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ) || defined( Wireless_Stick )
extern SSD1306 Display;
#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
