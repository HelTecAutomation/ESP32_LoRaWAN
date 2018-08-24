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
 * Board IO Extender pins definitions
 */

/*!
 * Board MCU pins definitions
 */

/*
#define RADIO_DIO_0                                 PB_0
#define RADIO_DIO_1                                 PB_1
#define RADIO_DIO_2                                 PB_2
#define RADIO_DIO_3                                 PB_3
//#define RADIO_DIO_4                                 PB_4
//#define RADIO_DIO_5                                 PB_5

#define RF_RXTX1                                    PB_7 
#define RF_RXTX2                                    PB_8 

#define RADIO_RESET                                 PB_9

#define RADIO_NSS                                   PB_12
#define RADIO_SCLK                                  PB_13
#define RADIO_MISO                                  PB_14
#define RADIO_MOSI                                  PB_15

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

#define UART_TX                                     PA_9
#define UART_RX                                     PA_10

#define SWDIO                                       PA_13
#define SWCLK                                       PA_14
*/
/*!
 * Board MCU unusedpins definitions
 */

/*!
 * MCU objects
 */



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
