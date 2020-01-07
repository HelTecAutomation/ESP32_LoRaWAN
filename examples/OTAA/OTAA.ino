/*
 * HelTec Automation(TM) LoRaWAN 1.0.2 OTAA example use OTAA, CLASS A
 *
 * Function summary:
 *
 * - use internal RTC(15KHz);
 *
 * - Include stop mode and deep sleep mode;
 *
 * - 60S data send cycle;
 *
 * - Informations output via serial(115200);
 *
 * - Debug log message can be configed in board.h(DebugLevel);
 *
 * - Only ESP32 + LoRa series boards can use this library, need a license
 *   to make the code run(check you license here: http://www.heltec.cn/search/);
 *
 * You can change some definition in "Commissioning.h" and "LoRaMac-definitions.h"
 *
 * HelTec AutoMation, Chengdu, China.
 * 成都惠利特自动化科技有限公司
 * https://heltec.org
 * support@heltec.cn
 *
 *this project also release in GitHub:
 *https://github.com/HelTecAutomation/ESP32_LoRaWAN
*/

#include "Arduino.h"
#include "board.h"
#include "LoRaMac.h"
#include "Commissioning.h"

#include <SPI.h>
#include <LoRa.h>
#include <Mcu.h>

#define  V2
#define  CLASS  CLASS_A

#ifdef V2 //WIFI Kit series V1 not support Vext control
  #define DIO1    35   // GPIO35 -- SX127x's IRQ(Interrupt Request) V2
#else
  #define DIO1    33   // GPIO33 -- SX127x's IRQ(Interrupt Request) V1
#endif

#define Vext  21
bool OVER_THE_AIR_ACTIVATION = true;
//OTAA
uint8_t DevEui[] =  { 0x22,0x32,0x33,  0x00, 0x00, 0x88, 0x88, 0x33 };
uint8_t AppEui[] =   { 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x04 };
uint8_t AppKey[] =  { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x66, 0x01 };
//ABP
uint32_t ABP_DevAddr =     ( uint32_t )0x26011713 ;
RTC_DATA_ATTR uint8_t  ABP_NwkSKey[] =  { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x66, 0x02 };
RTC_DATA_ATTR uint8_t  ABP_AppSKey[] =  { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x66, 0x01 };
//LICENSE
uint32_t  LICENSE[4] = {0xB4C6B30A,0x4AE94785,0xDCCD9893,0x2D2775D8};//470v2
//Delivery of data on port 2
void app(uint8_t data)
 {
	 lora_printf("data:%d\r\n",data);
 }
void setup()
{
// Add your initialization code here
  Serial.begin(115200);
  while (!Serial);
  SPI.begin(SCK,MISO,MOSI,SS);
  Mcu.begin(SS,RST_SX127x,DIO0,DIO1,LICENSE);
  DeviceState = DEVICE_STATE_INIT;
}

// The loop function is called in an endless loop
void loop()
{
  switch( DeviceState )
  {
    case DEVICE_STATE_INIT:
    {
      LoRa.DeviceStateInit(CLASS);
      if(IsLoRaMacNetworkJoined==false)
      {DeviceState = DEVICE_STATE_JOIN;}
      else
      {DeviceState = DEVICE_STATE_SEND;}
      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRa.DeviceStateJion(OVER_THE_AIR_ACTIVATION);
      break;
    }
    case DEVICE_STATE_SEND:
    {
      lora_printf("Into send state\n");
      PrepareTxFrame( AppPort );
      LoRa.DeviceStateSend();
      DeviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      // Schedule next packet transmission
      TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
      TimerStart( &TxNextPacketTimer );
      DeviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP:
    {
      LoRa.DeviceSleep(CLASS,DebugLevel);
      break;
    }
    default:
    {
      DeviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}
