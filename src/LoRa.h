#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include "SPI.h"
#include "board.h"
#include "Commissioning.h"
#include "Mcu.h"


#if defined( USE_BAND_868 )

#include "LoRaMacTest.h"

/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON   true

#define USE_SEMTECH_DEFAULT_CHANNEL_LINEUP    1

#if ( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 )

#define LC4                { 867100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 867300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 867500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 867700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 867900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9                { 868800000, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }
#define LC10               { 868300000, { ( ( DR_6 << 4 ) | DR_6 ) }, 1 }

#endif

#endif

/*!
* Application port
*/
extern uint8_t AppPort;

/*!
* User application data size
*/
extern uint8_t AppDataSize;

/*!
* User application data
*/
extern uint8_t AppData[LORAWAN_APP_DATA_MAX_SIZE];//Ҫ���͵�����

/*!
* Indicates if the node is sending confirmed or unconfirmed messages
*/
extern uint8_t IsTxConfirmed;

/*!
* Defines the application data transmission duty cycle
*/
extern uint32_t TxDutyCycleTime;

/*!
* Timer to handle the application data transmission duty cycle
*/
extern TimerEvent_t TxNextPacketTimer;

/*!
* Specifies the state of the application LED
*/
extern bool AppLedStateOn;

/*!
* Indicates if a new packet can be sent
*/
extern bool NextTx;

extern enum eDeviceState DeviceState;
extern struct ComplianceTest_s ComplianceTest;

extern "C" void OnTxNextPacketTimerEvent( void );
extern "C" void PrepareTxFrame( uint8_t port );


class LoRaClass{
public:
  void DeviceStateInit(DeviceClass_t CLASS);
  void DeviceStateJion();
  void DeviceStateSend();
  void DeviceSleep(DeviceClass_t CLASS,uint8_t debuglevel);
};


extern LoRaClass LoRa;


#endif
