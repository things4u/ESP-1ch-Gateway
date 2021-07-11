# Single Channel LoRaWAN Gateway

Version 6.2.4, February 29, 2020 
Author: M. Westenberg (mw12554@hotmail.com)  
Copyright: M. Westenberg (mw12554@hotmail.com)  

All rights reserved. This program and the accompanying materials are made available under the terms 
of the MIT License which accompanies this distribution, and is available at
https://opensource.org/licenses/mit-license.php  
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Maintained by Maarten Westenberg (mw12554@hotmail.com)



# Release Notes

Features  release 6.2.7 (October 30, 2020)
- Further repair timing
- Make interrupt handling simplier
- improve website of gateway, making data dynamic

Features release 6.2.6 (September 08, 2020)
- Better timing for downstream
- Other display of statistics

Features release 6.2.5 (April 30, 2020)
- Repaired SF and BW for upstream
- Rewrote Monitoring to output only the most significant messages for debug==1
- Make definitions of LASTSEEN and PACKAGE STATISTICS dynamic. Allowing GUI resizing. 
Look in expert mode to change these settings
- Rewrote the addLog() function
- Repaired typos
- Removed unncessary serial and MONITOR prints

Features release 6.2.4 (April 25, 2020)
- Changes the date layout used in the output to be more standard: 3 characters weekday, months and day 2 chars
- Changed the configGway.h file a lot ro define default values for parameters while at the 
	same time allowing changing through PlatformIO
- Updated the documentation itself and updated link in the software
- For Monitor output use the same stringTime() function
- Added the _PROFILER definition to read the timing (PlatformIO only)
- Adapt the documentation and start writing hardware guide (for compile options)
- Changed the delay for TX (Downlink messages) -. Removed to 0 for _STIRCT_1CH
- Index of iSeen messages was in Hex -> Changed to decimal
- Added a timing correction for sending message of aound 700000 uSec (0.7 Sec) to make downlink work
- Changed naming of tmst (TimeStamp) functions
- Removed function printlog(), as it was not used
- Moved the SPIFFS file operations to a timer function in loop() as it consumer far too much time. 
As a result regular timestamp is reduced.
- Removed the logging functions for the same reasons. User can choose to put those on again.
- Added Udp.flush() when NTP messages arrive during operations 
- Changed layout of webpage. Changed sequence to first OFF and then ON
- Changed website to include loraWait statistics

Features release 6.2.3 (February 23, 2020)
- Lots of bugs and documentation fixes
- Added customizable #define statements through platformio.ini file (Read!!!)
- Changed the WiFiManager code to better support both architectures: ESP8266 and ESP32

Features release 6.2.1 (February 2, 2020)
- PlatformIO support

Features release 6.2.0 (January 29, 2020)
- Indicate WPA and WIFI mode on OLED display
- Correct sensor totals in Webserver overview.
- Change delays for GPS and other internal sensors, and repair internal 
  sensors to make sure we do not miss sensor messages
- Changed the MONITOR part to a circular buffer to save time when filling buffer
- Statistics log redefined
- Removed variables never used (PackaIO)


Features release 6.1.8 (January 21, 2020)
- Repair wifimanager compile for ESP8266, and make it work for ESP8266
- Substitues _DUSB by _MONITOR in a lot of cases
- Repair c_str() bugs
- Make getNtpTime more predictable
- Several syntax error optimizations
- InLine documentation updated and errors corrected
- Repair Format code
- Repair WLAN Connect code, remove code to make connecting lean.
- Add a lot of output to MONITOR screen
- Repair STAT_LOG so enable running without log output
- Add '_' to front of defined names

Features release 6.1.7 (December 27, 2019)
- Repair bugs for DNS
- Enable internal T-Beam sensors to output values in "raw" format. 
See configNode.h and _sensors.ino

Features release 6.1.5 (December 20, 2019)
- Bug fix for "#define _DUSB 0"
- Added number of times a node has been called to the history records listSeen
- Fix LastSeen issues.
- Added 3 Indian frequencies as published by TTN
- Changes configGway.h to contain better info
- Added a message console on the GUI page (_define _MAXMONITOR 1)
- Changed the DNS code to allow .local domains with MDNS
- Added buttons for Expert mode, Monitor and Last Seen.
- Added code for "REBOOT" to be used from the API only *http://your_IP/REBOOT '
- Added the IP to the www server header (visible in browser).
- Removed version 5 of pi-out as it is the same as version 4.
- Added "monitor" functions. This allows to write to monitor screen on browser or use 
USB for desbugging messages.

Features release 6.1.4 (November 25, 2019)
- Compacting Code and Solve Errors
- Look at _DUSB define and add to Serial.print directive where not found
- Renewed the GPS functions, changed "Serial1" to "sGps" to avoid double definitions.
- Downloaded TinyGPS++ library. All working on my T-Beam again.

Features release 6.1.3 (November 20, 2019)
- Made changes to _TRUSTED_NODES in _wwwServer.ino to make sure only named nodes are 
displayed when the value of trusted nodes in "Gateways Settings" has value 2. 
The value of this vaiable is stored in the filesystem of the Gateway SPIFFS andwith every startup of 
the Gateway it is read so that users can access when sensors last were seen by the Gateway 
even after reboots.
Note: all messages ARE handled by gateway but not shown in the user interface "Last Seen" 
under message history.
- Used id_print in _loraFiles.ino in order to avoid _DUSB issues
- Reorganized the conigGway.h file to move items that normally do not change to the end of file.
- Repaired bugs and writing errors

Features release 6.1.1 (November 6, 2019)
- Added "last seen" for a node. An overview when each known node has last been seen by the gateway,
	This would mean that a node that does not fit in the regular history overview would still be visible
	even when it has been seen three days ago.
- Changed name of the ESP-sc-gway.h file into configGway.h and removed most privacy info. 
  This way, whis fie does need less editing and allows faster releasing. 
- Also moved sensor.h into configNode.h and devided between both configXXX.h file.
- Added the documentation for release 6.1.1 and correctd a number of typos.
- Corrected bug for ipPrint() in _wwwServer.ino
- Updated code to read and write config file at the setup() of the gateway, and more often if
	variables are updated in the web server.

Features release 6.1.0 (October 20, 2019)
- Changed name of the ESP-sc-gway.h file into configGway.h and removed most privacy info. 
  This way, whis fie does need less editing and allows faster releasing
- Removed lib from the library directory for libs that are present in the library manager of the node.
- Changed name of sensor.h file into configNode.h and added all privacy configuration info such as SSID, 
  WiFi password, node data etc.
- Display the IP of the Gateway node 5 seconds prior to passing the first message. 
So please keep on the line if you do not know your IP.
- Edit all files and make ready for publising to Github with correct version number
- Change the country/region setting of the Gateway (As a result you probably have to update most of 
  the package).
- Upon connecting over WiFi, display the address for 4 seconds before starting the Gateway function.
- Made a (not complete) list of lib info in the README.md document.
- Correct (again) some typos

New features in version 5.3.4 (March 25, 2019)
- Make use of the latest Arduino IDE available version 1.8.21
- Added new iteams to the statc function so that it now has all the Package Statistics. 
  Fields are such as: mesg_ttl, msg_ttl_0 etc. Removed reference to the old counters.
- !!! Changed Json version in the IDE to ArduinoJSON 6 in _txRx.ino
- Larger message history for ESP32 based on free heap

New features in version 5.3.3 (August 25, 2018)
- Bug Fixing SPIFFS Format in GUI
- Included a confirm dialog in RESET, BOOT and STATISTICS buttons for user to confirm
- Repaired WlanConnect issues
- Improved Downlink function. Work for SF8-SF10. Does not work reliable for SF7, SF11, SF12
- Provided documentation button o top of page
- Added expert mode button in GUI (Wifi, System and Interrupt data is only shown in expert mode)
- bug fixes and documentation

New features in version 5.3.2 (July 07, 2018)
- Support for local decoding of sensor messages received by the gateway. 
	Use #define _LOCALSERVER 1, to enable this functionality. Also specify for each node that you want
	to inspect the messages from the NwkSKey and AppSKey in the sensor.h file.
	NOTE: the heap of the ESP32 is much larger than of the ESP8266. SO please be careful not to add 
	too many features to the "old" gateway modules
- As a result reworked the sensor functions and changes such as adding/changing DevAddrm NwkSKEY 
	and AppSKey parameters to several functions
- Several in-line documentaton enhancements and typos were fixed

New features in version 5.3.1 (June 30, 2018)
- Included support for T-Beam board including on board GPS sensor (_sensor.ino). #define _GATEWAYNODE 1 will
	turn the gateway into a node as well. Remember to set the address etc in configGway.h.
- First version to explore possibilities of 433 MHz LoRa frequencies. 
	Included frequency setting in the configGway.h file
- Changes to the WiFi inplementation. The gateway does now store the SSID and password.
- 

New features in version 5.3.0 (June 20, 2018)
- Connect to both public and private routers


New features in version 5.2.1 (June 6, 2018)
- Repair the downlink functions
- Repair sersor functions
- Bufgixes

New features in version 5.2.0 (May 30, 2018)
- Enable support for ESP32 from TTGO, several code changes where ESP32 differs from ESP8266. 
	OLED is supported but NOT tested. Some hardware specific reporting functions of the WebGUI 
	do not work yet.
- Include new ESP32WebServer library for support of ESP32
- Made pin configuration definitions in Gateway.h file, and support in loraModem.h and .ino files.

New features in version 5.1.1 (May 17, 2018)
- The LOG button in the GUI now opens a txt .CSV file in the browser with loggin details.
- Improved debugging in WebGUI, not only based on debug level but also on part of the software 
	we want to debug.
- Clean up of StateMachine
- Enable filesystem formatting from the GUI

New features in version 5.1.0 (May 03, 2018)
- Improved debuggin in WebGUI, not only based on debug level but also on part of the software 
	we want to debug.
- Clean up of StateMachine

New features in version 5.0.9 (Apr 08, 2018)
- In statistics overview the option is added to specify names for known nodes (in ESP-sc-gateway.h file)
- Keep track of the amount of messages per channel (only 3 channels supported).
- Use the SPIFFS filesystem to provide log statistics of messages. Use the GUI and on the top select log. 
The log data is displayed in the USB Serial area (for the moment).
- Remove a lot of the debug==1 and debug==2 messages as they are not useful.

New features in version 5.0.8 (Mar 26, 2018)
- Simplified State machine and removed unnecessary code
- Changed the WiFi Disconnect code (bug in SDK). When WiFi.begin() is executed, 
  the previous accesspoint is not deleted. But the ESP8266 does also not connect 
  to it anymore. The workaround restarts the WiFi completely.
- Repair the bug causing the Channel setting to switch back to channel 0 when the RFM95 modem is reset 
  (after upstream message)
- Introduced the _utils.ino with Serial line utilities
- Documentation Changes
- Small bug fixes
- Removal of unused global variables

New features in version 5.0.7 (Feb 12, 2018)
- On low debug value (0) we show the time in the rx status message on 
- WlanConnect function updated

New features in version 5.0.7 (Feb 24, 2018)
- Changed WlaConnect function to not hang and give more debug info
- Made the change to display correct MAC address info

New features in version 5.0.6 (Feb 11, 2018)
- All timer functions that show lists on website etc are now based on now() en NTP, 
the realtime functions needed for LoRa messages are still based on micros() or millis()
- Change some USB debug messages
- Added to the documentation of the README.md

New Features in version 5.0.5 (Feb 2, 2018)
- Change timer functions to now() and secons instead of millis() as the latter one overflows once 
every 50 days.
- Add more debug information
- Simplified and enhanced the State Machine function

New features in version 5.0.4 (January 1, 2018)
- Cleanup of the State machine
- Separate file for oLED work, support for 1.3" SH3006 chips based oLED.
- Still not supported: Multi Frequency works, but with loss of #packages, 
  and some packages are recognizeg at the wrong frequency (but since they are so close that could happen).
- In-line documenattion cleaned up

New features in version 5.0.1 (November 18, 2017)
- Changed the state machine to run in user space only
- No Watchdog Resets anymore
- For each SF, percentage of such packages received of total packages
- OTAA and downlink work (again) although not always
- Nober of packages per hour displayed in webserver
- All Serial communication only when _DUSB==1 is defined at compile time

New features in version 4.0.9 (August 11, 2017)

- This release contains updates for memory leaks in several Gateway files
- Also changes in OLED functions

New features in version 4.0.8 (August 05, 2017)

- This release updates for memory leaks in NTP routines (see configGway.h file for NTP_INTR
- OLED support contributed by Dorijan Morelj (based on Adreas Spies' release)

New features in version 4.0.7 (July 22, 2017)

- This release contains merely updates to memory leaks and patches to avoid chip resets etc.
- The webinterface allows the user to see more parameters and has buttons to set/reset these parameters.
- By setting debug >=2, the webinterface will display more information.
- The gateway allows OTA (Over the Air) update. Please have an Apple "Bonjour" somewhere on your network 
(included in iTunes) and you will see the network port in the "Port" section of your IDE.

New features in version 4.0.4 (June 24, 2017):

- Review of the _wwwServer.ino file. Repaired some of the bugs causing crashes of the webserver.
- Updated the README.md file with more cofniguration information

New features in version 4.0.3 (June 22, 2017):

- Added CMAC functions so that the sensor functions work as expected over TTN
- Webserver prints a page in chunks now, so that memory usage is lower and more heap is left over 
for variables
- Webserver does refresh every 60 seconds
- Implemented suggested change of M. for answer to PKT_PULL_RESP
- Updated README.md to correctly displa all headers
- Several small bug fixes

New features in version 4.0.0 (January 26, 2017)):

- Implement both CAD (Channel Activity Detection) and HOP functions (HOP being experimental)
- Message history visible in web interface
- Repaired the WWW server memory leak (due to String assignments)
- Still works on one interrupt line (GPIO15), or can be configured to work with 2 interrupt lines 
  for dio0 and dio1 for two or more interrupt lines (better performance for automatic SF setting?)
- Webserver with debug level 3 and level 4 (for interrupt testing).
  dynamic setting thorugh the web interface. Level 3 and 4 will show more info
  on sf, rssi, interrupt flags etc.
- Tested on Arduino IDE 1.18.0
- See http://things4u.github.io for documentation

New features in version 3.3.0 (January 1, 2017)):

- Redesign of the Webserver interface
- Use of the SPIFFS filesystem to store SSID, Frequency, Spreading Factor and Framecounter to 
survice reboots and resets of the ESP8266
- Possibility to set the Spreading Factor dynamically throug the web interface
- Possibility to set the Frequency in the web interface
- Reset the Framecounter in te webinterface

New features in version 3.2.2 (December 29, 2016)):

- Repair the situation where WIFIMANAGER was set to 0 in the configGway.h file. 
The sketch would not compile which is now repaired
- The compiler would issue a set of warnings related to the ssid and passw setting in the configGway.h file. 
Compiler was complaining (and it should) because char* were statically initialised and modified in the code.

New features in version 3.2.1 (December 20, 2016)):

- Repair the status messages to the server. All seconds, minutes, hours etc. are now reported in 2 digits. 
The year is reported in 4 digits.

New features in version 3.2.0 (December 08, 2016)):

- Several bugfixes

New features in version 3.1 (September 29, 2016)):

- In the configGway.h it is possible to set the gateway as sensor node as well. 
	Just set the DevAddr and AppSKey in the _sensor.ino file and be able to forward any sensor or other 
	values to the server as if they were coming from a LoRa node.
- If the #define _STRICT_1CH is set (to 1) then the system will be able to send downlink messages 
	to LoRa nodes that are strict 1-channel devices (all frequencies but frequency 0 are disabled 
	and Spreading Factor (SF) is fixed to one value).
- Code clean-up. The code has been made smaller in the area of loraWait() functions and where the 
	radio is initiated for receiving of transmitting messages.
- Several small bug fixes
- Licensing, the license has been changed to MIT

New features in version 3.0 (September 27, 2016):

- WiFiManager support
- Limited SPIFFS (filesystem) support for persistent data storage
- Added functions to webserver. Webserver port now 80 by default (!)

Other

- Supports ABP nodes (TeensyLC and Arduino Pro-mini)
- Supports OTAA functions on TeensyLC and Arduino Pro-Mini (not all of them) for SF7 and SF8.
- Supports SF7, SF8. SF7 is tested for downstream communication
- Listens on configurable frequency and spreading factor
- Send status updates to server (keepalive)
- PULL_DATA messages to server
- It can forward messages to two servers at the same time (and read from them as well)
- DNS support for server lookup
- NTP Support for time sync with internet time servers
- Webserver support (default port 8080)
- .h header file for configuration

Not (yet) supported:

- SF7BW250 modulation
- FSK modulation
- RX2 timeframe messages at frequency 869,525 MHz are not (yet) supported.
- SF9-SF12 downlink messaging available but needs more testing


# License

The source files of the gateway sketch in this repository is made available under the MIT
license. The libraries included in this repository are included for convenience only and 
all have their own license, and are not part of the ESP 1ch gateway code.
