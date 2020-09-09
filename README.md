# Single Channel LoRaWAN Gateway

Version 6.2.6, 
Data: September 08, 2020  
Author: M. Westenberg (mw12554@hotmail.com)  
Copyright: M. Westenberg (mw12554@hotmail.com)  

All rights reserved. This program and the accompanying materials are made available under the terms 
of the MIT License which accompanies this distribution, and is available at
https://opensource.org/licenses/mit-license.php  
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Maintained by Maarten Westenberg (mw12554@hotmail.com) 

![logo](https://github.com/things4u/ESP-1ch-Gateway/blob/master/images/illustration-Maarten-In.jpg?raw=true "aap")
 
# Description

First of all: PLEASE READ THIS FILE AND [Documentation](http://things4u.github.io/Projects/SingleChannelGateway) 
it should contain most of the information you need to get going.
Unfortunately I do not have the time to follow up on all emails, and as most information including pin-outs 
etc. etc. are contained on these pages I hope you have the time to read them and post any remaining questions.

I do have more than 10 Wemos D1 mini boards running, some I built myself, 
some 10+ on Hallard, 3 on ComResult and 4 ESP32 boards. They ALL work without problems
on this code.
I did find however that good soldering joints and wiring makes all the difference,
so if you get resets/errors you cannot explain, please have a second look at your wiring.

This repository contains a proof-of-concept implementation of a single channel LoRaWAN gateway for the ESP8266. 
Starting version 5.2 also the ESP32 of TTGO (and others) is supported.
The software implements a standard LoRa gateway with the following exceptions and changes:

-  This LoRa gateway is not a full gateway but it implements just a one-channel/one frequency gateway. 
The minimum amount of frequencies supported by a full gateway is 3, most support 9 or more frequencies.
This software started as a proof-of-concept to prove that a single low-cost RRFM95 chip which was present 
in almost every LoRa node in Europe could be used as a cheap alternative to the far more expensive full 
gateways that were making use of the SX1301 chip.

- As the software of this gateway will often be used during the development phase of a project or in 
demo situations, the software is flexible and can be easily configured according to environment or 
customer requirements. There are two ways of interacting with the software: 
1. Modifying the configGway.h file at compile time allows the administrator to set almost all parameters. 
2. Using the webinterface (http://<gateway_IP>) will allow administrators to set and reset several of the 
parameters at runtime.

Full documentation of the Single Channel Gateway is found at things4u.github.io, please look at the Hardware Guide 
under the Gateway chapter.

# PlatformIO or ArduinoIDE

The source works on both environments, both the classic Arduino IDE and on PlatformIO. 
Unfortunately there are small differences between these two envrironments. 
At this moment the src directory contains the PlatformIO source, and therefore we will decribe how to connect to Arduino IDE.
The applies to the libraries.

## PlatformIO
When in PlatformIO, choose <File> and then <Add folder to Workspace...> and select your new LoRa-1ch-ESP-Gateway 
top directory.
Then just open the ESP-sc-gway.ino file at src directory and build or upload

## Arduino IDE

Create a place on you filesystem to work on the files. In this directory create the source directory "ESP-sc-gway" 
and the libraries directory "libraries". When unpacking the source at github: 
Copy the content of the "src" directory to the Aruino IDE "ESP-sc-gway" directory and copy the contents 
of the "lib" directory to the Arduino IDE "libraries" directory;

## testing

The single channel gateway has been tested on a gateway with the Wemos D1 Mini, 
using a HopeRF RFM95W transceiver.  Tests were done on 868 version of LoRa and some 
testing on 433 MHz.
The LoRa nodes tested against this gateway are:

- TeensyLC with HopeRF RFM95 radio
- Arduino Pro-Mini (default Armega328 model, 8MHz 3.3V and 16MHz 3.3V)
- ESP8266 based nodes with RFM95 transceivers.

The code has been tested on at least 8 separate gateway boards both based on the Hallard and the Comresult boards. 
I'm still working on the ESP32 pin-out and functions (expected soon).

# Getting Started

It is recommended to compile and start the single channel gateway with as little modificatons as possible. 
This means that you should use the default settings in the 2 configuration files as much as possible and 
only change the SSID/Password for your WiFi setup and the pins of your ESP8266 that you use in loraModem.h.
This section describes the minimum of configuration necessary to get a working gateway which than can be 
configured further using the webpage.

1. Unpack the source code including the libraries in a separate folder.
2. Connect the gateway to a serial port of your computer, and configure that port in the IDE. 
Switch on the Serial Monitor for the gateway. As the Wemos chip does not contain any code, 
you will probably see nothing on the Serial Monitor.
3. If necessary, modify the _loraModem.h file and change the "struct pins" area and configure 
either for a traditional (=Comresult) PCB or configure for a Hallard PCB where the dio0, 
dio1 and dio2 pins are shared. You HAVE to check this section. In the configGway.h file the 
most used pin-outs are documented so it might be that you use a standard pin-out
4. Edit the configNode.h file and adapt the "wpas" structure. Make sure that the first line of 
this structure remains empty and put the SSID and Password of your router on the second 
line of the array.
5. In the preferences part of the IDE, set the location of your sketch to the place where 
you put the sketch on your computer. This will make sure that for example the required 
libraries that are shipped with this sketch in the libraries folder can be found by the 
compiler.
6. If not yet done: Load the support for ESP8266 in your IDE. <Tools><Board><Board Manager...>
7. Load the other necessary libraries that are not shipped with this sketch in your IDE. 
Goto <Sketch><Include Library><Manage Libraries...> in the IDE to do so. Most of the include files 
can be loaded through this library section. Some cannot and are shipped with the Gateway.


- LoRaCode (Version 1.0.0, see library shipped)
- gBase64 (changed name from Adam Rudd's Base64 version)
- TinyGPS++ (Version 1.0.0)

Through Library Manager:

- ArduinoJson (version 6.12.0)
- Heltec ESP32 Dev-Boards (Version 1.0.6)
- Heltec ESL8266 Dev-Boards (Version 1.0.2)
- SPI (Version 1.0.0)
- SPIFFS (Version 1.0.0)
- Streaming (Version 5.0.0)
- Ticker (Version 1.1.0)
- Time (Version 1.5.0)
- Update (Version 1.0.0)
- Webserver (Version 1.0.0)
- WiFiClientSecure (Version 1.0.0)
- WifiEsp (Version 2.2.2)
- Wire (Version 1.0.1)
- ESP_WifiManager (Version 1.0.2 by Khoi Hoang)

8. Compile the code and download the executable over USB to the gateway. If all is right, you should
see the gateway starting up on the Serial Monitor.
9. Note the IP address that the device receives from your router. Use that IP address in a browser on 
your computer to connect to the gateway with the browser.

Now your gateway should be running. Use the webpage to set "debug" to 1 and you should be able to see packages
coming in on the Serial monitor.


# Configuration

There are two ways of changing the configuration of the single channel gateway:

1. Changing the configGway.h and the configNode.h file at compile-time
2. Run the http://<gateway-IP> web interface to change settings at run time.

Where you have a choice, option 2 is far more friendly and useful.

## Editing the configGway.h file

The configGway.h file contains the user configurable gateway settings. 
All have their definitions set through #define statements. 
In general, setting a #define to 1 will enable the function and setting it to 0 will disable it. 

Also, some settings can be initialised by setting their value with a #define but can be changed at runtime in the web interface.
For some settings, disabling the function with a #define will remove the function from the webserver as well.

NOTE regarding memory usage: The ESP8266 has an enormous amount of memory available for program space and SPIFFS filesystem. 
However the memory available for heap and variables is limited to about 80K bytes (For the ESP-32 this is higher). 
The user is advised to turn off functions not used in order to save on memory usage. 
If the heap drops below 18 KBytes some functions may not behave as expected (in extreme case the program may crash).

## Editing the configNode.h file

The configNode.h file is used to set teh WiFi access point and the structure of known 
sensors to the 1-channel gateway.
Setting the known WiFi access points (SSID and password) must be done at compile time.

When the gateway is not just used as a gateway but also for debugging purposes, the used can specify not only
the name of the sensor node but also decrypt for certain nodes the message.

### Setting USB

The user can determine whether or not the USB console is used for output messages.
When setting _DUSB to 0 all output by Serial is disabled 
(actually the Serial statements are not included in the code).

 \#define _DUSB 1

### Selecting Class mode of operation

Define the class of operation that is supported by the gateway. 
Class A is supported and contains the basic operation for battery sensors. 

Class B contains the beacon/battery mode of operation. The Gateway will send ou a beacon to the 
connected sensors which enables them to synchronize downlink messagaging.

Class C (Continuous) mode of operation contains support for devices that are probably NOT battery 
operated and will listen to the network at all times. As a result the latency of these devices 
is also shorter than for class A devices.
Class C devices are not dependent on battery power and will extend the receive windows
until the next transmission window. In fact, only transmissions will make the device abort 
listening as long as this tranmission lasts. 
Class C devices cannot do Class B operation.

 \#define _CLASS "A"
 
 All devices will start as class A devices, and may decide to "upgrade" to class B or C.
Also the gateway may or may not support Class B, which is a superset of class A.
 NOTE: Only class A is supported
 
### Selecting you standard pin-out

We support five pin-out configurations out-of-the-box, see below.
If you use one of these, just set the parameter to the right value.
If your pin definitions are different, update the loraModem.h and oLED.h file to reflect these settings.
	1: HALLARD
	2: COMRESULT pin out
	3: ESP32/Wemos based board
	4: ESP32/TTGO based ESP32 boarda
	5: ESP32/Heltec Wifi LoRA 32(V2)

 \#define _PIN_OUT 1


### Forcing a SPIFF format at startup

The following parameter shoudl be set to 0 under normal circumstances.
It does allow the system to foce formatting of the SPIFFS filesystem.

 \#define SPIFF_FORMAT 0  
 
### Setting Spreading Factor

Set the _SPREADING factor to the desired SF7, SF8 - SF12 value. 
Please note that this value is closely related to the value used for _CAD. 
If _CAD is enabled, the value of _SPREADING is not used by the gateway as it has all sreading factors enabled.

 \#define _SPREADING SF9
 
Please note that the default frequency used is 868.1 MHz which can be changed in the loraModem.h file. 
The user is advised NOT to change this etting and only use the default 868.1 MHz frequency.


### Channel Activity Detection

Channel Activity Detection (CAD) is a function of the LoRa RFM95 chip to detect incoming messages (activity). 
These incoming messages might arrive on any of the well know spreading factors SF7-SF12. 
By enabling CAD, the gateway can receive messages of any of the spreading factors.

Actually it is used in normal operation to tell the receiver that another signal is using the 
channel already.

The CAD functionality comes at a (little) price: The chip will not be able to receive very weak signals as 
the CAD function will use the RSSI register setting of the chip to determine whether or not it received a 
signal (or just noise). As a result, very weak signals are not received which means that the range of the 
gateway will be reduced in CAD mode.

 \#define _CAD 1

 
### Over the Air Updates (OTA)

As from version 4.0.6 the gateway allows over the air updating if the setting A_OTA is on. 
The over the air software requires once setting of the 4.0.6 version over USB to the gateway,
after which the software is (default) enabled for use.

The first release only supports OTA function using the IDE which in practice means the IDE has to 
be on the same network segment as the gateway.

Note: You have to use Bonjour software (Apple) on your network somewhere. A version is available
for most platforms (shipped with iTunes for windows for example). The Bonjour software enables the
gateway to use mDNS to resolve the gateway ID set by OTA after which download ports show up in the IDE.

Todo: The OTA software has not (yet) been tested in conjuction with the WiFiManager software.

 \#define A_OTA 1  



### Enable Webserver

This setting enables the webserver. Although the webserver itself takes a lot of memory, it greatly helps 
to configure the gatewayat run-time and inspects its behaviour. It also provides statistics of last messages received.
The A_REFRESH parameter defines whether the webserver should renew every X seconds.

 \#define A_SERVER 1				// Define local WebServer only if this define is set  
 \#define A_REFRESH 1				// is the webserver enabled to refresh yes/no?  (yes is OK)
 \#define A_SERVERPORT 80			// local webserver port  
 \#define A_MAXBUFSIZE 192			// Must be larger than 128, but small enough to work  

 The A_REFRESH parameter defines whether or not we can set the refresh yes/no setting in the webbrowser. 
 The setting in the webbrowser is normally put on "no" as a default, but we can leave the define on
 "1" to enabled that setting in the webbrowser.
 
### Strict LoRa behaviour

In order to have the gateway send downlink messages on the pre-set spreading factor and 
on the default frequency, you have to set the _STRICT_1CH parameter to 1. Note that when 
it is not set to 1, the gateway will respond to downlink requests with the frequency 
and spreading factor set by the backend server. And at the moment TTN responds to downlink 
messages for SF9-SF12 in the RX2 timeslot and with frequency 869.525MHz and on SF12 
(according to the LoRa standard when sending in the RX2 timeslot). 

 \#define _STRICT_1CH 0

You are advised not to change the default setting of this parameter.

### Enable OLED panel

By setting the OLED you configure the system to work with OLED panels over I2C.
Some panels work by both SPI and I2C where I2c is solwer. However, since SPI is use for RFM95 transceiver 
communication you are stronly discouvared using one of these as they will not work with this software.
Instead choose a OLED solution that works over I2C.

 \#define OLED 1

The following values are defined for OLED:
1. 0.9 inch OLED screen for I2C bus
2. 1.1 inch OLED screen for I2C bus

### Define to gather statistics

When this is defined (==1) we will gather the statistics of every message and output
it to the SPIFFS filesystem. We make sure that we use a number of files with each a fixed number of records
for statistics. The REC number tells us how many records are allowed in each statistics file.
As soon as the REC number is higher than the number of records allowed, we open a new file.
Once the number of files exceeds the NUM amount of statistics files, we delete the oldeest file and 
open a new file.
When selecting the "log" button on top of the GUI screen, all rthe log files are ouptu to the USB
Serial device. This way, we can examine far more records than fitting the GUI screen or the Serial
output.
 
 \#define STAT_LOG 1
 

 
Setting the I2C SDA/SCL pins is done in the configGway.h file right after the #define of OLED.
Standard the ESP8266 uses pins D1 and D2 for the I2C bus SCL and SDA lines but these can be changed by the user.
Normally thsi is not necessary. 
The OLED functions are found in the _loraModem.ino file, and can be adapted to show other fields. 
The functions are called when a message is received(!) and therefore potentionally this will add to the
instability of the ESP as these functions may require more time than expected.
If so, swithc off the OLED function or build in a function in the main loop() that displays in user time 
(not interrupt).

### Setting TTN server

The gateway allows to connect to 2 servers at the same time (as most LoRa gateways do BTW). 
You have to connect to at least one standard LoRa router, in case you use The Things Network (TTN) 
than make sure that you set:

 \#define _TTNSERVER "router.eu.thethings.network"  
 \#define _TTNPORT 1700  
  
In case you setup your own server, you can specify as follows using your own router URL and your own port:

 \#define _THINGSERVER "your_server.com"			// Server URL of the LoRa udp.js server program  
 \#define _THINGPORT 1701							// Your UDP server should listen to this port  

 
### Gateway Identity
Set the identity parameters for your gateway:   

\#define _DESCRIPTION "ESP-Gateway"  
\#define _EMAIL "your.email@provider.com"  
\#define _PLATFORM "ESP8266"  
\#define _LAT 52.00  
\#define _LON 5.00  
\#define _ALT 0  


### Using the gateway as a sensor node

It is possible to use the gateway as a node. This way, local/internal sensor values are reported.
This is a cpu and memory intensive function as making a sensor message involves EAS and CMAC functions.

 \#define GATEWAYNODE 0  

Further below in the configNode.h configuration file, it is possible to set the address 
and other LoRa information of the gateway node.


### Connect to WiFi with WiFiManager

The easiest way to configure the Gateway on WiFi is by using the WiFimanager function. This function works out of the box. 
WiFiManager will put the gateway in accesspoint mode so that you can connect to it as a WiFi accesspoint. 

 \#define _WIFIMANAGER 0  

If Wifi Manager is enabled, make sure to define the name of the accesspoint if the gateway is in accesspoint 
mode and the password.

 \#define AP_NAME "ESP8266-Gway-Things4U"  
 \#define AP_PASSWD "ttnAutoPw"  

The standard access point name used by the gateway is "ESP8266 Gway" and its password is "ttnAutoPw". 
After binding to the access point with your mobile phone or computer, go to htp://192.168.4.1 in a browser and tell the gateway to which WiFi network you want it to connect, and specify the password.

The gateway will then reset and bind to the given network. If all goes well you are now set and the ESP8266 will remember the network that it must connect to. NOTE: As long as the accesspoint that the gateway is bound to is present, the gateway will not any longer work with the wpa list of known access points.
If necessary, you can delete the current access point in the webserver and power cycle the gateway to force it to read the wpa array again.

## Editing the configNode.h file

### Specify the gateway node data (as with T-beam)

 \#if GATEWAYNODE==1  
 \#define _DEVADDR { 0x26, 0x01, 0x15, 0x3D }  
 \#define _APPSKEY { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  
 \#define _NWKSKEY { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  
 \#define _SENSOR_INTERVAL 300  
 \#endif 
 
### Specify a name for known nodes in configNode.h

- In configNode.h It is possible to substitue the address for known nodes with a chosen name. 
This will greatly enhance the readibility of the statistics overview 
especially for your own nodes, Now you will find names for your own nodes in the webserver.
- set the TRUSTED_NODES to either 0 (no names), 1 (specify names for known nodes) and 2 
(Do not show or transfer to TTN other than known nodes)
- Although this will work with OTAA nodes as well, please remind that OTAA nodes will change
their LORA id with every reboot. So for these nodes this function does not add much value.

### Other Settings configNode.h

- static char *wpa[WPASIZE][2] contains the array of known WiFi access points the Gateway will connect to.
Make sure that the dimensions of the array are correctly defined in the WPASIZE settings. 
Note: When the WiFiManager software is enabled (it is by default) there must at least be one entry in the wpa file, wpa[0] is used for storing WiFiManager information.
- Only the sx1276 (and HopeRF 95) radio modules are supported at this time. The sx1272 code should be 
working without much work, but as I do not have one of these modules available I cannot test this.



## Webserver

The built-in webserver can be used to display status and debugging information. 
Also the webserver allows the user to change certain settings at run-time such as the debug level or switch on 
and off the CAD function.
It can be accessed with the following URL: http://<YourGatewayIP>:80 where <YourGatewayIP> is the IP given by 
the router to the ESP8266 at startup. It is probably something like 192.168.1.XX
The webserver shows various configuration settings as well as providing functions to set parameters.

The following parameters can be set using the webServer. 
- Debug Level (0-4)
- CAD mode on or off (STD mode)
- Switch frequency hopping on and off (Set to OFF)
- When frequency Hopping is off: Select the frequency the gateway will work with. 
NOTE: Frequency hopping is experimental and does not work correctly.
- When CAD mode is off: Select the Spreading Factor (SF) the gateway will work with


# Dependencies

The software is dependent on several pieces of software, the Arduino IDE for ESP8266 
being the most important. Several other libraries are also used by this program, 
make sure you install those libraries with the IDE:

- gBase64 library, The gBase library is actually a base64 library made 
	by Adam Rudd (url=https://github.com/adamvr/arduino-base64). I changed the name because I had
	another base64 library installed on my system and they did not coexist well.
- Time library (http://playground.arduino.cc/code/time)
- Arduino JSON; Needed to decode downstream messages
- SimpleTimer; ot yet used, but reserved for interrupt and timing
- WiFiManager
- ESP8266 Web Server
- Streaming library, used in the wwwServer part
- AES library (taken from ideetron.nl) for downstream messages
- Time

For convenience, the libraries are also found in this github repository in the libraries directory. 
Please note that they are NOT part of the ESP 1channel gateway and may have their own licensing.
However, these libraries are not part of the single-channel Gateway software.


# Pin Connections

See http://things4u.github.io in the hardware section for building and connection instructions.

# Dependencies

The following dependencies are valid for the Single Channel gateway:
- ArduinoJson 6, version 6.10.0 of Benoit Blanchon
- gBase64 library, adapted by me to work in the expected way

# To-DO

The following things are still on my wish list to make to the single channel gateway:  

- Receive downstream message with commands from the server. These can be used to configure
  the gateway through downlink messages (such as setting the SF)
- Support for ESP32 and RFM95 on 433 MHz (seems to work now)
- Display for each node the last time it was seen
- Use the SPIFFS for storing .css files
- Look at Class B and C support


# License

The source files of the gateway sketch in this repository is made available under the MIT
license. The libraries included in this repository are included for convenience only and all have their own license, 
and are not part of the ESP 1ch gateway code.
