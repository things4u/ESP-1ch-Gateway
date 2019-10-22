// sensor.h; 1-channel LoRa Gateway for ESP8266
// Copyright (c) 2016, 2017, 2018, 2019 Maarten Westenberg version for ESP8266
// Version 6.1.0
// Date: 2019-10-20
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

#if _TRUSTED_NODES >= 1
struct nodex {
	uint32_t id;				// This is the LoRa ID (coded in 4 bytes uint32_t
	char nm[32];				// Name of the node
};

// Add all your named and trusted nodes to this list
nodex nodes[] = {
	{ 0x2601148C , "lora-36 test node"  },						// F=0
	{ 0x00000000 , "lora-00 well known sensor" }				// F=0
};
#endif //_TRUSTED_NODES


// In some cases we like to decode the lora message at the single channel gateway.
// In thisase, we need the NkwSKey and the AppsSKey of the node so that we can decode
// its messages.
// Although this is probably overkill in normal gateway situations, it greatly helps
// in debugging the node messages before they reach the TTN severs.
//
#if _LOCALSERVER==1
struct codex  {
	uint32_t id;				// This is the device ID (coded in 4 bytes uint32_t
	char nm[32];				// A name string which is free to choose
	uint8_t nwkKey[16];			// The Network Session Key of 16 bytes
	uint8_t appKey[16];			// The Application Session Key of 16 bytes
};

// Sometimes we want to decode the sensor completely as we do in the TTN server
// This means that for all nodes we want to view the data of, we need to provide
// the AppsSKey and the NwkSKey

// Definition of all nodes that we want to decode locally on the gateway.
//
codex decodes[] = {
	{ 0x2601148C , "lora-36", 	// F=0
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 } 
	},
	{ 0x00000000 , "lora-00",	// F=0
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		{ 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 } 
	}					
};
#endif //_LOCALSERVER
