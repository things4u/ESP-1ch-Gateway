// 1-channel LoRa Gateway for ESP32 and ESP8266
// Copyright (c) Maarten Westenberg 2016-2020 

#define VERSION "V.6.2.6.EU868; PlatformIO 200908 d"

//
// Based on work done by Thomas Telkamp for Raspberry PI 1ch gateway and many others.
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License
// which accompanies this distribution, and is available at
// https://opensource.org/licenses/mit-license.php
//
// NO WARRANTY OF ANY KIND IS PROVIDED
//
// Author: Maarten Westenberg (mw12554@hotmail.com)
//
// This file contains a number of compile-time settings that can be set on (=1) or off (=0)
// The disadvantage of compile time is minor compared to the memory gain of not having
// too much code compiled and loaded on your ESP device.
//
// NOTE: 
// If version is for ESP32 Heltec board, compile with ESP32 setting and board 
// "ESP32 Dev Module" or "Heltec WiFi Lora 32"
// 
// For ESP8266 Wemos: compile with "Wemos R1 D1" and choose
// the right _PIN_OUT below. Selecting Oled while that is not connected does not 
// really matter (so you can leave that in).
//
// The source has been optimized for PlatformIO usage. That means some variables
// can be defined in the .platformio.ini file. Please look at the example file to see 
// how to set these #defines
//
// ========================================================================================


// Define whether we should do a formatting of SPIFFS when starting the gateway
// This is usually a good idea if the webserver is interrupted halfway a writing
// operation. Also to be used when software is upgraded
// Normally, value 0 is a good default and should not be changed.
#if !defined _SPIFFS_FORMAT
#	define _SPIFFS_FORMAT 0
#endif


// Allows configuration through WifiManager AP setup. Must be 0 or 1	
#if !defined _WIFIMANAGER				
#	define _WIFIMANAGER 0
#endif


// Debug message will be put on Serial is this one is set.
// If set to 0, no printing to USB devices is done.
// Set to 1 it will print all user level messages (with correct debug set)
// If set to 2 it will also print interrupt messages (not recommended)
#if !defined _DUSB
#	define _DUSB 1
#endif


// Define the monitor screen. When it is greater than 0 then logging is displayed in
// the special screen at the GUI.
// If _DUSB is also set to 1 then most messages will also be copied to USB devices.
#if !defined _MONITOR
#	define _MONITOR 1
#endif


// Gather statistics on sensor and Wifi status
// 0= No statistics
// 1= Keep track of messages statistics, number determined by _MAXSTAT
// 2= Option 1 + Keep track of messages received PER each SF (default)
// 3= See Option 2, but with extra channel info (Not used when Hopping is not selected)
#if !defined _STATISTICS
#	define _STATISTICS 3
#endif

// Define the frequency band the gateway will listen on. Valid options are
// EU863_870	Europe 
// US902_928	North America
// AU925_928	Australia
// CN470_510	China
// IN865_867	India
// CN779-787	(Not Used!)
// EU433		Europe
// AS923		(Not Used)
// You can find the definitions in "loraModem.h" and frequencies in
// See https://www.thethingsnetwork.org/docs/lorawan/frequency-plans.html
#define EU863_870 1
 

// Define the CLASS mode of the gateway
// A: Baseline Class
// B: Beacon/Battery Class
// C: Continuous Listen Class
#define _CLASS "A"


// Define whether to use the old Semtech gateway API, which is still supported by TTN,
// but is more lightweight than the new TTN tcp based protocol.
// NOTE: Only one of the two should be defined! TTN Router project has stopped
//
#define _UDPROUTER 1
//#define _TTNROUTER 1


#if !defined _CHANNEL
#	define _CHANNEL 0
#endif

// The spreading factor is the most important parameter to set for a single channel
// gateway. It specifies the speed/datarate in which the gateway and node communicate.
// As the name says, in principle the single channel gateway listens to one channel/frequency
// and to one spreading factor only.
// This parameters contains the default value of SF, the actual version can be set with
// the webserver and it will be stored in SPIFF
// NOTE: The frequency is set in the loraModem.h file and is default 868.100000 MHz.
#if !defined _SPREADING
#	define _SPREADING SF9
#endif

// Channel Activity Detection
// This function will scan for valid LoRa headers and determine the Spreading 
// factor accordingly. If set to 1 we will use this function which means the 
// 1-channel gateway will become even more versatile. If set to 0 we will use the
// continuous listen mode.
// Using this function means that we HAVE to use more dio pins on the RFM95/sx1276
// device and also connect enable dio1 to detect this state. 
#define _CAD 1


// CRCCHECK
// Defines whether we should check on the CRC of RXDONE messages (see stateMachine.ino)
// This should prevent us from getting a lot os stranges messgages of unknown nodes.
// Note: DIO3 must be connected for this to work (Heltec and later Wemos gateways). 
#define _CRCCHECK 1


// Definitions for the admin webserver.
// _SERVER determines whether or not the admin webpage is included in the sketch.
// Normally, leave it in!
#define _SERVER 1				// Define local WebServer only if this define is set
#define _REFRESH 1				// Allow the webserver refresh or not?
#define _SERVERPORT 80			// Local webserver port (normally 80)
#define _MAXBUFSIZE 192			// Must be larger than 128, but small enough to work


// Definitions for over the air updates. At the moment we support OTA with IDE
// Make sure that tou have installed Python version 2.7 and have Bonjour in your network.
// Bonjour is included in iTunes (which is free) and OTA is recommended to install 
// the firmware on your router witout having to be really close to the gateway and 
// connect with USB.
#if !defined _OTA
#	define _OTA 1
#endif


// We support a few pin-out configurations out-of-the-box: HALLARD, COMPRESULT and 
// Heltec/TTGO ESP32.
// If you use one of these, just set the parameter to the right value.
// If your pin definitions are different, update the loraModem.h file to reflect the
// hardware settings.
//	1: HALLARD
//	2: COMRESULT pin out
//	3: ESP32, Wemos pin out (Not used)
//	4: ESP32, Heltec and TTGO pin out (should work for Heltec, 433 and Oled too).
//	5: Other, define your own in loraModem.h (does not include GPS Code)
#if !defined _PIN_OUT
#	define _PIN_OUT 1
#endif


// Single channel gateways if they behave strict should only use one frequency 
// channel and one, or in case _CAD all, spreading factors. 
// The TTN backend replies on RX1 timeslot for spreading factors SF9-SF12. 
// If the 1ch gateway is working in and for nodes that ONLY transmit and receive on the set
// and agreed frequency and spreading factor. make sure to set STRICT to 1.
// In this case, the frequency and spreading factor for downlink messages is adapted by this
// gateway
// NOTE: If your node has only one frequency enabled and one SF, you must set this to 1
//		in order to receive downlink messages. This is the default mode.
// NOTE: In all other cases, value 0 works for most gateways with CAD enabled
#if !defined _STRICT_1CH
#	define _STRICT_1CH 1
#endif


// Extra Microseconds delay added by the Up receiver from the sensor to the server.
// As the tmst is also corrected, this will add to the downlink messages also.
// The server will use this value to compute the receive window.
#if !defined _RXDELAY1
//#	define _RXDELAY1 1000
#	define _RXDELAY1 0
#endif


//
// Also, normally the server will respond with SF12 in the RX2 timeslot.
// For TTN, the RX2 timeslot is SF9, so we should use that one for TTN
#if !defined _RX2_SF
#	define _RX2_SF 9
#endif


// This section defines whether we use the gateway as a repeater
// For his, we use another output channel as the channel (default==0) we are 
// receiving the messages on.
#if !defined _REPEATER
#	define _REPEATER 0
#endif


// Define if Oled Display is connected to I2C bus. Note that defining an Oled display does not
// impact performance negatively, certainly if no Oled is connected. Wrong Oled will not show
// sensible results on the Oled display
// _OLED==0;	No Oled display connected
// _OLED==1;	0.9" Oled Screen based on SSD1306
// _OLED==2;	1.3" Oled screens for Wemos, 128x64 SH1106
#if !defined _OLED
#	define _OLED 1
#endif


// Define whether we want to manage the gateway over UDP (next to management 
// thru webinterface).
// This will allow us to send messages over the UDP connection to manage the gateway 
// and its parameters. Sometimes the gateway is not accesible from remote, 
// in this case we would allow it to use the SERVER UDP connection to receive 
// messages as well.
// NOTE: Be aware that these messages are NOT LoRa and NOT LoRa Gateway spec compliant.
//	However that should not interfere with regular gateway operation but instead offer 
//	functions to set/reset certain parameters from remote.
#define _GATEWAYMGT 0


// Do extensive logging to file(s)
// Use the ESP8266 SPIFS filesystem to do extensive logging.
// We must take care that the filesystem never(!) is full, and for that purpose we
// rather have new records/line of statistics than very old.
// Of course we must store enough records to make the filesystem work
// NOTE:
// Please do NOT USE file logging if you are concerned about downstream timing.
//	The status logging taken about 6 ms (6000 uSec) for each write whereas
//	normally this is less than 100 uSecs.
#if !defined _STAT_LOG
#	define _STAT_LOG 0
#endif


// Set the Server Settings (IMPORTANT)
#define _LOCUDPPORT 1700					// UDP port of gateway! Often 1700 or 1701 is used for upstream comms



// This defines whether or not we would use the gateway as 
// as sort of backend decoding system for local sensors which decodes (such as TTGO T-Beam)
// 1: _LOCALSERVER is used
// 0: Do not use _LOCALSERVER 
#if !defined _LOCALSERVER
#	define _LOCALSERVER 0					// See server definitions for decodes
#endif


// ntp
// Please add daylight saving time to NTP_TIMEZONES when desired
#define NTP_TIMESERVER "nl.pool.ntp.org"	// Country and region specific
#define NTP_TIMEZONES	2					// How far is our Timezone from UTC (excl daylight saving/summer time)
#define SECS_IN_HOUR	3600
#define NTP_INTR 0							// Do NTP processing with interrupts or in loop();


// lora sensor code definitions
// Defines whether the gateway will also report sensor/status value on MQTT 
// (such as battery and GPS)
// after all, a gateway can be a node to the system as well. Some sensors like GPS can be
// sent to the backend as a parameter, some (like humidity for example) can only be sent
// as a regular sensor value.
// Set its LoRa address and key below in this file, See spec. para 4.3.2
// NOTE: The node is switched off by default. Switch it on in the GUI
#if !defined _GATEWAYNODE
#	define _GATEWAYNODE 0
#endif


// We can put the gateway in such a mode that it will (only) recognize
// nodes that are put in a list of trusted nodes.
// Values:
// 0: Do not use names for trusted Nodes
// 1: Use the nodes as a translation table for hex codes to names (in TLN)
// 2: Same as 1, but is nodes NOT in the nodes list below they are NOT shown
// NOTE: We probably will make this list dynamic!
#define _TRUSTED_NODES 1
#define _TRUSTED_DECODE 1


// ========================================================================
// DO NOT CHANGE BELOW THIS LINE
// Probably do not change items below this line, only if lists or 
// configurations on configNode.h are not large enough for example.
// ========================================================================


// Name of he configfile in SPIFFs	filesystem
// In this file we store the configuration and other relevant info that should
// survive a reboot of the gateway		
#define _CONFIGFILE "/gwayConfig.txt"


// Maximum number of Message History statistics records gathered. 20 is a good maximum 
// (memory intensive). For ESP32 maybe 30 could be used as well
#if !defined _MAXSTAT
#	define _MAXSTAT 20
#endif


// We will log a list of LoRa nodes that was forwarded using this gateway.
// For eacht node we record:
//	- node Number, or known node name
//	- Last seen 'seconds since 1/1/1970'
//	- SF seen (8-bit integer with SF per bit)
// The initial version _NUMMAX stores max this many nodes, please make
// _MAXSEEN==0 when not used
#if !defined _MAXSEEN
#	define _MAXSEEN 20
#endif
#define _SEENFILE "/gwaySeen.txt"


// Define the maximum amount of items we monitor on the screen
#if !defined _MAXMONITOR
#	define _MAXMONITOR 20
#endif


// Timing
#define _PULL_INTERVAL 16					// PULL_DATA messages to server to get downstream in seconds
#define _STAT_INTERVAL 120					// Send a 'stat' message to server
#define _NTP_INTERVAL 3600					// How often do we want time NTP synchronization
#define _WWW_INTERVAL 60					// Number of seconds before we refresh the WWW page
#define _FILE_INTERVAL 30					// Number of timer (in secs) before writing to files
#define _MSG_INTERVAL 31					// Message timeout timer in seconds
#define _RST_INTERVAL 97					// Reset interval in seconds, total chip reset


// Define the correct radio type that you are using
#define CFG_sx1276_radio		
//#define CFG_sx1272_radio


// Serial Port speed
#define _BAUDRATE 115200						// Works for debug messages to serial momitor


// Will we use Mutex or not?
// +SPI is input for SPI, SPO is output for SPI
#define _MUTEX 0


// MQTT definitions, these settings should be standard for TTN
// and need no changing. When _REPEATER function is selected, we do not
// use the backend function to send message to server over MQTT.
#if _REPEATER==0
#	define _TTNSERVER "router.eu.thethings.network"
#	define _TTNPORT 1700							// Standard port for TTN
#endif