#ifndef McuSet_H
#define McuSet_H

#include <Arduino.h>
#include "SPI.h"
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include "driver/rtc_io.h"
//#include "LoRaMac.h"
#define LORA_DEFAULT_SS_PIN     18
#define LORA_DEFAULT_RESET_PIN  14
#define LORA_DEFAULT_DIO0_PIN   26
#define LORA_DEFAULT_DIO1_PIN   35
#define Timer_DEFAULT_DIV       80
extern uint8_t LoraWanStarted;
class McuClass{
public:
  McuClass();
  int begin(int ss, int reset, int dio0, int dio1,uint32_t * codeid);
  void setSPIFrequency(uint32_t frequency);
  void sleep(uint8_t isLowPowerOn,uint8_t debuglevel);
  void setTimerDiv(uint16_t div);
  uint8_t readRegister(uint8_t address);
  void writeRegister(uint8_t address, uint8_t value);
  uint8_t singleTransfer(uint8_t address, uint8_t value);
  void writefifo0(uint8_t address, uint8_t *buffer, uint8_t size);
  void readfifo0(uint8_t address, uint8_t *buffer, uint8_t size);

private:
  SPISettings _spiSettings;
  int _ss;
  int _reset;
  int _dio0;
  int _dio1;
  uint16_t _div;
};


#ifdef __cplusplus
extern "C" void write0(uint8_t address, uint8_t value);
extern "C" uint8_t read0(uint8_t address);
extern "C" void lora_printf(const char *format, ...);
extern "C" void writefifo(uint8_t address, uint8_t *buffer, uint8_t size);
extern "C" void readfifo(uint8_t address, uint8_t *buffer, uint8_t size);
extern "C" uint64_t timercheck();
#endif

extern McuClass Mcu;
#endif
