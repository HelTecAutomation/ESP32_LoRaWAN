# The Overview
- ESP32_LoRaWAN arduino development environment installation guide under Winows operating platform
- First, make sure you have the latest Arduino IDE installed on your computer. If it is not installed, please download and install it at https://www.arduino.cc/en/Main/Software;

## Contents
- [Installation](#installation)
- [Important](#important)
- [How to get your license ID](#how-to-get-your-license-id)
- [Issue/Bug report template](#issuebug-report-template)
- [Board Picture](#board-picture)
- [Contact us](#contact-us)

## Installation
### Installation of ESP32hardware into Arduino

Before installing the ESP32_LoRaWAN library, we must first build the ESP32 chip in the Arduino compiler environment. This article will describe the ESP32 (Arduino) environment construction method in detail.

1. After installing Arduino, first find the storage address of the library in the Arduino software. In my computer, it is on the desktop → library → document → Arduino.
![](https://github.com/solotaker/Picture/blob/master/Arduino_folder.png)
- When we find this folder we can proceed to the next step.
2. Construction of the ESP32 environmentFirst we locate the address of the Arduino folder above, enter Arduino to create a folder named hardware, and enter the hardware folder.Press the right mouse button to select the Git Bash Here option
In the pop-up command window, 

```Bash
type: git clone https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series.git #Download the ESP32 hardware information to the hardware folder.
```

![](https://github.com/solotaker/Picture/blob/master/hardware.png)

3. After the cloning is finished, return to the hardware folder, the files on Github are cloned locally, find the esp32→tools folder and run the get.exe file in this folder (you need to install python software), after the pop-up command window ends. Restart Arduino and you will find the options for WIFI_LoRa32 and WIFI_kit32 in the development board options under Tools.

![](https://github.com/solotaker/Picture/blob/master/Get.exe.png)
* At this point, the ESP32 chip is built in the compilation environment under Arduino.
![](https://github.com/solotaker/Picture/blob/master/ESP32.png)

### Installation of ESP32_LoRa library into Arduino
Locate the Arduino→libraries folder above, press the right mouse button to select the Git Bash Here option, and in the pop-up command window,
```Bash
type：“git clone https://github.com/HelTecAutomation/ESP32_LoRaWAN.git” #put “ESP32_LoRaWAN” The library is cloned locally.
```
![](https://github.com/solotaker/Picture/blob/master/LoRaWAN.png)

Wait for the clone to complete, restart the Arduino, ESP32_LoRaWAN added to the Arduino library is complete.


## Important
When we want to use the ESP32 program in the node, we still need some steps.
* We need to get the product ID and use it to get the license ID through the official website query.[Heltec](http://www.heltec.cn/)
* How to get the product ID and license ID, please refer to "How to get your license ID" at the end of the article.


### How to get your license ID

* If you purchase our company's equipment, each node device has its own serial number ID, you only need to download a program to your node device through Arduino software, it will automatically output the product ID to the serial port connected to it.

![](https://github.com/solotaker/Picture/blob/master/ChipID.png)
* We will find the license serial number of your device based on this product ID.
![](https://github.com/solotaker/Picture/blob/master/Get_product_ID.png)
* Copy this product ID into our company's official website: [Heltec](http://www.heltec.cn/search/)
* Enter the product ID and click "Confirm". 
![](https://github.com/solotaker/Picture/blob/master/Get_your_license.png)
![](https://github.com/solotaker/Picture/blob/master/License_number.png)
* After getting this license number, copy it to the specified location in the routine we wrote for you.        
![](https://github.com/solotaker/Picture/blob/master/Enter_your_license_ID.png)
### After completing all the steps, you can use the ESP32_LoRaWAN program on your node device.

# Contact us
- [http://www.heltec.cn/](http://www.heltec.cn/)
# Issue/Bug report template
Before reporting an issue, make sure you've searched for similar one that was already created. Also make sure to go through all the issues labelled as [for reference](https://github.com/HelTecAutomation/ESP32_LoRaWAN/issues).     
# Board Picture
![](https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/blob/master/InstallGuide/win-screenshots/WIFI_LoRa_32.png)  
  
