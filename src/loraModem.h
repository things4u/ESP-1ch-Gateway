// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2020 Maarten Westenberg version for ESP8266
//
// 	based on work done by Thomas Telkamp for Raspberry PI 1ch gateway
//	and many other contributors.
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
// This file contains a number of compile-time settings and declarations that are
// specific to the LoRa rfm95, sx1276, sx1272 radio of the gateway.
//
//
// ------------------------------------------------------------------------------------


// ----------------------------------------
// Used by REG_PAYLOAD_LENGTH to set receive payload length
#define PAYLOAD_LENGTH              0x40		// 64 bytes
#define MAX_PAYLOAD_LENGTH          0x80		// 128 bytes

// In order to make the CAD behaviour dynamic we set a variable
// when the CAD functions are defined. Value of 3 is minimum frequencies a
// gateway should support to be fully LoRa compliant.
// For performance reasons, 3 is the maximum as well!
//
#define NUM_HOPS 3

// Do not change these setting for RSSI detection. They are used for CAD
// Given the correction factor of 157, we can get to -122dB with this rating
// 
#define RSSI_LIMIT	35							// 

// How long to wait in LoRa mode before using the RSSI value.
// This period should be as short as possible, yet sufficient
// 
#define RSSI_WAIT	6							// was 25

// How long will it take when hopping before a CDONE or CDETD value
// is present and can be measured.
//
#define EVENT_WAIT	15000						// XXX 180520 was 25 milliseconds before CDDETD timeout
#define DONE_WAIT	1950						// 2000 microseconds (1/500) sec between CDDONE events


// SPI setting. 8MHz seems to be the max
#define SPISPEED 8000000						// Set to 8 * 10E6

// Frequencies
// Set center frequency. If in doubt, choose the first one, comment out all others
// Each "real" gateway should support the first 3 frequencies according to LoRa spec.
// NOTE: This means you have to specify at least 3 frequencies here for the single
//	channel gateway to work according to spec. 
struct vector {
	// Upstream messages
	uint32_t upFreq;							// 4 bytes unsigned int32 Frequency
	uint16_t upBW;								// 2 bytes
	uint8_t  upLo;								// 1 bytes
	uint8_t  upHi;								// 1 bytes
	// Downstream messages
	uint32_t dwnFreq;							// 4 bytes unsigned int32 Frequency
	uint16_t dwnBW;								// 2 bytes BW Specification
	uint8_t  dwnLo;								// 1 bytes Spreading Factor
	uint8_t  dwnHi;								// 1 bytes
};

// Define all the relevant LoRa Regions
//==
#ifdef EU863_870
// This the the EU863_870 format as used in most of Europe
// It is also the default for most of the single channel gateway work.
// For each frequency SF7-SF12 are used.
vector freqs [] = 
{
	{ 868100000, 125, 7, 12, 868100000, 125, 7, 12},			// Channel 0, 868.1 MHz/125 primary
	{ 868300000, 125, 7, 12, 868300000, 125, 7, 12},			// Channel 1, 868.3 MHz/125 mandatory and (SF7BW250)
	{ 868500000, 125, 7, 12, 868500000, 125, 7, 12},			// Channel 2, 868.5 MHz/125 mandatory
	{ 867100000, 125, 7, 12, 867100000, 125, 7, 12},			// Channel 3, 867.1 MHz/125 Optional
	{ 867300000, 125, 7, 12, 867300000, 125, 7, 12},			// Channel 4, 867.3 MHz/125 Optional
	{ 867500000, 125, 7, 12, 867500000, 125, 7, 12}, 			// Channel 5, 867.5 MHz/125 Optional
	{ 867700000, 125, 7, 12, 867700000, 125, 7, 12},			// Channel 6, 867.7 MHz/125 Optional 
	{ 867900000, 125, 7, 12, 867900000, 125, 7, 12},			// Channel 7, 867.9 MHz/125 Optional 
	{ 868800000, 125, 7, 12, 868800000, 125, 7, 12},			// Channel 8, 868.9 MHz/125 FSK Only										
	{ 0,         0  , 0,  0, 869525000, 125, 9, 9}				// Channel 9, 869.525 MHz/125 for RX2 responses SF9(10%)
	// TTN defines an additional channel at 869.525 MHz using SF9 for class B. Not used
};

#elif defined(EU433)
// The following 3 frequencies should be defined/used in an EU433 
// environment. The plan is not defined for TTN yet so we use this one.
vector freqs [] = {
	{ 433175000, 125, 7, 12, 433175000, 125, 7, 12},			// Channel 0, 433.175 MHz/125 primary
	{ 433375000, 125, 7, 12, 433375000, 125, 7, 12},			// Channel 1, 433.375 MHz primary
	{ 433575000, 125, 7, 12, 433575000, 125, 7, 12},			// Channel 2, 433.575 MHz primary
	{ 433775000, 125, 7, 12, 433775000, 125, 7, 12},			// Channel 3, 433.775 MHz primary
	{ 433975000, 125, 7, 12, 433975000, 125, 7, 12},			// Channel 4, 433.975 MHz primary
	{ 434175000, 125, 7, 12, 434175000, 125, 7, 12},			// Channel 5, 434.175 MHz primary
	{ 434375000, 125, 7, 12, 434375000, 125, 7, 12},			// Channel 6, 434.375 MHz primary
	{ 434575000, 125, 7, 12, 434575000, 125, 7, 12},			// Channel 7, 434.575 MHz primary
	{ 434775000, 125, 7, 12, 434775000, 125, 7, 12}				// Channel 8, 434.775 MHz primary
};

#elif defined(US902_928)
// The frequency plan for USA is a difficult one. As you can see, the uplink protocol uses
// SF7-SF10 and BW125 whereas the downlink protocol uses SF7-SF12 and BW500.
// Also the number of chanels is not equal.
vector freqs [] = {
	// Uplink
	{ 903900000, 125, 7, 10, 923300000, 500, 7, 12},			// Up Ch 0, SF7BW125 to SF10BW125 primary
	{ 904100000, 125, 7, 10, 923900000, 500, 7, 12},			// Up Ch 1, SF7BW125 to SF10BW125
	{ 904300000, 125, 7, 10, 924500000, 500, 7, 12},			// Up Ch 2, SF7BW125 to SF10BW125, Dwn SF7-SF12 924,5 BW500
	{ 904500000, 125, 7, 10, 925100000, 500, 7, 12},			// Up Ch 3, SF7BW125 to SF10BW125, Dwn SF7-SF12 925,1 BW500
	{ 904700000, 125, 7, 10, 925700000, 500, 7, 12},			// Up Ch 3, SF7BW125 to SF10BW125, Dwn SF7-SF12 925,1
	{ 904900000, 125, 7, 10, 926300000, 500, 7, 12},			// Up Ch 4, SF7BW125 to SF10BW125, Dwn SF7-SF12 
	{ 905100000, 125, 7, 10, 926900000, 500, 7, 12},			// Up Ch 5, SF7BW125 to SF10BW125, Dwn SF7-SF12 
	{ 905300000, 125, 7, 10, 927500000, 500, 7, 12},			// Up Ch 6, SF7BW125 to SF10BW125, Dwn SF7-SF12 
	{ 904600000, 500, 8,  8, 0        , 0,   0, 00},			// Up Ch 7, SF8BW5000, no Dwn 0 																						// SFxxxBW500
};

#elif defined(AU925_928)
// Australian plan or TTN/Lora frequencies
vector freqs [] = { 
	{ 916800000, 125, 7, 10, 916800000, 125, 7, 12},			// Channel 0, 916.8 MHz primary
	{ 917000000, 125, 7, 10, 917000000, 125, 7, 12},			// Channel 1, 917.0 MHz mandatory
	{ 917200000, 125, 7, 10, 917200000, 125, 7, 12},			// Channel 2, 917.2 MHz mandatory
	{ 917400000, 125, 7, 10, 917400000, 125, 7, 12},			// Channel 3, 917.4 MHz Optional
	{ 917600000, 125, 7, 10, 917600000, 125, 7, 12},			// Channel 4, 917.6 MHz Optional
	{ 917800000, 125, 7, 10, 917800000, 125, 7, 12},			// Channel 5, 917.8 MHz Optional
	{ 918000000, 125, 7, 10, 918000000, 125, 7, 12}, 			// Channel 6, 918.0 MHz Optional 
	{ 918200000, 125, 7, 10, 918200000, 125, 7, 12}	,  			// Channel 7, 918.2 MHz Optional
	{ 917500000, 500, 8,  8,         0,   0, 0,  0}	  			// Channel 8, 917.5 SF8BW500 MHz Optional Uplink
};

#elif defined(CN470_510)
// China plan for TTN frequencies
vector freqs [] = { 
	{ 486300000, 125, 7, 12, 486300000, 125, 7, 12},			// 486.3 - SF7BW125 to SF12BW125
	{ 486500000, 125, 7, 12, 486500000, 125, 7, 12},			// 486.5 - SF7BW125 to SF12BW125
	{ 486700000, 125, 7, 12, 486700000, 125, 7, 12},			// 486.7 - SF7BW125 to SF12BW125
	{ 486900000, 125, 7, 12, 486900000, 125, 7, 12},			// 486.9 - SF7BW125 to SF12BW125
	{ 487100000, 125, 7, 12, 487100000, 125, 7, 12},			// 487.1 - SF7BW125 to SF12BW125
	{ 487300000, 125, 7, 12, 487300000, 125, 7, 12},			// 487.3 - SF7BW125 to SF12BW125
	{ 487500000, 125, 7, 12, 487500000, 125, 7, 12},			// 487.5 - SF7BW125 to SF12BW125
	{ 487700000, 125, 7, 12, 487700000, 125, 7, 12}				// 487.7 - SF7BW125 to SF12BW125
};

#elif defined(IN865_867)
vector freqs [] = { 
	{ 865062500, 125, 7, 12, 865062500, 125,  7, 12},			// And RX1
	{ 865402500, 125, 7, 12, 865402500, 125,  7, 12},
	{ 865985000, 125, 7, 12, 865985000, 125,  7, 12},
	{         0,   0, 0,  0, 866550000, 125, 10, 10}			// RX2
};

#else
vector freqs [] = {
	// Print an Error, Not supported
#	error "Sorry, but your frequency plan is not supported"
};
#endif



// Set the structure for spreading factor
enum sf_t { SF6=6, SF7, SF8, SF9, SF10, SF11, SF12 };

// The state of the receiver. See Semtech Datasheet (rev 4, March 2015) page 43
// The _state is of the enum type (and should be cast when used as a number)
enum state_t { S_INIT=0, S_SCAN, S_CAD, S_RX, S_TX, S_TXDONE};

volatile state_t _state=S_INIT;
volatile uint8_t _event=0;

// rssi is measured at specific moments and reported on others
// so we need to store the current value we like to work with
uint8_t _rssi;	

uint32_t msgTime=0;							// in seconds, Thru nowSeconds, now()
uint32_t hopTime=0;							// in micros()
uint32_t detTime=0;							// In micros()

#if _PIN_OUT==1
// ----------------------------------------------------------------------------
// Definition of the GPIO pins used by the Gateway for Hallard type boards
//
struct pins {
	uint8_t dio0=15;	// GPIO15 / D8. For the Hallard board shared between DIO0/DIO1/DIO2
	uint8_t dio1=15;	// GPIO15 / D8. Used for CAD, may or not be shared with DIO0
	uint8_t dio2=15;	// GPIO15 / D8. Used for frequency hopping, don't care
	uint8_t ss=16;		// GPIO16 / D0. Select pin connected to GPIO16 / D0
	uint8_t rst=0;		// GPIO 0 / D3. Reset pin not used	
	// MISO 12 / D6
	// MOSI 13 / D7
	// CLK  14 / D5
} pins;

#elif _PIN_OUT==2
// ----------------------------------------------------------------------------
// For ComResult gateway PCB use the following settings
struct pins {
	uint8_t dio0=5;		// GPIO5 / D1. Dio0 used for one frequency and one SF
	uint8_t dio1=4;		// GPIO4 / D2. Used for CAD, may or not be shared with DIO0
	uint8_t dio2=0;		// GPIO0 / D3. Used for frequency hopping, don't care
	uint8_t ss=15;		// GPIO15 / D8. Select pin connected to GPIO15
	uint8_t rst=0;		// GPIO0  / D3. Reset pin not used	
} pins;


#elif _PIN_OUT==3
// ----------------------------------------------------------------------------
// For ESP32/Wemos based board
// SCK  == GPIO5/ PIN5
// SS   == GPIO18/PIN18
// MISO == GPIO19/ PIN19
// MOSI == GPIO27/ PIN27
// RST  == GPIO14/ PIN14
// This Pinning is not used and is under construction
struct pins {
	uint8_t dio0=26;		// GPIO26 / Dio0 used for one frequency and one SF
	uint8_t dio1=26;		// GPIO26 / Used for CAD, may or not be shared with DIO0
	uint8_t dio2=26;		// GPI2O6 / Used for frequency hopping, don't care
	uint8_t ss=18;			// GPIO18 / Dx. Select pin connected to GPIO18
	uint8_t rst=14;			// GPIO0  / D3. Reset pin not used	
} pins;


#elif _PIN_OUT==4
// ----------------------------------------------------------------------------
// For ESP32/TTGO based board.
// SCK  == GPIO5/  PIN5
// SS   == GPIO18/ PIN18 CS
// MISO == GPIO19/ PIN19
// MOSI == GPIO27/ PIN27
// RST  == GPIO14/ PIN14
struct pins {
	uint8_t dio0=26;		// GPIO26 / Dio0 used for one frequency and one SF
	uint8_t dio1=33;		// GPIO33 / Used for CAD, may or not be shared with DIO0
	uint8_t dio2=32;		// GPIO32 / Used for frequency hopping, don't care
	uint8_t ss=18;			// GPIO18 / CS. Select pin connected to GPIO18
	uint8_t rst=14;			// GPIO14 / D3. Reset pin not used	
} pins;
#define SCK 5
#define MISO 19
#define MOSI 27
#define RST 14
#define SS 18

#if _GPS==1
#define GPS_RX 15
#define GPS_TX 12
#endif //_GPS

#elif _PIN_OUT==5
// ----------------------------------------------------------------------------
// For ESP32/Heltec Wifi LoRA 32(V2) HTIT-WB32LA board with 0.9" OLED
//
// SCK  == GPIO5/ PIN5
// SS   == GPIO18/PIN18 CS
// MISO == GPIO19/ PIN19
// MOSI == GPIO27/ PIN27
// RST  == GPIO14/ PIN14
struct pins {
	uint8_t dio0=26;		// GPIO26 / Dio0 used for one frequency and one SF
	uint8_t dio1=35;		// GPIO35 / Used for CAD, may or not be shared with DIO0
	uint8_t dio2=34;		// GPIO34 / Used for frequency hopping, don't care
	uint8_t ss=18;			// GPIO18 / Dx. Select pin connected to GPIO18
	uint8_t rst=14;			// GPIO0 / D3. Reset pin not used	
} pins;
#define SCK 5				// Check
#define MISO 19				// Check
#define MOSI 27				// Check
#define RST 14				// Check
#define SS 18


#else
// ----------------------------------------------------------------------------
// Use your own pin definitions, and comment #error line below
// MISO 12 / D6
// MOSI 13 / D7
// CLK  14 / D5
// SS   16 / D0
#error "Pin Definitions _PIN_OUT must be defined in loraModem.h"
#endif

// stat_t contains the statistics that are kept for a message. 
// Each time a message is received or sent the statistics are updated.
// In case _STATISTICS==1 we define the last gwayConfig.maxStat message as statistics
struct stat_t {
	uint32_t time;							// Time since 1970 in seconds		
	uint32_t node;							// 4-byte DEVaddr (the only one known to gateway)
	uint8_t ch;								// Channel index to freqs array
	uint8_t sf;
#if RSSI==1
	int8_t	rssi;						// XXX Can be < -128
#endif
	int8_t	prssi;						// XXX Can be < -128
	uint8_t upDown;							// 0==up, 1==down
#if _LOCALSERVER>=1
	uint8_t data[24];						// For memory purposes, only 24 chars
	uint8_t datal;							// Length of decoded message 1 char
#endif
} stat_t;


#if _STATISTICS >= 1
// statc_c contains the statistic that are gateway related and not per
// message. Example: Number of messages received on SF7 or number of (re) boots
// So where statr contains the statistics gathered per packet the statc_c
// contains general statistics of the node

struct stat_c {

	uint32_t msg_ok;
	uint32_t msg_ttl;
	uint32_t msg_down;
	uint32_t msg_sens;

	// Of statistics == 2 we add spreading factor data to the statistics
#if _STATISTICS >= 2						// Only if we explicitly set it higher	
	uint32_t sf7, sf8, sf9;					// Spreading factor 7, 8, 9 statistics/Count
	uint32_t sf10, sf11, sf12;				// Spreading factor 10, 11, 12
	
	// If _STATISTICS is 3, we add statistics about the channel 
	// When only one channel is used, we normally know the spread of these
	// statistics, but when HOP mode is selected we migth want to add this info
#if _STATISTICS >=3
	uint32_t msg_ok_0,   msg_ok_1,   msg_ok_2;
	uint32_t msg_ttl_0,  msg_ttl_1,  msg_ttl_2;
	uint32_t msg_down_0, msg_down_1, msg_down_2;
	uint32_t msg_sens_0, msg_sens_1, msg_sens_2;

	uint32_t  sf7_0,  sf7_1,  sf7_2;
	uint32_t  sf8_0,  sf8_1,  sf8_2;
	uint32_t  sf9_0,  sf9_1,  sf9_2;
	uint32_t sf10_0, sf10_1, sf10_2;
	uint32_t sf11_0, sf11_1, sf11_2;
	uint32_t sf12_0, sf12_1, sf12_2;
#endif //3
	
	uint16_t boots;							// Number of boots
	uint16_t resets;
#endif // 2

} stat_c;
struct stat_c statc;

// History of received uplink and downlink messages from nodes
struct stat_t * statr;


#else //_STATISTICS==0
struct stat_t	statr[1];					// Always have at least one element to store in
#endif


// Define the payload structure used to separate interrupt and SPI
// processing from the loop() part
uint8_t payLoad[128];						// Payload i



// ====================================================================
// PACKET FORWARDER
// Smetech Specification
// https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT
// Some parts are included both at Upstram and Downstream since
// the gateway can also be used as a repeater
// ====================================================================
struct LoraDown {
	uint32_t	tmst;						// Timestamp (will ignore time)
	uint32_t	tmms;						// Timestamp according to GPS (sync required)
	uint32_t	time;
	uint32_t	freq;						// Frequency
	
	uint16_t	fcnt;						// Framecount of the requesting LoraUp message

	uint8_t		size;
	uint8_t		chan;						// = <NOT USED>
	uint8_t		sf;							// through datr
	uint8_t		bw;							// through datr
	
	bool		ipol;
	uint8_t		powe;						// transmit power, normally 14, except when using special channel
	uint8_t		crc;
	uint8_t		iiq;						// message inverted or not for node-node communiction
	uint8_t		imme;						// Immediate transfer execution
	uint8_t		ncrc;						// no CRC check
	uint8_t		prea;						// preamble
	uint8_t		rfch;						// Antenna "RF chain" used for TX (unsigned integer)

	char *		modu;						//	"LORA" os "FSCK"
	char *		datr;						// = "SF12BW125", contains both .sf and .bw parts
	char *		codr;

	uint8_t	* 	payLoad;
} LoraDown;



// Up buffer (from Lora sensor to UDP)
// This struct contains all data of the buffer received from devices to gateway

struct LoraUp {
	uint32_t	tmst;						// Timestamp of message
	uint32_t	tmms;						// <not used at the moment>
	uint32_t	time;						// <not used at the moment>
	uint32_t	freq;						// MMM frequency used in HZ
	uint8_t		size;						// Length of the message Payload
	uint8_t		chan;						// Channel "IF" used for RX
	uint8_t		sf;							// Spreading Factor

	int32_t		snr;
	int16_t		prssi; 
	int16_t		rssicorr;

	char *		modu;						//	"LORA" os "FSCK"

	uint8_t		payLoad[128];
} LoraUp;




// ============================================================================
// Set all definitions for Gateway
// ============================================================================	
// Register definitions. These are the addresses of the RFM95, SX1276 that we 
// need to set in the program.

#define REG_FIFO                    0x00		// rw FIFO address
#define REG_OPMODE                  0x01		// Operation Mode Register (Page 108)
// 0x02	 reserved
// 0x03
// 0x04
// 0x05											// Register 2 to 5 are unused for LoRa
#define REG_FRF_MSB					0x06
#define REG_FRF_MID					0x07
#define REG_FRF_LSB					0x08
#define REG_PAC                     0x09
#define REG_PARAMP                  0x0A
#define REG_OCP						0x0B		// 200927 Not used yet
#define REG_LNA                     0x0C
#define REG_FIFO_ADDR_PTR           0x0D		// rw SPI interface address pointer in FIFO data buffer
#define REG_FIFO_TX_BASE_AD         0x0E		// rw write base address in FIFO data buffer for TX modulator
#define REG_FIFO_RX_BASE_AD         0x0F		// rw read base address in FIFO data buffer for RX demodulator (0x00)

#define REG_FIFO_RX_CURRENT_ADDR	0x10		// r  Address of last packet received
#define REG_IRQ_FLAGS_MASK			0x11
#define REG_IRQ_FLAGS				0x12
#define REG_RX_BYTES_NB				0x13
// 0x14
// 0x15
// 0x16
// 0x17
// 0x18
#define REG_PKT_SNR_VALUE			0x19
#define REG_PKT_RSSI				0x1A		// latest package
#define REG_RSSI					0x1B		// Current RSSI, section 6.4, or  5.5.5
#define REG_HOP_CHANNEL				0x1C
#define REG_MODEM_CONFIG1           0x1D		// LoRa: Modem PHY config 1
#define REG_MODEM_CONFIG2           0x1E		// LoRa: Modem PHY config 2
#define REG_SYMB_TIMEOUT_LSB  		0x1F

#define REG_PREAMBLE_MSB			0x20
#define REG_PREAMBLE_LSB			0x21		// MMM 200930: set to 0x08
#define REG_PAYLOAD_LENGTH          0x22
#define REG_MAX_PAYLOAD_LENGTH 		0x23
#define REG_HOP_PERIOD              0x24
#define REG_FIFO_RX_BYTE_ADDR_PTR	0x25		// 200927 Not used yet
#define REG_MODEM_CONFIG3           0x26		// Modem PHY config 3, bit 2 AgcAutoOn and 3 LowDataRateOptimize used
#define REG_PPM_CORRECTION			0x27
#define REG_FREQ_ERROR_MSB			0x28 (r)
#define REG_FREQ_ERROR_MID			0x29 (r)
#define REG_FREQ_ERROR_LSB			0x2A (r)
// 0x2B reserved
#define REG_RSSI_WIDEBAND			0x2C
// 0x2D reserved
// 0x2E reserved
// 0x2F reserved

// 0x30 reserved
#define REG_DETECT_OPTIMIZE			0x31
// 0x32 reserved
#define REG_INVERTIQ				0x33		// 0x27 for normal and 0x40 for inverted IIQ
// 0x34 reserved
// 0x35 reserved
// 0x36 reserved
#define REG_DET_TRESH				0x37		// SF6
// 0x38 reserved
#define REG_SYNC_WORD				0x39
#define REG_TEMP					0x3C

#define REG_DIO_MAPPING_1           0x40		// Mapping of pins DIO0 to DIO3
#define REG_DIO_MAPPING_2           0x41		// Mapping of pins DIO4 and DIO5, ClkOut frequency
#define REG_VERSION	  				0x42		// 0x12? Semtech def

#define REG_PADAC					0x5A
#define REG_PADAC_SX1272			0x5A
#define REG_PADAC_SX1276			0x4D


// ----------------------------------------
// opModes
#define SX72_MODE_SLEEP             0x80
#define SX72_MODE_STANDBY           0x81
#define SX72_MODE_FSTX              0x82
#define SX72_MODE_TX                0x83		// 0x80 | 0x03
#define SX72_MODE_RX_CONTINUOS      0x85

// ----------------------------------------
// LMIC Constants for radio registers
#define OPMODE_LORA      			0x80		// bit 6 is AccessSharing Reg. Bits 4 and 5 are 0x00
#define OPMODE_MASK      			0x0F		// Select LSB 8 bits 0, ignore LoRa bit for example

#define OPMODE_LOWFREQ				0x08		// Should be - for 868.1 MHZ operation

#define OPMODE_SLEEP     			0x00
#define OPMODE_STANDBY   			0x01
#define OPMODE_FSTX      			0x02
#define OPMODE_TX        			0x03
#define OPMODE_FSRX      			0x04
#define OPMODE_RX        			0x05
#define OPMODE_RX_SINGLE 			0x06
#define OPMODE_CAD       			0x07



// ----------------------------------------
// LOW NOISE AMPLIFIER

#define LNA_MAX_GAIN                0x23		// Max gain 0x20 | Boost 0x03
#define LNA_OFF_GAIN                0x00
#define LNA_LOW_GAIN		    	0x20

// CONF REG
#define REG1                        0x0A
#define REG2                        0x84

// ----------------------------------------
// MC1 sx1276 RegModemConfig1
#define SX1276_MC1_BW_125           0x70
#define SX1276_MC1_BW_250           0x80
#define SX1276_MC1_BW_500           0x90
#define SX1276_MC1_CR_4_5           0x02			// sx1276
#define SX1276_MC1_CR_4_6           0x04
#define SX1276_MC1_CR_4_7           0x06
#define SX1276_MC1_CR_4_8           0x08
#define SX1276_MC1_IMPLICIT_HEADER_MODE_ON  0x01

#define SX72_MC1_LOW_DATA_RATE_OPTIMIZE     0x01 	// mandated for SF11 and SF12

// ----------------------------------------
// MC2 definitions
#define SX72_MC2_FSK                0x00
#define SX72_MC2_SF7                0x70		// SF7 == 0x07, so (SF7<<4) == SX7_MC2_SF7
#define SX72_MC2_SF8                0x80
#define SX72_MC2_SF9                0x90
#define SX72_MC2_SF10               0xA0
#define SX72_MC2_SF11               0xB0
#define SX72_MC2_SF12               0xC0

// ----------------------------------------
// MC3
#define SX1276_MC3_LOW_DATA_RATE_OPTIMIZE  0x08
#define SX1276_MC3_AGCAUTO                 0x04

// ----------------------------------------
// FRF
#define FRF_MSB						0xD9		// 868.1 MHz
#define FRF_MID						0x06
#define FRF_LSB						0x66

// ----------------------------------------
// DIO function mappings           		     D0D1D2D3
#define MAP_DIO0_LORA_RXDONE   		0x00  // 00------ bit 7 and 6
#define MAP_DIO0_LORA_TXDONE   		0x40  // 01------
#define MAP_DIO0_LORA_CADDONE  		0x80  // 10------
#define MAP_DIO0_LORA_NOP   		0xC0  // 11------

#define MAP_DIO1_LORA_RXTOUT   		0x00  // --00---- bit 5 and 4
#define MAP_DIO1_LORA_FCC			0x10  // --01----
#define MAP_DIO1_LORA_CADDETECT		0x20  // --10----
#define MAP_DIO1_LORA_NOP      		0x30  // --11----

#define MAP_DIO2_LORA_FCC0      	0x00  // ----00-- bit 3 and 2
#define MAP_DIO2_LORA_FCC1      	0x04  // ----01-- bit 3 and 2
#define MAP_DIO2_LORA_FCC2      	0x08  // ----10-- bit 3 and 2
#define MAP_DIO2_LORA_NOP      		0x0C  // ----11-- bit 3 and 2

#define MAP_DIO3_LORA_CADDONE  		0x00  // ------00 bit 1 and 0
#define MAP_DIO3_LORA_HEADER		0x01  // ------01
#define MAP_DIO3_LORA_CRC			0x02  // ------10
#define MAP_DIO3_LORA_NOP      		0x03  // ------11

// FSK specific
#define MAP_DIO0_FSK_READY     		0x00  // 00------ (packet sent / payload ready)
#define MAP_DIO1_FSK_NOP       		0x30  // --11----
#define MAP_DIO2_FSK_TXNOP     		0x04  // ----01--
#define MAP_DIO2_FSK_TIMEOUT   		0x08  // ----10--

// ----------------------------------------
// Bits masking the corresponding IRQs from the radio
#define IRQ_LORA_RXTOUT_MASK 		0x80	// RXTOUT
#define IRQ_LORA_RXDONE_MASK 		0x40	// RXDONE after receiving the header and CRC, we receive payload part
#define IRQ_LORA_CRCERR_MASK 		0x20	// CRC error detected. Note that RXDONE will also be set
#define IRQ_LORA_HEADER_MASK 		0x10	// valid HEADER mask. This interrupt is first when receiving a message
#define IRQ_LORA_TXDONE_MASK 		0x08	// End of Transmission
#define IRQ_LORA_CDDONE_MASK 		0x04	// CDDONE
#define IRQ_LORA_FHSSCH_MASK 		0x02
#define IRQ_LORA_CDDETD_MASK 		0x01	// Detect preamble channel


// ----------------------------------------
// Definitions for UDP message arriving from server
#define PROTOCOL_VERSION			0x01

#define PUSH_DATA					0x00
#define PUSH_ACK					0x01
#define PULL_DATA					0x02
#define PULL_RESP					0x03
#define PULL_ACK					0x04
#define TX_ACK						0x05

#define MGT_RESET					0x15		// Not a LoRa Gateway Spec message
#define MGT_SET_SF					0x16
#define MGT_SET_FREQ				0x17

