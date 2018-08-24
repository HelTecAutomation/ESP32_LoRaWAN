/*
/ _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
\____ \| ___ |    (_   _) ___ |/ ___)  _ \
  _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
(C)2013 Semtech

Description: SX1276 driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "radio.h"
#include "sx1276.h"
#include "sx1276-board.h"

/*!
* Flag used to set the RF switch control pins in low power mode when the radio is not active.
*/
static bool RadioIsActive = false;

/*!
* Radio driver structure initialization
*/
const struct Radio_s Radio =
{
  SX1276Init,
  SX1276GetStatus,
  SX1276SetModem,
  SX1276SetChannel,
  SX1276IsChannelFree,
  SX1276Random,
  SX1276SetRxConfig,
  SX1276SetTxConfig,
  SX1276CheckRfFrequency,
  SX1276GetTimeOnAir,
  SX1276Send,
  SX1276SetSleep,
  SX1276SetStby,
  SX1276SetRx,
  SX1276StartCad,
  SX1276SetTxContinuousWave,
  SX1276ReadRssi,
  SX1276Write,
  SX1276Read,
  SX1276WriteBuffer,
  SX1276ReadBuffer,
  SX1276SetMaxPayloadLength,
  SX1276SetPublicNetwork
};

void SX1276IoInit( void )
{

}

void SX1276IoIrqInit( DioIrqHandler **irqHandlers )
{

}

void SX1276IoDeInit( void )
{

}

void SX1276SetRfTxPower( int8_t power )
{
  uint8_t paConfig = 0;
  uint8_t paDac = 0;
  
  paConfig = SX1276Read( REG_PACONFIG );
  paDac = SX1276Read( REG_PADAC );
  
  paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1276GetPaSelect( SX1276.Settings.Channel );
  paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK ) | 0x70;
  
  if( ( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
  {
    if( power > 17 )
    {
      paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
    }
    else
    {
      paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
    }
    if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
    {
      if( power < 5 )
      {
        power = 5;
      }
      if( power > 20 )
      {
        power = 20;
      }
      paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
    }
    else
    {
      if( power < 2 )
      {
        power = 2;
      }
      if( power > 17 )
      {
        power = 17;
      }
      paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
    }
  }
  else
  {
    if( power < -1 )
    {
      power = -1;
    }
    if( power > 14 )
    {
      power = 14;
    }
    paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power + 1 ) & 0x0F );
  }
  SX1276Write( REG_PACONFIG, paConfig );
  SX1276Write( REG_PADAC, paDac );
}

uint8_t SX1276GetPaSelect( uint32_t channel )
{
  if( channel < RF_MID_BAND_THRESH )
  {
    return RF_PACONFIG_PASELECT_PABOOST;
  }
  else
  {
    return RF_PACONFIG_PASELECT_RFO;
  }
}

void SX1276SetAntSwLowPower( bool status )
{
  if( RadioIsActive != status )
  {
    RadioIsActive = status;
    
    if( status == false )
    {
      SX1276AntSwInit( );
    }
    else
    {
      SX1276AntSwDeInit( );
    }
  }
}

void SX1276AntSwInit( void )
{
  //GpioInit( &rftrx1, RF_RXTX1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
  //GpioInit( &rftrx2, RF_RXTX2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
}

void SX1276AntSwDeInit( void )
{
  //GpioInit( &rftrx1, RF_RXTX1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
  //GpioInit( &rftrx2, RF_RXTX2, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 ); 
}

void SX1276SetAntSw( uint8_t opMode )
{
  switch( opMode )
  {
  case RFLR_OPMODE_TRANSMITTER:
 //   GpioWrite( &rftrx1, 1 );
  //  GpioWrite( &rftrx2, 0 );
    break;
  case RFLR_OPMODE_RECEIVER:
  case RFLR_OPMODE_RECEIVER_SINGLE:
  case RFLR_OPMODE_CAD:
  default:
  //  GpioWrite( &rftrx1, 0 );
  //  GpioWrite( &rftrx2, 1 );
    break;
  }
}

bool SX1276CheckRfFrequency( uint32_t frequency )
{
  // Implement check. Currently all frequencies are supported
  return true;
}
