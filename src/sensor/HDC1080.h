/*

Arduino Library for Texas Instruments HDC1080 Digital Humidity and Temperature Sensor
Written by AA for ClosedCube
---

The MIT License (MIT)

Copyright (c) 2016-2017 ClosedCube Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef _HDC1080_h

#define _HDC1080_h
#include <Arduino.h>

typedef enum {
	HDC1080_RESOLUTION_8BIT,
	HDC1080_RESOLUTION_11BIT,
	HDC1080_RESOLUTION_14BIT,
} HDC1080_MeasurementResolution;

typedef enum {
	HDC1080_TEMPERATURE		= 0x00,
	HDC1080_HUMIDITY		= 0x01,
	HDC1080_CONFIGURATION	= 0x02,
	HDC1080_MANUFACTURER_ID = 0xFE,
	HDC1080_DEVICE_ID		= 0xFF,
	HDC1080_SERIAL_ID_FIRST	= 0xFB,
	HDC1080_SERIAL_ID_MID	= 0xFC,
	HDC1080_SERIAL_ID_LAST	= 0xFD,
} HDC1080_Pointers;

typedef union {
	uint8_t rawData[6];
	struct {
		uint16_t serialFirst;
		uint16_t serialMid;
		uint16_t serialLast;
	};
} HDC1080_SerialNumber;

typedef union {
	uint8_t rawData;
	struct {
		uint8_t HumidityMeasurementResolution : 2;
		uint8_t TemperatureMeasurementResolution : 1;
		uint8_t BatteryStatus : 1;
		uint8_t ModeOfAcquisition : 1;
		uint8_t Heater : 1;
		uint8_t ReservedAgain : 1;
		uint8_t SoftwareReset : 1;
	};
} HDC1080_Registers;

class HDC1080 {
public:
	HDC1080();

	void begin(uint8_t address);
	void end();
	uint16_t readManufacturerId(); // 0x5449 ID of Texas Instruments
	uint16_t readDeviceId(); // 0x1050 ID of the device

	HDC1080_Registers readRegister();	
	void writeRegister(HDC1080_Registers reg);

	HDC1080_SerialNumber readSerialNumber();

	void heatUp(uint8_t seconds);
	
	void setResolution(HDC1080_MeasurementResolution humidity, HDC1080_MeasurementResolution temperature);

	double readTemperature();
	double readHumidity();

	double readT(); // short-cut for readTemperature
	double readH(); // short-cut for readHumidity

private:
	uint8_t _address;
	uint16_t readData(uint8_t pointer);
	
};

#endif

