// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2020 Maarten Westenberg
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
// This file contains code for using the single channel gateway also as a repeater node. 
// Please note that for node to node communication your should change the polarity
// of messages.
//
// ============================================================================
		
#if _REPEATER==1

// Define input channel and output channel
#define _ICHAN 0
#define _OCHAN 1

#ifdef _TTNSERVER
#error "Please undefined _THINGSERVER, for REPEATER shutdown WiFi"
#endif

// Send a LoRa message out from the gateway transmitter
// XXX Maybe we should block the received ontul the message is transmitter

int sendLora(char *msg, int len) {
	// Check when len is not exceeding maximum length
	Serial.print("sendLora:: ");
	
	for (int i=0; i< len; i++) {
		Serial.print(msg[1],HEX);
		Serial.print('.');
	}
	
	if (debug>=2) Serial.flush();
	return(1);
}

#endif //_REPEATER==1