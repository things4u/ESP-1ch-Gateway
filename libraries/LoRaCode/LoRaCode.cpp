// LoRa encoding and decoding functions
// Copyright (c) 2016, 2017 Maarten Westenberg (mw12554@hotmail.com)
// Version 1.2.0
// Date: 2016-10-23
// Version 1.2.0 on April 20, 2017
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

#define DEBUG 0

#if defined(__AVR__)
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <Battery.h>
#elif defined(ARDUINO_ARCH_ESP8266) | defined(ESP32)
#include <ESP.h>
#elif defined(__MKL26Z64__)
#include <Arduino.h>
#else
#error Unknown architecture in aes.cpp
#endif

#include "LoRaCode.h"

int ldebug=DEBUG;

// --------------------------------------------------------------------------------
// Encode Temperature. 
// We use 2 bytes for temperature, first contains the integer partial_sort.
// We add 100 so we effectively will measure temperatures between -100 and 154 degrees.
// Second byte contains the fractional part in 2 decimals (00-99).
// --------------------------------------------------------------------------------
int LoRaCode::eTemperature(float val, byte *msg) {
	int len=0;
	byte i= (byte) (val+100);					// Integer part
	byte f= (byte) ( ((float)(val - (float)i -100)) * 100); // decimal part
	msg[len++] = ((byte)O_TEMP << 2) | 0x01;	// Last 2 bits are 0x01, two bytes
	msg[len++] = i;
	msg[len++] = f;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Temperature "));
		Serial.print(i-100);					// For readibility
		Serial.print(".");
		Serial.println(f);
	}
#endif
	return(len);
}

// --------------------------------------------------------------------------------
// Code Humidity in the *msg
// The humidity of the sensor is between 0-100%, we encode this value * 2 so that
// byte value is betwene 0 and 199.
// --------------------------------------------------------------------------------
int LoRaCode::eHumidity(float val, byte *msg) {
	int len=0;
	byte i = (byte) ((float)val *2);				// Value times 2
	msg[len++] = ((byte)O_HUMI << 2) | 0x00;	// Last 2 bits are 0x00, one byte 
	msg[len++] = i;								//
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Humidity "));
		Serial.println((float)(i/2));			// For readibility
	}
#endif
	return(len);
}

// --------------------------------------------------------------------------------
// Code the Airpressure.
// The coded value is the measured value minus 900. This gives an airpressure
// range from 850-1104
// --------------------------------------------------------------------------------
int LoRaCode::eAirpressure(float val, byte *msg) {
	int len=0;
	byte i = (byte) ((float)val -850);			// Value times 2
	msg[len++] = ((byte)O_AIRP << 2) | 0x00;	// Last 2 bits are 0x00, one byte 
	msg[len++] = i;								//
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Airpressure "));
		Serial.println((float)(i + 850));		// For readibility
	}
#endif
	return(len);
}

// --------------------------------------------------------------------------------
// GPS lat and lng encoding
// There are several ways to encode GPS data. Eother by mapping on 3 or 4 bytes of
// Floating, or by just multiplying with 1,000,000 for example (gives 6 digits
// precision which is enough for almost all applications)
// --------------------------------------------------------------------------------
int LoRaCode::eGps(double lat, double lng, byte *msg) {
#if DEBUG>0
	if (ldebug>=1) {
		Serial.print(F(" LAT: ")); Serial.print(lat,6);
		Serial.print(F(" LNG: ")); Serial.println(lng,6);
	}
#endif
	int len=0;
	long factor = 1000000;
	msg[len++] = ((byte)O_GPS << 2) | 0x00;		// Last 2 bits are 0x00, don't care
	const long calculatedLat = (long)(lat * factor);
	msg[len++] = (calculatedLat >> (8*3)) & 0xFF;
	msg[len++] = (calculatedLat >> (8*2)) & 0xFF;
	msg[len++] = (calculatedLat >> (8*1)) & 0xFF;
	msg[len++] = (calculatedLat >> (8*0)) & 0xFF;
	const long calculatedLng = (long)(lng * factor);
	msg[len++] = (calculatedLng >> (8*3)) & 0xFF;
	msg[len++] = (calculatedLng >> (8*2)) & 0xFF;
	msg[len++] = (calculatedLng >> (8*1)) & 0xFF;
	msg[len++] = (calculatedLng >> (8*0)) & 0xFF;
	return(len);
}

// --------------------------------------------------------------------------------
// Code the extended GPS format
// latitude and longitude are coded just like the short format
// Altitude is a long containing the altitude in cm
// --------------------------------------------------------------------------------
int LoRaCode::eGpsL(double lat, double lng, long alt, int sat,
					byte *msg) {
	if (ldebug>=1) {
		Serial.print(F(" LAT: ")); Serial.print(lat,6);
		Serial.print(F(" LNG: ")); Serial.print(lng,6);
		Serial.print(F(" ALT: ")); Serial.print(alt/100);
		Serial.print(F(" SAT: ")); Serial.println(sat);
	}
	int len=0;
	long factor = 1000000;
	msg[len++] = ((byte)O_GPSL << 2) | 0x00;	// Last 2 bits are 0x00, don't care
	const long calculatedLat = (long)(lat * factor);
	msg[len++] = (calculatedLat >> (8*3)) & 0xFF;
	msg[len++] = (calculatedLat >> (8*2)) & 0xFF;
	msg[len++] = (calculatedLat >> (8*1)) & 0xFF;
	msg[len++] = (calculatedLat >> (8*0)) & 0xFF;
	const long calculatedLng = (long)(lng * factor);
	msg[len++] = (calculatedLng >> (8*3)) & 0xFF;
	msg[len++] = (calculatedLng >> (8*2)) & 0xFF;
	msg[len++] = (calculatedLng >> (8*1)) & 0xFF;
	msg[len++] = (calculatedLng >> (8*0)) & 0xFF;
	const long calculatedAlt = (long)(alt);		// Altitude is integer specified in cm (converted later to m)
	msg[len++] = (calculatedAlt >> (8*3)) & 0xFF;
	msg[len++] = (calculatedAlt >> (8*2)) & 0xFF;
	msg[len++] = (calculatedAlt >> (8*1)) & 0xFF;
	msg[len++] = (calculatedAlt >> (8*0)) & 0xFF;
	msg[len++] = sat & 0xFF;						// Positive number, assumed always to be less than 255
	return(len);
}

// --------------------------------------------------------------------------------
// Encode the 1-bit PIR value
// --------------------------------------------------------------------------------
int LoRaCode::ePir(int val, byte *msg) {
	int i = (byte) ( val );	
	int len=0;										//
	msg[len++] = (O_PIR << 2) | 0x00;				// Last 2 bits are 0x00, one byte 
	msg[len++] = i;	
	return(len);
}


// --------------------------------------------------------------------------------
// Encode Airquality Value.
// Airquality through SDS011 sensors is measured with two float values
// and read as a 10-bit value (0-1024). We use a 2 x 2-byte value
// --------------------------------------------------------------------------------
int LoRaCode::eAirquality(int pm25, int pm10, byte *msg) {
	int len=0;

	uint16_t val = (uint16_t) (pm25);
	msg[len++] = (O_AQ << 2) | 0x03;				// Last 2 bits are 0x03, so 4 bytes data
	msg[len++] = (val >> 8) & 0xFF;
	msg[len++] = val & 0xFF;
	
	val = (uint16_t) (pm10);
	msg[len++] = (val >> 8) & 0xFF;
	msg[len++] = val & 0xFF;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Airquality <"));
		Serial.print(len);
		Serial.print(F("> "));
		Serial.println(val);						// For readibility
	}
#endif
	return(len);
}


// --------------------------------------------------------------------------------
// Encode a Multi-Button sensor. This sensor is a concentrator that receives
// several sensor values over 433MHz and retransmits them over LoRa 
//	The LoRa sensor node contains the main address (LoRa id), the concentrated
// sendors have their own unique address/channel combination.
// --------------------------------------------------------------------------------
int	LoRaCode::eMbuttons(byte val, unsigned long address, unsigned short channel, byte *msg) {
	int len=0;
	msg[len++] = (O_MB << 2) | 0x00;			// Several bytes, do not care
	
	msg[len++] = val;							// First code the value
		
	msg[len++] = (address >> (8*3)) & 0xFF;		// Address
	msg[len++] = (address >> (8*2)) & 0xFF;
	msg[len++] = (address >> (8*1)) & 0xFF;
	msg[len++] = (address >> (8*0)) & 0xFF;
	
	msg[len++] = (channel >> (8*1)) & 0xFF;		// Channel
	msg[len++] = (channel >> (8*0)) & 0xFF;
	
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Multi-Button "));
		Serial.println(val);						// For readibility
	}
#endif	
	return(len);
}

// --------------------------------------------------------------------------------
// Moisture detection with two metal sensors in fluid or soil
//
// --------------------------------------------------------------------------------
int LoRaCode::eMoist(int val, byte *msg) {
	int len=0;
	msg[len++] = (O_MOIST << 2) | 0x00;				// Last 2 bits are 0x00, 1 byte
	msg[len++] = (val  / 4 ) & 0xFF;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Moisture "));
		Serial.println(val);						// For readibility
	}
#endif
	return(len);
}

// --------------------------------------------------------------------------------
// Luminescense detection with two metal sensors in fluid or soil
// The value to be decoded is 2-byte value which gives
// an integer resolution from 0 to 65535
// It is possible to add a third byte for extra resolution (2 decimals).
// --------------------------------------------------------------------------------
int LoRaCode::eLuminescense(float val, byte *msg) {
	int len=0;
	uint16_t lux = (uint16_t) (val);
	// now determine the fraction for a third byte
	msg[len++] = (O_LUMI << 2) | 0x01;				// Last 2 bits are 0x00, 2 bytes resolution
	msg[len++] = (lux >> (8*1)) & 0xFF;				// LSB
	msg[len++] = (lux >> (8*0)) & 0xFF;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Lumi "));
		Serial.println(val);						// For readibility
	}
#endif
	return(len);
}

int LoRaCode::eLuminescenseL(float val, byte *msg) {
	int len=0;
	uint16_t lux = (uint16_t) (val);
	// now determine the fraction for a third byte
	uint8_t frac = (uint8_t) ((val-lux) * 100);

	msg[len++] = (O_LUMI << 2) | 0x02;				// Last 2 bits are 0x00, 3 bytes resolution
	msg[len++] = (lux >> (8*1)) & 0xFF;				// LSB
	msg[len++] = (lux >> (8*0)) & 0xFF;
	msg[len++] = (frac) & 0xFF;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Lumi L="));
		Serial.print(lux);						// For readibility
		Serial.print('.');
		Serial.println(frac);						// For readibility
	}
#endif
	return(len);
}

// --------------------------------------------------------------------------------
// Distance detection 
// In this case we use 2 bytes for 0-65535 mm (which is more than we need)
// NOTE: The sensor will report in mm resolution, but the server side
// will decode in cm resolution with one decimal fraction (if available)
// --------------------------------------------------------------------------------
int LoRaCode::eDistance(int val, byte *msg) {
	int len=0;
	msg[len++] = (O_DIST << 2) | 0x01;				// Last 2 bits are 0x00, 2 bytes resolution
	msg[len++] = (val >> (8*1)) & 0xFF;	// LSB
	msg[len++] = (val >> (8*0)) & 0xFF;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Dist "));
		Serial.println(val);						// For readibility
	}
#endif
	return(len);
}


// --------------------------------------------------------------------------------
// Encode Gas concentration Value.
// Airquality through MQxx or AQxx sensors is measured through the Analog port
// and read as a 10-bit value (0-1024). We use a 2-byte value
// In order to tell the server which sensor is reporting (we can have several)
// --------------------------------------------------------------------------------
int LoRaCode::eGas(int val, byte *msg) {
	int len=0;
	msg[len++] = (O_GAS << 2) | 0x01;				// Last 2 bits are 0x01, 2 bytes
	msg[len++] = (val >> 8) & 0xFF;
	msg[len++] = val & 0xFF;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Gas "));
		Serial.println(val);						// For readibility
	}
#endif
	return(len);
}

// --------------------------------------------------------------------------------
// Encode Battery Value.
// Battery Voltage is between 2 and 4 Volts (sometimes 12V)
// We therefore multiply by 20, reported values are between 1 and 255, so
// sensor values between 0,05 and 12.75 Volts
// --------------------------------------------------------------------------------
int LoRaCode::eBattery(float val, byte *msg) {
	int len=0;
	int i = (byte) ((float)val *20);				// Value times 20
	msg[len++] = (O_BATT << 2) | 0x00;				// Last 2 bits are 0x00, one byte 
	msg[len++] = i;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Battery "));
		Serial.println((float)(i/20));				// For readibility
	}
#endif
	return(len);
}

// --------------------------------------------------------------------------------
// Encode AD Converter value for pin adc0 and adv1
// Battery Voltage is between 2 and 4 Volts (sometimes 12V)
// We therefore multiply by 20, reported values are between 1 and 255, so
// sensor values between 0,05 and 12.75 Volts
// --------------------------------------------------------------------------------
int LoRaCode::eAdc0(int val, byte *msg) {
	int len=0;
	msg[len++] = (O_ADC0 << 2) | 0x00;				// Last 2 bits are 0x00, 1 byte
	msg[len++] = (val  / 4 ) & 0xFF;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Adc0 "));
		Serial.println(val);						// For readibility
	}
#endif
	return(len);
}
int LoRaCode::eAdc1(int val, byte *msg) {
	int len=0;
	msg[len++] = (O_ADC1 << 2) | 0x00;				// Last 2 bits are 0x00, 1 byte
	msg[len++] = (val  / 4 ) & 0xFF;
#if DEBUG>0
	if (ldebug >=1) {
		Serial.print(F("lcode:: Add Adc1 "));
		Serial.println(val);						// For readibility
	}
#endif
	return(len);
}



// --------------------------------------------------------------------------------
// Function to encode the sensor values to the Payload message
// Input: Opcode and Value
// Output: msg
// return: Number of bytes added to msg
// --------------------------------------------------------------------------------
int LoRaCode::eVal (int opcode, byte *val, byte *msg) {
	int len=0;
	switch (opcode) {
		case O_TEMP:								// Temperature
			len += eTemperature((float) *val, msg);
		break;
		case O_HUMI:								// Humidity
			len += eHumidity((float) *val, msg);
		break;
		case O_AIRP:								// Airpressure
			len += eAirpressure((float) *val, msg);	
		break;
		case O_GPS:									// GPS short Info
			//len += eGps(lat, lng, msg);
		break;
		case O_PIR:									// PIR 
			len += ePir((int) *val, msg);
		break;

		case O_MOIST:								// Moisture
			len += eMoist((int) *val, msg);
		break;
		case O_LUMI:								// Luminescense
			len += eLuminescense((int) *val, msg);
		break;
		case O_BATT:								// Battery
			len += eBattery((float) *val, msg);
		break;
		default:
#if DEBUG>0
			Serial.println("lCode:: Error opcode not known");
#endif
		break;
	}
	return(len);
}

// --------------------------------------------------------------------------------
// Function to encode the Payload message (final step)
// This function will modify the first byte of the string to encode the length
// and the parity. Payload begins at position 1, byte 0 is reserved
// --------------------------------------------------------------------------------
bool LoRaCode::eMsg (byte * msg, int len) {
	unsigned char par = 0;
	if (len > 64) {
#if DEBUG>0
		Serial.println("lCodeMsg:: Error, string to long");
#endif
		return(false);
	}
#if DEBUG>0
	if (ldebug>=1) {Serial.print(F("LCodeMsg:: <")); Serial.print(len); Serial.print(F("> ")); }
#endif
	// else we probably have a good message
	msg[0] = ( len << 1 ) | (0x80);
	
	// Now we have to calculate the Parity for the message
	for (int i=0; i< len; i++) {
	  byte cc = msg[i];
	  par ^= cc;
	}
	
	// Now we have par as one byte and need to XOR the bits of that byte.
	unsigned char pp = 8;							// width of byte in #bits
	while (pp > 1) {
		par ^= par >> (pp/2);
		pp -= pp/2;
	}

	if (par & 0x01) {								// if odd number of 1's in string
#if DEBUG>0
		if (ldebug>=1) Serial.print(F(" odd "));
#endif
		msg[0] |= 0x01;								// Add another 1 to make parity even
	}
	else {
#if DEBUG>0
		if (ldebug>=1) Serial.print(F(" even "));
#endif
	}
	return(true);
}

// --------------------------------------------------------------------------------
// Print an encoded string
// --------------------------------------------------------------------------------
void LoRaCode::lPrint (byte *msg, int len) {
	Serial.print(F("lCode:  "));
	for (int i=0; i< len; i++) {
		if (msg[i]<10) Serial.print('0');
		Serial.print(msg[i],HEX);
		Serial.print(" ");
	}
	Serial.println();
}

// --------------------------------------------------------------------------------
// Decode first byte of message containing length
// --------------------------------------------------------------------------------
int LoRaCode::dLen (byte *msg) {
	if ( ! (msg[0] & 0x80 )) return(-1);		// Error
	int len = msg[0] & 0x7F;					// Blank out first bit which is always 1 with LoRaCode
	return (len >> 1);
}

// --------------------------------------------------------------------------------
// Decode message. One function for all, always returns a float
// We expect that the buffer is large enough and will contain all bytes required
// by that specific encoding
// XXX This function needs finishig (never used at the moment)
// PARAMETERS:
//	msg: Contains the message as a byte string
//	val: contains the decoded value as a float (xxx)
//	mode: Contains the opcode of the decoded command
// RETURN:
//	The number of bytes read from the buffer
// --------------------------------------------------------------------------------
int LoRaCode::dMsg (byte *msg, byte *val, byte *mode) {
	float res;
	byte len = msg[0] & 0x03;					// Last 2 bits
	*mode = (byte) (msg[0] >> 2);
	switch (*mode) {
		case O_TEMP:
			*val = (float) msg[1]- 100 + ( (float) msg[2] / 100);
			return(3);
		break;
		case O_HUMI:
			*val = (float) msg[1] / 2;
			return(2);
		break;
		case O_AIRP:
			*val = 0;
			return(2);
		break;
		case O_GPS:							// Returning one float does not work for GPS. function never used
			*val = 0;
			return(9);
		break;
		case O_GPSL:
			*val = 0;
			return(14);
		break;
		case O_PIR:
			*val = msg[1];
			return(2);
		break;
		case O_AQ:
			*val = 0;
			return(3);
		break;
		case O_BATT:
			*val = (float) msg[1] / 20;
			return(2);
		break;
		case O_STAT:
			*val = 0;
			return(1);
		case O_1CH:
			*val = msg[1];
			return(2);
		break;
		case O_SF:
			*val = msg[1];
			return(2);
		break;
		case O_TIM:						// Timing of the wait cyclus
			val[0] = msg[1];
			val[1] = msg[2];
			return(3);					// total lengt 2 bytes data + 1 byte opcode
		break;
		default:
			return(0);
	}
	return (0);
}

// Variable declaration
LoRaCode lcode;