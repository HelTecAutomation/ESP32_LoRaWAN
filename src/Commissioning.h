/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2015 Semtech

Description: End device commissioning parameters

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

#ifndef __LORA_COMMISSIONING_H__
#define __LORA_COMMISSIONING_H__

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_MAX_SIZE                   128

/*!
 * Indicates if the end-device is to be connected to a private or public network
 */
#define LORAWAN_PUBLIC_NETWORK                      true

/*!
 * Current network ID
 */
#define LORAWAN_NETWORK_ID                          ( uint32_t )0

#endif // __LORA_COMMISSIONING_H__
