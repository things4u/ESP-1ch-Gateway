// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2020 Maarten Westenberg 
//
// Based on work done by Thomas Telkamp for Raspberry PI 1ch gateway
// and many others.
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License
// which accompanies this distribution, and is available at
// https://opensource.org/licenses/mit-license.php
//
// NO WARRANTY OF ANY KIND IS PROVIDED
//
// Author: Maarten Westenberg
//
// This file contains the functions to do management over UDP
// We could make use of the LoRa message function for the Gateway sensor
// itself. However the functions defined in this file are not sensor
// functions and activating them through the LoRa interface would add
// no value and make the code more complex.
//
// So advantage: Simple, and does not mess with TTN setup.
//
// Disadvantage of course is that you need to setup you own small backend
// function to exchange messages with the gateway, as TTN won't do this.
//
// XXX But, if necessary we can always add this later.

#if _GATEWAYMGT==1

#if !defined _THINGPORT
#error "The management functions needs _THINGPORT defined (and not over _TTNPORT)"
#endif



// ----------------------------------------------------------------------------
// This function gateway_mgt is called in the UDP Receive function after
// all well-known LoRa Gateway messages are scanned.
//
// As part of this function, we will listen for another set of messages
// that is defined in loraModem.h.
// All opCodes start with 0x1y for at leaving opcodes 0x00 to 0x0F to the
// pure Gateway protocol
//
// Incoming mesage format:
//	buf[0]-buf[2], These are 0x00 or dont care
//	buf[3], contains opcode
//	buf[4]-buf[7], contains parameter max. 4 bytes.
//
// Upstream Message format:
//
// ----------------------------------------------------------------------------
void gateway_mgt(uint8_t size, uint8_t *buff) {

	uint8_t opcode = buff[3];
	
	switch (opcode) {
		case MGT_RESET:
#			if _MONITOR>=1
				mPrint(F("gateway_mgt:: RESET"));
#			endif //_MONITOR
			// No further parameters, just reset the GWay
			setup();								// Call the sketch setup function
			// Send Ack to server
			
		break;
		case MGT_SET_SF:
#			if _MONITOR>=1
				mPrint(F("gateway_mgt:: SET SF"));
#			endif //_MONITOR
			// byte [4] contains desired SF code (7 for SF7 and 12 for SF12)
		break;
		case MGT_SET_FREQ:
#			if _MONITOR>=1
				mPrint(F("gateway_mgt:: SET FREQ"));
#			endif //_MONITOR
			// Byte [4] contains index of Frequency
		break;
		default:
#			if _MONITOR>=1
				mPrint(F("gateway_mgt:: Unknown UDP code=") + String(opcode) ); 
#			endif
			return;
		break;
	}
}

#endif //_GATEWAYMGT==1