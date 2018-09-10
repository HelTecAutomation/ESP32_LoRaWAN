## Contents
- [The Overview](#the-overview)
- [Installing](#installing)
- [Features](#features)
- [Test information](#test-information)
- [API](#api)
- [How to use this library](#how-to-use-this-library)
- [Contact us](#contact-us)

# The Overview
----------
- This library is make LoRaWAN 1.0.2 protocol running with ESP32. Only support the ESP32 + LoRa series products made by HelTec Automation(TM), and a LoRa gateway is must needed.
- You may need change some definition in "Commissioning.h" ,"LoRaMac-definitions.h", "board.h"...
- This library is transplanted form https://github.com/Lora-net/LoRaMac-node
- Use RTC and deep sleep mode supported, only few millisecond wake up in a cycle.


# Installing
----------
To install this library:

- install it using the Arduino Library manager ("Sketch" -> "Include Library" -> "Manage Libraries..."), or
- download a zipfile from github using the "Download ZIP" button and install it using the IDE ("Sketch" -> "Include Library" -> "Add .ZIP Library..."
- clone this git repository into your sketchbook/libraries folder.

For more info, see https://www.arduino.cc/en/Guide/Libraries

# Features
----------
The ESP32_LoRaWAN library provides a fairly complete LoRaWAN Class A and Class C implementation, supporting the EU-433 CN-470 EU-868 and US-915 bands. Only a limited number of features was tested using this port on Heltec hardware, so be careful when using any of the untested features.

The following functions included:
 - LoRaWAN protocol V1.0.2 Class A and Class C;
 - Use RTC and 32.768KHz external crystal oscillator(it means WIFI LoRa 32 old version(no V2 LOGO on board) can't use this library, for the old boards, another ESP32 + LoRaWAN library use RCC avilabe here: xxxxxxxxxxx coming soon );
 - Support deepsleep and stop mode;
 - An unique license ralate to Chip ID is needed, you can check your license here: http://www.heltec.cn/search/

# Test information
----------
![](https://github.com/solotaker/Picture/blob/master/TestHardware.jpg)
 - Gateway: [HT-M01 Mini LoRa Gateway](http://www.heltec.cn/project/ht-m01-lora-gateway/?lang=en) + Raspberry Pi Zero W
 - Node: WIFI LoRa 32 V2
 - LoRaServer: Aliyun ECS + Ubuntu 16.04 + [loraserver.io](https://www.loraserver.io/)
 - Arudino 18.04

| working band | status |
| :----------------: | :------------:|
| EU_433 | not test |
| CN_470_510 | work well |
| EU_863_870 | Work with bug |
| US_902_928 | not test |

What certainly works:
 - Sending packets uplink, taking into account duty cycling.
 - Encryption and message integrity checking;
 - Over-the-air activation (OTAA / joining);
 - Deep sleep and wake up；
 - Class A operation.

What has not been tested:
 - Receiving downlink packets in the RX2 window.
 - Class B operation.
 - Class C operation.
 - ABP mode.
 
What are the known bugs:
 - EU_863_870: if one upload missed, resend will spend a very long time, or can't resend again.

# API
Coming soon.

# How to use this library
The only different with a common Arduino library is need a uniqu license. It's ralate to ESP32 Chip ID.

## How to get your board's Chip ID?
 - Use this simple example to read your Chip ID: https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/tree/master/esp32/libraries/ESP32/examples/ChipID/GetChipID

## How to get the unique license?
 - http://www.heltec.cn/search/ Open this page and input your ChipID

Only the boards made after August 2018 can be checked from this page, if you have a former version, you can mail to support@heltec.cn and improve you already have a HelTec Automation made board.


# Contact us
- [http://www.heltec.cn/](http://www.heltec.cn/)
- [support mail](mailto:support@heltec.cn)

# Issue/Bug report template
Before reporting an issue, make sure you've searched for similar one that was already created. Also make sure to go through all the issues labelled as [for reference](https://github.com/HelTecAutomation/ESP32_LoRaWAN/issues).     
  
