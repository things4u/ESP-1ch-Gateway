// LoRa encoding and decoding functions
// Copyright (c) 2016 Maarten Westenberg 
// Version 1.1.0
// Date: 2016-10-23
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License
// which accompanies this distribution, and is available at
// https://opensource.org/licenses/mit-license.php
//
// Author: Maarten Westenberg
//
// The protocols used in this code: 
// 1. LoRA Specification version V1.0 and V1.1 for Gateway-Node communication
//	
// 2. Semtech Basic communication protocol between Lora gateway and server version 3.0.0
//	https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT
//
// Notes: 
// The lCode specification is documented on a sparate page on github.
//
// Todo:
// The luminescense is read as a 16-bit value in the library and converted to
//	a float value in order to get proper scaling etc. In the lCode lib it is 
//	coded as a 2-byte value over the air, which might be incorrect for lux values
//	over 650 lux (which IS posible since bright daylight has more than 1000 lux).
//	So XXX we have to add another byte to cover values above 65
// ----------------------------------------------------------------------------------------

#ifndef LoRaCode_h
#define LoRaCode_h

// Op Codes
#define O_TEMP		0x01				// Temperature is a one-byte code
#define O_HUMI		0x02				// Humidity is a one-byte code
#define O_AIRP		0x03				// Air pressure is a one-byte code
#define O_GPS		0x04				// Short version: ONLY 3 bytes LAT and 3 bytes LONG
#define O_GPSL		0x05				// Long GPS
#define O_PIR		0x06				// Movement, 1 bit (=1 byte)
#define O_AQ		0x07				// Airquality
#define O_RTC		0x08				// Real Time Clock
#define O_COMPASS	0x09				// Compass
#define O_MB		0x0A				// Multi Sensors 433
#define O_MOIST 	0x0B				// Moisture	is one-byte
#define O_LUMI  	0x0C				// Luminescense u16
#define O_DIST		0x0D				// Distance is 2-byte
#define O_GAS		0x0E				// GAS 
// 0x0F

// 0x10									// 16 values
// 0x11
// ..
// 0x1F

#define O_BATT		0x20				// Internal Battery
#define O_ADC0		0x21				// AD converter on pin 0
#define O_ADC1		0x22

// Reserved for LoRa messages (especially downstream)
#define O_STAT		0x30				// Ask for status message from node
#define O_SF		0x31				// Spreading factor change OFF=0, values 7-12
#define O_TIM		0x32				// Timing of the wait cyclus (20 to 7200 seconds)
#define O_1CH		0x33				// Single channel: Channel Value=0-9, OFF==255
#define O_LOC		0x34				// Ask for the location. Responds with GPS (if available)

// ..
// 0x3F

class LoRaCode
{
	public:

		int		eVal(int opcode, byte *val, byte *msg);
		int		eTemperature(float val, byte *msg);
		int		eHumidity(float val, byte *msg);
		int		eAirpressure(float val, byte *msg);
		int		eGps(double lat, double lng, byte *msg);
		int		eGpsL(double lat, double lng, long alt, int sat, byte *msg);
		int		ePir(int val, byte *msg);
		int		eAirquality(int pm25, int pm10, byte *msg);			// value 0 (good) -1024 (gas)
		int		eMbuttons(byte val, unsigned long address, unsigned short channel, byte *msg);		// concentrator for multi-buttons
		int		eMoist(int val, byte *msg);							// 255 is dry, 0 is wet
		int		eLuminescense(float val, byte *msg);				// val contains light intensity
		int		eLuminescenseL(float val, byte *msg);				// long contains light intensity
		int		eDistance(int val, byte *msg);
		int		eGas(int val, byte *msg);
		
		// opcodes 0x0F until 0x1F
		int		eBattery(float val, byte *msg);
		int		eAdc0(int val, byte *msg);							// Pin A0 has 1024 values, we use 256
		int		eAdc1(int val, byte *msg);							// Pin A1 has 1024 values, we use 256
		
		bool	eMsg(byte *msg, int len);
		void	lPrint(byte *msg, int len);
	
		//Decoding (downstream)
		int		dLen (byte *msg);
		int		dMsg (byte *msg, byte *val, byte *mode);
		
};

extern LoRaCode lcode;
#endif