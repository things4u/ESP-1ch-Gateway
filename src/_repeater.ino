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

#ifdef _TTNSERVER
#	error "Please undefine _TTNSERVER, for _REPEATER"
#endif //_TTNSERVER
#ifdef _THINGSERVER
#	error "Please undefine _THINGSERVER, for _REPEATER"
#endif //_THINGSERVER


// ------------------------ UP / DOWN -----------------------------	
// REPEATLORA()
// This function picks up a message upstream coming from the chip
// and sends it again downstream at a different frequency
// Parameters:
//	LUP:	Lora Up message coming int at freq IN
//	LDWN:	Lora Down message going to the transmitter at a 
//			different frequancy
//
// Incoming Message Format:
//	Byte 0:		// MHDR 0x40 == unconfirmed up message,
//	Byte 1-4:	Dev Address in LSBF sequence
//	Byte 5-
//
// Return:
//	1 when succesful
//	-<code> when failure
// ----------------------------------------------------------------
int repeatLora(struct LoraUp * LUP) {

	struct LoraDown lDown;								// We send ALMOST the same message down
	struct LoraDown * LDWN = & lDown;
	
	LDWN->tmst 		= LUP->tmst;
	LDWN->freq 		= freqs[(gwayConfig.ch-1)%3].dwnFreq;
	LDWN->size		= LUP->size;
	LDWN->sf		= LUP->sf;
	LDWN->powe		= 14;								// Default for normal frequencies
	LDWN->crc		= 1;
	LDWN->iiq		= 0x27;								// 0x40 when ipol true or 0x27 when false
	LDWN->imme		= false;

	//strncpy((char *)((* LDWN).payLoad), (char *)((* LUP).payLoad), (int)((* LUP).size));
	
	LDWN->payLoad	= LUP->payLoad;

	yield();										// During development, clean kernel.

	Serial.print("repeatLora:: size="); Serial.print(LDWN->size);
	Serial.print(", sf="); Serial.print(LDWN->sf);
	Serial.print(", iiq="); Serial.print(LDWN->iiq,HEX);
	Serial.print(", UP ch="); Serial.print(gwayConfig.ch,HEX);
	Serial.print(", DWN ch="); Serial.print((gwayConfig.ch-1)%3,HEX);
	Serial.print(", freq="); Serial.print(LDWN->freq);
	Serial.println();
	
	// Set the new channel to transmit on
	//setFreq(freqs[(gwayConfig.ch-1)%3].dwnFreq);	// Set OFREQ One lower (initially 0)

	// Once the frequency is set, we can transmit

	writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);			// MMM 200407 Reset
	writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);				// reset interrupt flags
	txLoraModem(LDWN);
	_event=1;
	_state=S_TXDONE;

	yield();
	startReceiver();											// (re)Start the receiver function


	// Check when len is not exceeding maximum length
	// char s[129];
	// sprintf(s,"repeatLora:: ch=%d, to=%d, data=", gwayConfig.ch, (gwayConfig.ch-1)%3) );
	Serial.print("repeatLora:: ch=");
	Serial.print(gwayConfig.ch);
	Serial.print(", to ch=");
	Serial.print((gwayConfig.ch-1)%3);
	Serial.print(", data=");
	
	for (int i=0; i< LDWN->size; i++) {
		Serial.print(LDWN->payLoad[i],HEX);
		Serial.print('.');
	}
	Serial.println();
	if (debug>=2) Serial.flush();
	
	return(1);
}


#endif //_REPEATER