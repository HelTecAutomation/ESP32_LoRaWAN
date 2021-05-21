#ifndef McuSet_H
#define McuSet_H

#include <Arduino.h>
#include "SPI.h"
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include "driver/rtc_io.h"
#include "timer.h"
#include "rtc-board.h"
#include "board-config.h"
//#include "LoRaMac.h"
#include "esp_spi_flash.h"
#include "esp_partition.h"

#define LORA_DEFAULT_NSS_PIN    18
#define LORA_DEFAULT_RESET_PIN  14
#define LORA_DEFAULT_DIO0_PIN   26
#define LORA_DEFAULT_DIO1_PIN   33
#define Timer_DEFAULT_DIV       80
extern uint8_t mcuStarted;
class McuClass{
public:
  McuClass();
  void calrtc();
  void init(int nss, int reset, int dio0, int dio1,uint32_t * codeid);
  void setSPIFrequency(uint32_t frequency);
  void sleep(uint8_t CLASS,uint8_t debuglevel);
  void setTimerDiv(uint16_t div);
  uint8_t readRegister(uint16_t address);
  void writeRegister(uint16_t address, uint8_t value);
  uint8_t singleTransfer(uint16_t address, uint8_t value);
  void writefifo0(uint16_t address, uint8_t *buffer, uint8_t size);
  void readfifo0(uint16_t address, uint8_t *buffer, uint8_t size);

private:
  SPISettings _spiSettings;
  int _nss;
  int _reset;
  int _dio0;
  int _dio1;
  uint16_t _div;
};
extern TimerEvent_t TxNextPacketTimer;

extern bool factory_test;
#ifdef __cplusplus
extern "C" void write0(uint16_t address, uint8_t value);
extern "C" uint8_t read0(uint16_t address);
extern "C" void lora_printf(const char *format, ...);
extern "C" void writefifo(uint16_t address, uint8_t *buffer, uint8_t size);
extern "C" void readfifo(uint16_t address, uint8_t *buffer, uint8_t size);
extern "C" uint64_t timercheck();
extern "C" void calRTC();
extern "C" size_t getLicenseAddress();
#endif

extern McuClass Mcu;
#endif
