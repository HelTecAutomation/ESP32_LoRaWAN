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
 * www.heltec.cn
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

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DIO0    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define DIO1    35   // GPIO33 -- SX127x's IRQ(Interrupt Request)
uint32_t  LICENSE[4] = {0xDD251C39,0x44C5F937,0xBB160873,0x072EB7C8};

void setup()
{
// Add your initialization code here
  Serial.begin(115200);
  while (!Serial);
  SPI.begin(SCK,MISO,MOSI,SS);
  Mcu.begin(SS,RST,DIO0,DIO1,LICENSE);
  DeviceState = DEVICE_STATE_INIT;
}

// The loop function is called in an endless loop
void loop()
{
  switch( DeviceState )
  {
    case DEVICE_STATE_INIT:
    {
      LoRa.DeviceStateInit();
      if(IsLoRaMacNetworkJoined==false)
      {DeviceState = DEVICE_STATE_JOIN;}
      else
      {DeviceState = DEVICE_STATE_SEND;}
      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRa.DeviceStateJion();
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
      LoRa.DeviceSleep(IsLowPowerOn,DebugLevel);
      break;
    }
    default:
    {
      DeviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}

