#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include "SPI.h"
#include "board.h"
#include "Commissioning.h"
#include "Mcu.h"
#include "Commissioning.h"

class LoRaClass{
public:
  void DeviceStateInit();
  void DeviceStateJion();
  void DeviceStateSend();
  void DeviceSleep(uint8_t isLowPowerOn,uint8_t debuglevel);
};


extern LoRaClass LoRa;


#endif
