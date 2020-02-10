/*!
 * \file      board-config.h
 *
 * \brief     Board configuration
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */

#define BOARD_TCXO_WAKEUP_TIME                      0


/*!
 * Board MCU pins definitions
 */
#define RADIO_RESET                                 14

#define RADIO_MOSI                                  27
#define RADIO_MISO                                  19
#define RADIO_SCLK                                  5
#define RADIO_NSS                                   18

#define RADIO_DIO_0                                 26

#define V2
#ifdef V2 //WIFI Kit series V1 not support Vext control
	#define RADIO_DIO_1    35   // GPIO35 -- SX127x's IRQ(Interrupt Request) V2
#else
	#define RADIO_DIO_1    33   // GPIO33 -- SX127x's IRQ(Interrupt Request) V1
#endif

//#define Vext  21



#endif // __BOARD_CONFIG_H__
