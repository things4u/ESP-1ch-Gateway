// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2020 Maarten Westenberg version for ESP mcu's
//
// 	based on work done by Thomas Telkamp for Raspberry PI 1ch gateway
//	and many others.
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
// too much code compiled and loaded on your ESP8266.
//
// ----------------------------------------------------------------------------------------

// This file describes the includes necessary for ESP Filesystem.
// At this moment there is only one record written to the ESP8266
// filesystem. We can add more info, which makes the gateway even more usable,
// however for large data we should only append to the existing file used.
// This also means we'll have to check for available space so we won't run out of 
// storage space to quickly.
// One way would be to use let's say 10 files of each 10000 lines and when full
// delete the first file and start writing on a new one (for example)



//
// Define Pattern debug settings, this allows debugging per
// module rather than per level. See also pdebug.
//
// P_SCAN statemachine
// P_CAD
// P_RX received actions only
// P_TX transmissions
// P_MAIN for main program setup() and loop()
// P_GUI for www server GUI related
// P_PRE for statemachine
// P_RADIO; For code in _loraModem and _sensor
#define P_SCAN		0x01
#define P_CAD		0x02
#define P_RX		0x04
#define P_TX		0x08
#define P_PRE		0x10
#define P_MAIN		0x20
#define P_GUI		0x40
#define P_RADIO		0x80


// Definition of the configuration record that is read at startup and written
// when settings are changed.

struct espGwayConfig {

	int32_t txDelay;			// Init 0 at setup
	uint32_t ntpErrTime;		// Record the time of the last NTP error

	uint16_t fcnt;				// =0 as init value	XXX Could be 32 bit in size
	uint16_t boots;				// Number of restarts made by the gateway after reset
	uint16_t resets;			// Number of statistics resets
	uint16_t views;				// Number of sendWebPage() calls
	uint16_t wifis;				// Number of WiFi Setups
	uint16_t reents;			// Number of re-entrant interrupt handler calls
	uint16_t ntps;
	uint16_t logFileRec;		// Logging File Record number
	uint16_t logFileNo;			// Logging File Number
	uint16_t formatCntr;		// Count the number of formats

	uint16_t ntpErr;			// Number of UTP requests that failed
	uint16_t waitErr;			// Number of times the loraWait() call failed
	uint16_t waitOk;			// Number of times the loraWait() call success
	
	uint8_t ch;					// index to freqs array, freqs[gwayConfig.ch]=868100000 default
	uint8_t sf;					// range from SF7 to SF12
	uint8_t debug;				// range 0 to 4
	uint8_t pdebug;				// pattern debug, 
	uint8_t trusted;			// pattern debug,
	
	uint8_t	maxSeen;			// Max Seen lines on GUI (not saved for reboots)
	uint8_t maxMoni;			// Max Monitoring lines	(not saved)
	uint8_t maxStat;			// Max history lines (not saved)
	
	bool cad;					// is CAD enabled?
	bool hop;					// Is HOP enabled (Note: default be disabled)
	bool isNode;				// Is gateway node enabled
	bool refresh;				// Is WWW browser refresh enabled
	bool seen;
	bool expert;
	bool monitor;
	bool showdata;				// If set, code and deconde know data
	bool dusbStat;				// Status of _DUSB
	
} gwayConfig;

// Define a log record to be written to the log file
// Keep logfiles SHORT in name! to save memory
#if _STAT_LOG == 1

// We do keep admin of logfiles by number
// 
#define LOGFILEMAX 10
#define LOGFILEREC 100

#endif //_STAT_LOG

// Define the node list structure
//
#define nSF6	0x01
#define nSF7	0x02
#define nSF8	0x04
#define nSF9	0x08
#define nSF10	0x10
#define nSF11	0x20
#define nSF12	0x40
#define nFSK	0x80

// define the Seen functon as when we have seen seen nodes last time
struct nodeSeen {
	time_t timSeen;
	uint8_t	upDown;
	uint32_t idSeen;
	uint32_t cntSeen;
	uint8_t chnSeen;
	uint8_t sfSeen;				// Encode the SF seen.This might differ per message!
};
struct nodeSeen * listSeen;


// define the logging structure used for printout of error and warning messages
// We use a string for these lines (only time) as it is convenient.
struct moniLine {
	String txt;
};
struct moniLine monitor[_MAXMONITOR];

