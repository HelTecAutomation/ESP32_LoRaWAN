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
 * - Debug informations can be configed in board.h(Debug_Level);
 *
 * - Informations output via serial(115200);
 *
 * - Informations shown in OLED;
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
#include <SPI.h>
#include <LoRa.h>
#include <Mcu.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`


#define  V2
#define  CLASS  CLASS_A



#ifdef V2 //WIFI Kit series V1 not support Vext control
	#define DIO1    35   // GPIO35 -- SX127x's IRQ(Interrupt Request) V2
#else
	#define DIO1    33   // GPIO33 -- SX127x's IRQ(Interrupt Request) V1
#endif

#define Vext  21

uint32_t  LICENSE[4] = {0x6899A6F1,0xD6A04116,0x7EDEA042,0x7E7218C3};//470v2

SSD1306  display(0x3c, SDA, SCL, RST_LED);
extern McpsIndication_t McpsIndication;

void LEDdisplayJOINING()
{
	digitalWrite(Vext,LOW);
	delay(50);
	display.wakeup();
	display.clear();
	display.drawString(58, 22, "JOINING...");
	display.display();
}
void LEDdisplayJOINED()
{
	digitalWrite(Vext,LOW);
	delay(50);
	display.wakeup();
	display.clear();
	display.drawString(64, 22, "JOINED");
	display.display();
	delay(1000);
	display.sleep();
	digitalWrite(Vext,HIGH);
}
void LEDdisplaySENDING()
{
	digitalWrite(Vext,LOW);
	delay(10);
	display.wakeup();
	display.init();
	delay(50);
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.clear();
	display.drawString(58, 22, "SENDING...");
	display.display();
}
void LEDdisplayACKED()
{
	display.clear();
	display.drawString(64, 10, "ACK RECEIVED");
	display.setFont(ArialMT_Plain_10);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.drawString(64, 40,"rssi="+String(McpsIndication.Rssi)+
			",datarate="+String(McpsIndication.RxDatarate));
	if(CLASS==CLASS_A)
	{
		display.drawString(64, 50, "Into deep sleep in 2S");
	}
	display.display();
	if(CLASS==CLASS_A)
	{	delay(2000);
		display.sleep();
		digitalWrite(Vext,HIGH);
	}
}
void LEDdisplaySTART()
{
	display.wakeup();
	display.init();
	delay(100);
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.clear();
	display.drawString(64, 11, "LORAWAN");
	display.drawString(64, 33, "STARTING");
	display.display();
}

void setup()
{
	pinMode(Vext,OUTPUT);
	digitalWrite(Vext, LOW);    // OLED USE Vext as power supply, must turn ON Vext before OLED init

	Serial.begin(115200);
	while (!Serial);
	delay(100);

	SPI.begin(SCK,MISO,MOSI,SS);
	if(LoraWanStarted==0)
	{
		LEDdisplaySTART();
	}
	Mcu.begin(SS,RST_SX127x,DIO0,DIO1,LICENSE);

	DeviceState = DEVICE_STATE_INIT;

}



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
			LEDdisplayJOINING();
			LoRa.DeviceStateJion();
			break;
		}
		case DEVICE_STATE_SEND:
		{
			if(isJioned == 1)
			{
				isJioned--;
				LEDdisplayJOINED();
			}
			//lora_printf("LoRaMacParams.ChannelsMask[0]:%d\r\n",LoRaMacParams.ChannelsMask[0]);
			LEDdisplaySENDING();
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
			if(isAckReceived==1)
			{
				isAckReceived--;
				LEDdisplayACKED();

			}
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


