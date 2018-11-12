/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__


#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "utilities.h"
#include "timer.h"
#include "delay.h"


#include "radio.h"
#include "sx1276.h"
#include "rtc-board.h"
#include "sx1276-board.h"


#if defined( USE_USB_CDC )
#include "uart-usb-board.h"
#endif

// Set this to 0 -- Only send and ACK status will be printed;
// Set this to 1 -- Based on level 0, more TX and RX frequency will be printed;
// Set this to 2 -- Based on level 1, DIO0, DIO1 interrupt and deep sleep informations will be printed
//					print message "S": system into deep sleep mode
//					print message "0": DIO0 interrupt(TX and RX interrupt)
//					print message "1": DIO1 interrupt(timeout interrupt)
#define DebugLevel 	2

//set IsLowPowerOn to 1--open lowpower mode;
//set IsLowPowerOn to 0--close deepsleep mode;
#define IsLowPowerOn 1

/*!
 * Define indicating if an external IO expander is to be used
 */
//#define BOARD_IOE_EXT

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS  1
#endif

#ifndef FAIL0
#define FAIL0   0
#endif


/*!
 * Possible power sources
 */
enum BoardPowerSources
{
    USB_POWER = 0,
    BATTERY_POWER,
};

/*!
 * \brief Disable interrupts
 *
 * \remark IRQ nesting is managed
 */
void BoardDisableIrq( void );

/*!
 * \brief Enable interrupts
 *
 * \remark IRQ nesting is managed
 */
void BoardEnableIrq( void );

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Battery voltage
 *
 * \retval value  battery voltage in volts
 */
uint32_t BoardGetBatteryVoltage( void );

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level [  0: USB,
 *                                 1: Min level,
 *                                 x: level
 *                               254: fully charged,
 *                               255: Error]
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Get the board power source
 *
 * \retval value  power source [0: USB_POWER, 1: BATTERY_POWER]
 */
uint8_t GetBoardPowerSource( void );

uint32_t HexToString(/*IN*/  const char    * pHex,  
                     /*IN*/  uint32_t           hexLen,  
                     /*OUT*/ char          * pByteString);
#ifdef __cplusplus
}
#endif
#endif // __BOARD_H__
