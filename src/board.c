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
#include "board.h"
#include "Arduino.h"


/*
* MCU objects
*/

#if defined( USE_USB_CDC )
Uart_t UartUsb;
#endif

/*!
* Initializes the unused GPIO to a know status
*/
static void BoardUnusedIoInit( void );

/*!
* System Clock Configuration
*/
static void SystemClockConfig( void );

/*!
* Used to measure and calibrate the system wake-up time from STOP mode
*/
static void CalibrateSystemWakeupTime( void );

/*!
* System Clock Re-Configuration when waking up from STOP mode
*/
static void SystemClockReConfig( void );

/*!
* Timer used at first boot to calibrate the SystemWakeupTime
*/
static TimerEvent_t CalibrateSystemWakeupTimeTimer;

/*!
* Flag to indicate if the MCU is Initialized
*/
static bool McuInitialized = false;

/*!
* Flag to indicate if the SystemWakeupTime is Calibrated
*/
static bool SystemWakeupTimeCalibrated = false;

/*!
* Callback indicating the end of the system wake-up time calibration
*/
static void OnCalibrateSystemWakeupTimeTimerEvent( void )
{
  SystemWakeupTimeCalibrated = true;
}

/*!
* Nested interrupt counter.
*
* \remark Interrupt should only be fully disabled once the value is 0
*/
static uint8_t IrqNestLevel = 0;

void BoardDisableIrq( void )
{
    noInterrupts();
    IrqNestLevel++;
}

void BoardEnableIrq( void )
{
  IrqNestLevel--;
  if( IrqNestLevel == 0 )
  {
	  interrupts();
  }
}

void BoardInitPeriph( void )
{  

}

#define FIFO_TX_SIZE      512
uint8_t UARTTxBuffer[FIFO_TX_SIZE];

#define FIFO_RX_SIZE      512
uint8_t UARTRxBuffer[FIFO_RX_SIZE];


void BoardInitMcu( void )
{
  if( McuInitialized == false )
  {

  //  BoardUnusedIoInit( );
//#warning "Commented for test!"
  }
  else
  {
    SystemClockReConfig( );
//#warning "Commented for test!"
  } 
  
#if defined( USE_DEBUGGER )  
    //uart init
    FifoInit( &Uart1.FifoRx, UARTRxBuffer, FIFO_RX_SIZE );     
    FifoInit( &Uart1.FifoTx, UARTTxBuffer, FIFO_TX_SIZE );
    Uart1.IrqNotify = loraMcuIrqNotify;    
    
    UartInit( &Uart1, UART_1, UART_TX, UART_RX );
    UartConfig( &Uart1, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
    
    UartPutBuffer(&Uart1,"Heltec lora node demo\r\n",strlen("Heltec lora node demo\r\n"));    
#endif
  
  //SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
  SX1276IoInit( );
  
  if( McuInitialized == false )
  {
    McuInitialized = true;
 //   if( GetBoardPowerSource( ) == BATTERY_POWER )
 //   {
 //     CalibrateSystemWakeupTime( );
 //   }

  }
}

void BoardDeInitMcu( void )
{
}



/*!
* Factory power supply
*/
#define FACTORY_POWER_SUPPLY                        3300 // mV

/*!
* VREF calibration value
*/
#define VREFINT_CAL                                 ( *( uint16_t* )0x1FF80078 )

/*!
* ADC maximum value
*/
#define ADC_MAX_VALUE                               4095

/*!
* Battery thresholds
*/
#define BATTERY_MAX_LEVEL                           4150 // mV
#define BATTERY_MIN_LEVEL                           3200 // mV
#define BATTERY_SHUTDOWN_LEVEL                      3100 // mV

static uint16_t BatteryVoltage = BATTERY_MAX_LEVEL;

uint16_t BoardBatteryMeasureVolage( void )
{
  //    uint16_t vdd = 0;
  //    uint16_t vref = VREFINT_CAL;
  //    uint16_t vdiv = 0;
  uint16_t batteryVoltage = 0;
  
  //    vdiv = AdcReadChannel( &Adc, BAT_LEVEL_CHANNEL );
  //    //vref = AdcReadChannel( &Adc, ADC_CHANNEL_VREFINT );
  //
  //    vdd = ( float )FACTORY_POWER_SUPPLY * ( float )VREFINT_CAL / ( float )vref;
  //    batteryVoltage = vdd * ( ( float )vdiv / ( float )ADC_MAX_VALUE );
  //
  //    //                                vDiv
  //    // Divider bridge  VBAT <-> 470k -<--|-->- 470k <-> GND => vBat = 2 * vDiv
  //    batteryVoltage = 2 * batteryVoltage;
  return batteryVoltage;
}

uint32_t BoardGetBatteryVoltage( void )
{
  return BatteryVoltage;
}

uint8_t BoardGetBatteryLevel( void )
{
  uint8_t batteryLevel = 0;
  
  BatteryVoltage = BoardBatteryMeasureVolage( );
  
  if( GetBoardPowerSource( ) == USB_POWER )
  {
    batteryLevel = 0;
  }
  else
  {
    if( BatteryVoltage >= BATTERY_MAX_LEVEL )
    {
      batteryLevel = 254;
    }
    else if( ( BatteryVoltage > BATTERY_MIN_LEVEL ) && ( BatteryVoltage < BATTERY_MAX_LEVEL ) )
    {
      batteryLevel = ( ( 253 * ( BatteryVoltage - BATTERY_MIN_LEVEL ) ) / ( BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL ) ) + 1;
    }
    else if( ( BatteryVoltage > BATTERY_SHUTDOWN_LEVEL ) && ( BatteryVoltage <= BATTERY_MIN_LEVEL ) )
    {
      batteryLevel = 1;
    }
    else //if( BatteryVoltage <= BATTERY_SHUTDOWN_LEVEL )
    {
      batteryLevel = 255;
      //GpioInit( &DcDcEnable, DC_DC_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
      //GpioInit( &BoardPowerDown, BOARD_POWER_DOWN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    }
  }
  return batteryLevel;
}

static void BoardUnusedIoInit( void )
{

#if defined( USE_DEBUGGER )
#else
#endif
}

void SystemClockConfig( void )
{
  
}

void CalibrateSystemWakeupTime( void )
{
  if( SystemWakeupTimeCalibrated == false )
  {
    TimerInit( &CalibrateSystemWakeupTimeTimer, OnCalibrateSystemWakeupTimeTimerEvent );
    TimerSetValue( &CalibrateSystemWakeupTimeTimer, 1000 );
    TimerStart( &CalibrateSystemWakeupTimeTimer );
    while( SystemWakeupTimeCalibrated == false )
    {
      TimerLowPowerHandler( );
    }
  }
}

void SystemClockReConfig( void )
{

}

void SysTick_Handler( void )
{

}

uint8_t GetBoardPowerSource( void )
{
#if defined( USE_USB_CDC )
  if( GpioRead( &UsbDetect ) == 1 )
  {
    return BATTERY_POWER;
  }
  else
  {
    return USB_POWER;
  }
#else
  return BATTERY_POWER;
#endif
}

uint32_t HexToString(/*IN*/  const char    * pHex,  
                     /*IN*/  uint32_t           hexLen,  
                     /*OUT*/ char          * pByteString)  
{  
  unsigned long i;  
  
  if (pHex==NULL)  
    return 1;  
  
  if(hexLen <= 0)  
    return 2;  
  
  for(i=0;i<hexLen;i++)  
  {  
    if(((pHex[i]&0xf0)>>4)>=0 && ((pHex[i]&0xf0)>>4)<=9)  
      pByteString[2*i]=((pHex[i]&0xf0)>>4)+0x30;  
    else if(((pHex[i]&0xf0)>>4)>=10 && ((pHex[i]&0xf0)>>4)<=16)  
      pByteString[2*i]=((pHex[i]&0xf0)>>4)+0x37;   //  小写：0x37 改为 0x57   
    
    if((pHex[i]&0x0f)>=0 && (pHex[i]&0x0f)<=9)  
      pByteString[2*i+1]=(pHex[i]&0x0f)+0x30;  
    else if((pHex[i]&0x0f)>=10 && (pHex[i]&0x0f)<=16)  
      pByteString[2*i+1]=(pHex[i]&0x0f)+0x37;      //  小写：0x37 改为 0x57   
  }  
  return 0;  
} 

#ifdef USE_FULL_ASSERT
/*
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*/
void assert_failed( uint8_t* file, uint32_t line )
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while( 1 )
  {
  }
}
#endif
