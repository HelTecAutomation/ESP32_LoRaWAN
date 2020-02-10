# The Overview
Our LoRaWAN node with ESP32 chip has three working modes. 
There are class A and classB and classC, respectively, where classA is a low power mode. In classA mode, when the node does not send data, all peripherals except the crystal and the core on the board will be turned off. This greatly reduces the energy consumption of the equipment and greatly improves the endurance.

# How to install ESP32_LoRaWAN library into Arduino

## Before installing the ESP32_LoRaWAN library, we must first build the ESP32 chip in the Arduino compiler environment. This article will describe the ESP32 (Arduino) environment construction method in detail.

### 1. After installing Arduino, first find the storage address of the library in the Arduino software. In my computer, it is on the desktop → library → document → Arduino. When we find this folder we can proceed to the next step.
### 2. Construction of the ESP32 environmentFirst we locate the address of the Arduino folder above, enter Arduino to create a folder named hardware, and enter the hardware folder.
### In the pop-up command window, 

```Bash
type: git clone https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series.git #Download the ESP32 hardware information to the hardware folder.
```

### After the cloning is finished, return to the hardware folder, the files on Github are cloned locally, find the esp32→tools folder and run the get.exe file in this folder (you need to install python software), after the pop-up command window ends. Restart Arduino and you will find the options for WIFI_LoRa32 and WIFI_kit32 in the development board options under Tools.

#### At this point, the ESP32 chip is built in the compilation environment under Arduino.

### 3. Installation of ESP32_LoRa library
### Locate the Arduino→libraries folder above, press the right mouse button to select the Git Bash Here option, and in the pop-up command window,
```Bash
type：“git clone https://github.com/HelTecAutomation/ESP32_LoRaWAN.git” #put “ESP32_LoRaWAN” The library is cloned locally.
```
### Wait for the clone to complete, restart the Arduino, ESP32_LoRaWAN added to the Arduino library is complete.

## When the 1, 2, and 3 in the previous section are completed, the program can be downloaded to the ESP32 device.
### But when we need to download the ESP32_LoRaWAN program we cloned to the local, we need to obtain the license number of our company (HelTec) product and modify it at the specified location of the program.
#### How to get the license number, please refer to "How to get your license ID" at the end of the article.


## How to get your license ID

* If you purchase our company's equipment, each node device has its own serial number ID, you only need to download a program to your node device through Arduino software, it will automatically output the product ID to the serial port connected to it. Copy this product ID into our company's official website: http://www.heltec.cn/search/
Enter the product ID and click "Confirm". We will find the license serial number of your device based on this product ID.After getting this serial number, copy it to the specified location in the routine we wrote for you.After that you can download the ESP32_LoRaWAN to your node device via Arduino.         
