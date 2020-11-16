// sensor.ino; 1-channel LoRa Gateway for ESP8266 and ESP32
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
// This file contains code for using the single channel gateway also as a sensor node. 
// Please specify the DevAddr and the AppSKey below (and on your LoRa backend).
// Also you will have to choose what sensors to forward to your application.
//
// Note: disable sensors not used in configGway.h
//	- The GPS is included on TTGO T-Beam ESP32 boards by default.
//	- The battery sensor works by connecting the VCC pin to A0 analog port
// ============================================================================
	
#if _GATEWAYNODE==1

#include "LoRaCode.h"

unsigned char DevAddr[4]  = _DEVADDR ;				// see configGway.h


// Only used by GPS sensor code
#if _GPS==1
// ----------------------------------------------------------------------------
// Smartdelay is a function to delay processing but in the loop get info 
// from the GPS device
// ----------------------------------------------------------------------------
void smartDelay(uint32_t ms)                
{
	uint32_t start = millis();
	do
	{
		while (sGps.available()) {
			gps.encode(sGps.read());
		}
		yield();									// MMM Maybe  enable to fill buffer
	} while (millis() - start < ms);
}
#endif //_GPS





// ----------------------------------------------------------------------------
// LoRaSensors() is a function that puts sensor values in the MACPayload and 
// sends these values up to the server. For the server it is impossible to know 
// whther or not the message comes from a LoRa node or from the gateway.
//
// The example code below adds a battery value in lCode (encoding protocol) but
// of-course you can add any byte string you wish
//
// Parameters: 
//	- buf: contains the buffer to put the sensor values in (max==xx);
// Returns:
//	- The amount of sensor characters put in the buffer
//
// NOTE: The code in LoRaSensors() is provided as an example only.
//	The amount of sensor values as well as their message layout may differ
//	for each implementation.
//	Also, the message format used by this gateway is LoraCode, a message format
//	developed by me for sensor values. Each value is uniquely coded with an
//	id and a value, and the total message contains its length (less than 64 bytes)
//	and a parity value in byte[0] bit 7.
// ----------------------------------------------------------------------------
int LoRaSensors(uint8_t *buf) {

#	if defined(_LCODE)
#		if defined(_RAW) 
#		error "Only define ONE encoding in configNode.h, _LOCDE or _RAW"
#		endif

		String response="";
		uint8_t tchars = 1;
		buf[0] = 0x86;								// 134; User code <lCode + len==3 + Parity
		
#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_MAIN)) {
			response += "LoRaSensors:: ";
		}
#		endif //_MONITOR

		// GPS sensor is the second server we check for
#		if _GPS==1
			smartDelay(10);							// Use GPS to return fast!
			if ((millis() > 5000) && (gps.charsProcessed() < 10)) {
#				if _MONITOR>=1
					mPrint("ERROR: No GPS data received: check wiring");
#				endif //_MONITOR
				return(0);
			}
			// Assuming we have a value, put it in the buf
			// The layout of this message is specific to the user,
			// so adapt as needed.

			// Use lcode to code messages to server
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_MAIN)) {
				response += " Gps lcode:: lat="+String(gps.location.lat())+", lng="+String(gps.location.lng())+", alt="+String(gps.altitude.feet()/3.2808)+", sats="+String(gps.satellites.value());
			}
#			endif //_MONITOR
			tchars += lcode.eGpsL(gps.location.lat(), gps.location.lng(), gps.altitude.value(), gps.satellites.value(), buf + tchars);
#		endif //_GPS

#		if _BATTERY==1
#			if defined(ARDUINO_ARCH_ESP8266) || defined(ESP32)
				// For ESP there is no standard battery library
				// What we do is to measure GPIO35 pin which has a 100K voltage divider
				pinMode(35, INPUT);
				float volts=3.3 * analogRead(35) / 4095 * 2;	// T_Beam connects to GPIO35
#			else
				// For ESP8266 no sensor defined
				float volts=0;
#			endif // ARDUINO_ARCH_ESP8266 || ESP32
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_MAIN)){
				response += ", Battery V="+String(volts);
			}
#			endif //_MONITOR

			tchars += lcode.eBattery(volts, buf + tchars);
#		endif //_BATTERY


		// If all sensor data is encoded, we encode the buffer
		lcode.eMsg(buf, tchars);								// Fill byte 0 with bytecount and Parity
		
#		if _MONITOR>=1
			mPrint(response);
#		endif //_MONITOR

// Second encoding option is RAW format. 
//
// We do not use the lcode format but write all the values to the output
// buffer and we need to get them in sequence out off the buffer.
#	elif defined(_RAW)
		uint8_t tchars = 0;

		// GPS sensor is the second server we check for
#		if _GPS==1
			smartDelay(10);
			if (millis() > 5000 && gps.charsProcessed() < 10) {
#				if _MONITOR>=1
					mPrint("ERROR: No GPS data received: check wiring");
#				endif //_MONITOR
				return(0);
			}
			// Raw coding of LoRa messages to server so add the GPS data raw to the string
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_MAIN)){
				mPrint("Gps raw:: lat="+String(gps.location.lat())+", lng="+String(gps.location.lng())+", alt="+String(gps.altitude.feet()/3.2808)+", sats="+String(gps.satellites.value()) );
				//mPrint("Gps raw:: sizeof double="+String(sizeof(double)) );
			}
#			endif //_MONITOR
			// Length of lat and lng is double
			double lat = gps.location.lat();
			double lng = gps.location.lng();
			double alt = gps.altitude.feet() / 3.2808;
			memcpy((buf+tchars), &lat, sizeof(double)); tchars += sizeof(double);
			memcpy((buf+tchars), &lng, sizeof(double)); tchars += sizeof(double);
			memcpy((buf+tchars), &alt, sizeof(double)); tchars += sizeof(double);
#		endif //_GPS

#		if _BATTERY==1
#			if defined(ARDUINO_ARCH_ESP8266) || defined(ESP32)
				// For ESP there is no standard battery library
				// What we do is to measure GPIO35 pin which has a 100K voltage divider
				pinMode(35, INPUT);
				float volts=3.3 * analogRead(35) / 4095 * 2;	// T_Beam connects to GPIO35
#			else
				// For ESP8266 no sensor defined
				float volts=0;
#			endif // ARDUINO_ARCH_ESP8266 || ESP32
			memcpy((buf+tchars), &volts, sizeof(float)); tchars += sizeof(float);
			
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_MAIN)){
				mPrint("Battery raw="+String(volts));
			}
#			endif //_MONITOR
#		endif //_BATTERY


// If neither _LCODE or _RAW is defined this is an error
#	else
#		error "Please define an encoding format as in configNode.h"
#	endif


// GENERAL part
#	if _DUSB>=1 && _GPS==1
	if ((debug>=2) && (pdebug & P_MAIN)) {
		Serial.print("GPS sensor");
		Serial.print("\tLatitude  : ");
		Serial.println(gps.location.lat(), 5);
		Serial.print("\tLongitude : ");
		Serial.println(gps.location.lng(), 4);
		Serial.print("\tSatellites: ");
		Serial.println(gps.satellites.value());
		Serial.print("\tAltitude  : ");
		Serial.print(gps.altitude.feet() / 3.2808);
		Serial.println("M");
		Serial.print("\tTime      : ");
		Serial.print(gps.time.hour());
		Serial.print(":");
		Serial.print(gps.time.minute());
		Serial.print(":");
		Serial.println(gps.time.second());
	}
#	endif //_DUSB _GPS

	return(tchars);	// return the number of bytes added to payload
}	




// ----------------------------------------------------------------------------
// XOR()
// perform x-or function for buffer and key
// Since we do this ONLY for keys and X, Y we know that we need to XOR 16 bytes.
//
// ----------------------------------------------------------------------------
void mXor(uint8_t *buf, uint8_t *key) 
{
	for (uint8_t i = 0; i < 16; ++i) buf[i] ^= key[i];
}


// ----------------------------------------------------------------------------
// SHIFT-LEFT
// Shift the buffer buf left one bit
// Parameters:
//	- buf: An array of uint8_t bytes
//	- len: Length of the array in bytes
// ----------------------------------------------------------------------------
void shift_left(uint8_t * buf, uint8_t len) 
{
    while (len--) {
        uint8_t next = len ? buf[1] : 0;			// len 0 to 15

        uint8_t val = (*buf << 1);
        if (next & 0x80) val |= 0x01;
        *buf++ = val;
    }
}


// ----------------------------------------------------------------------------
// generate_subkey
// RFC 4493, para 2.3
// -----------------------------------------------------------------------------
void generate_subkey(uint8_t *key, uint8_t *k1, uint8_t *k2) 
{

	memset(k1, 0, 16);								// Fill subkey1 with 0x00
	
	// Step 1: Assume k1 is an all zero block
	AES_Encrypt(k1,key);
	
	// Step 2: Analyse outcome of Encrypt operation (in k1), generate k1
	if (k1[0] & 0x80) {
		shift_left(k1,16);
		k1[15] ^= 0x87;
	}
	else {
		shift_left(k1,16);
	}
	
	// Step 3: Generate k2
	for (int i=0; i<16; i++) k2[i]=k1[i];
	
	if (k1[0] & 0x80) {								// use k1(==k2) according rfc 
		shift_left(k2,16);
		k2[15] ^= 0x87;
	}
	else {
		shift_left(k2,16);
	}
	
	// step 4: Done, return k1 and k2
	return;
}


// ----------------------------------------------------------------------------
// MICPACKET()
// Provide a valid MIC 4-byte code (par 2.4 of spec, RFC4493)
// 		see also https://tools.ietf.org/html/rfc4493
//
// Although our own handler may choose not to interpret the last 4 (MIC) bytes
// of a PHYSPAYLOAD physical payload message of in internal sensor,
// The official TTN (and other) backends will interpret the complete message and
// conclude that the generated message is bogus.
// So we will really simulate internal messages coming from the -1ch gateway
// to come from a real sensor and append 4 MIC bytes to every message that are 
// perfectly legimate
// Parameters:
//	- data:			uint8_t array of bytes = ( MHDR | FHDR | FPort | FRMPayload )
//	- len:			8=bit length of data, normally less than 64 bytes
//	- FrameCount:	16-bit framecounter
//	- dir:			0=up, 1=down
//
// B0 = ( 0x49 | 4 x 0x00 | Dir | 4 x DevAddr | 4 x FCnt |  0x00 | len )
// MIC is cmac [0:3] of ( aes128_cmac(NwkSKey, B0 | Data )
//
// ----------------------------------------------------------------------------
uint8_t micPacket(uint8_t *data, uint8_t len, uint16_t FrameCount, uint8_t * NwkSKey, uint8_t dir)
{


	//uint8_t NwkSKey[16] = _NWKSKEY;
	uint8_t Block_B[16];
	uint8_t X[16];
	uint8_t Y[16];
	
	// ------------------------------------
	// build the B block used by the MIC process
	Block_B[0]= 0x49;						// 1 byte MIC code
			
	Block_B[1]= 0x00;						// 4 byte 0x00
	Block_B[2]= 0x00;
	Block_B[3]= 0x00;
	Block_B[4]= 0x00;
	
	Block_B[5]= dir;						// 1 byte Direction
	
	Block_B[6]= DevAddr[3];					// 4 byte DevAddr
	Block_B[7]= DevAddr[2];
	Block_B[8]= DevAddr[1];
	Block_B[9]= DevAddr[0];
	
	Block_B[10]= (FrameCount & 0x00FF);		// 4 byte FCNT
	Block_B[11]= ((FrameCount >> 8) & 0x00FF);
	Block_B[12]= 0x00; 						// Frame counter upper Bytes
	Block_B[13]= 0x00;						// These are not used so are 0
	
	Block_B[14]= 0x00;						// 1 byte 0x00
	
	Block_B[15]= len;						// 1 byte len
	
	// ------------------------------------
	// Step 1: Generate the subkeys
	//
	uint8_t k1[16];
	uint8_t k2[16];
	generate_subkey(NwkSKey, k1, k2);
	
	// ------------------------------------
	// Copy the data to a new buffer which is prepended with Block B0
	//
	uint8_t micBuf[len+16];					// B0 | data
	for (uint8_t i=0; i<16; i++) micBuf[i]=Block_B[i];
	for (uint8_t i=0; i<len; i++) micBuf[i+16]=data[i];
	
	// ------------------------------------
	// Step 2: Calculate the number of blocks for CMAC
	//
	uint8_t numBlocks = len/16 + 1;			// Compensate for B0 block
	if ((len % 16)!=0) numBlocks++;			// If we have only a part block, take it all
	
	// ------------------------------------
	// Step 3: Calculate padding is necessary
	//
	uint8_t restBits = len%16;				// if numBlocks is not a multiple of 16 bytes
	
	
	// ------------------------------------
	// Step 5: Make a buffer of zeros
	//
	memset(X, 0, 16);
	
	// ------------------------------------
	// Step 6: Do the actual encoding according to RFC
	//
	for(uint8_t i= 0x0; i < (numBlocks - 1); i++) {
		for (uint8_t j=0; j<16; j++) Y[j] = micBuf[(i*16)+j];
		mXor(Y, X);
		AES_Encrypt(Y, NwkSKey);
		for (uint8_t j=0; j<16; j++) X[j] = Y[j];
	}
	

	// ------------------------------------
	// Step 4: If there is a rest Block, padd it
	// Last block. We move step 4 to the end as we need Y
	// to compute the last block
	// 
	if (restBits) {
		for (uint8_t i=0; i<16; i++) {
			if (i< restBits) Y[i] = micBuf[((numBlocks-1)*16)+i];
			if (i==restBits) Y[i] = 0x80;
			if (i> restBits) Y[i] = 0x00;
		}
		mXor(Y, k2);
	}
	else {
		for (uint8_t i=0; i<16; i++) {
			Y[i] = micBuf[((numBlocks-1)*16)+i];
		}
		mXor(Y, k1);
	}
	mXor(Y, X);
	AES_Encrypt(Y,NwkSKey);
	
	// ------------------------------------
	// Step 7: done, return the MIC size. 
	// Only 4 bytes are returned (32 bits), which is less than the RFC recommends.
	// We return by appending 4 bytes to data, so there must be space in data array.
	//
	data[len+0]=Y[0];
	data[len+1]=Y[1];
	data[len+2]=Y[2];
	data[len+3]=Y[3];
	
	yield();										// MMM to avoid crashes
	
	return 4;
}


#if _CHECK_MIC==1
// ----------------------------------------------------------------------------
// CHECKMIC
// Function to check the MIC computed for existing messages and for new messages
// Parameters:
//	- buf: LoRa buffer to check in bytes, last 4 bytes contain the MIC
//	- len: Length of buffer in bytes
//	- key: Key to use for MIC. Normally this is the NwkSKey
//
// ----------------------------------------------------------------------------
void checkMic(uint8_t *buf, uint8_t len, uint8_t *key)
{
	uint8_t cBuf[len+1];
	uint8_t NwkSKey[16] = _NWKSKEY;

#	if _MONITOR>=1
	if ((debug>=2) && (pdebug & P_RX)) {
		String response = "";
		for (int i=0; i<len; i++) { 
			printHexDigit(buf[i], response); 
			response += ' '; 
		}
		mPrint("old="+response);
	}	
#	endif //_MONITOR

	for (int i=0; i<len-4; i++) {
		cBuf[i] = buf[i];
	}
	len -=4;
	
	uint16_t FrameCount = ( cBuf[7] * 256 ) + cBuf[6];
	len += micPacket(cBuf, len, FrameCount, NwkSKey, 0);
	
	if ((debug>=2) && (pdebug & P_RX)) {
		String response = "";

		for (int i=0; i<len; i++) { 
			printHexDigit(cBuf[i],response); 
			response += " "; 
		}
		mPrint("new="+response);
	}
	// Mic is only checked, but len is not corrected
}
#endif //_CHECK_MIC

// ----------------------------------------------------------------------------
// SENSORPACKET
// The gateway may also have local sensors that need reporting.
// We will generate a message in gateway-UDP format for upStream messaging
// so that for the backend server it seems like a LoRa node has reported a
// sensor value.
//
// NOTE: We do not need ANY LoRa functions here since we are on the gateway.
// We only need to send a gateway message upstream that looks like a node message.
//
// NOTE:: This function does encrypt the sensorpayload, and the backend
//		picks it up fine as decoder thinks it is a MAC message.
//
// Par 4.0 LoraWan spec:
//	PHYPayload =	( MHDR | MACPAYLOAD | MIC ) 
// which is equal to
//					( MHDR | ( FHDR | FPORT | FRMPAYLOAD ) | MIC )
//
//	This function makes the totalpackage and calculates MIC
// The maximum size of the message is: 12 + ( 9 + 2 + 64 ) + 4	
// So message size should be lass than 128 bytes if Payload is limited to 64 bytes.
//
// return value:
//	- On success returns the number of bytes to send
//	- On error returns -1
// ----------------------------------------------------------------------------
int sensorPacket() {

	uint8_t buff_up[512];								// Declare buffer here to avoid exceptions
	uint8_t message[64]={ 0 };							// Payload, init to 0
	//uint8_t mlength = 0;
	uint8_t NwkSKey[16] = _NWKSKEY;
	uint8_t AppSKey[16] = _APPSKEY;
	uint8_t DevAddr[4]  = _DEVADDR;
	
	// Init the other LoraUp fields
	
	struct LoraUp LUP;
	
	LUP.sf = 8;											// Send with SF8
	LUP.prssi = -50;
	LUP.rssicorr = 139;
	LUP.snr = 0;
	
	// In the next few bytes the fake LoRa message must be put
	// PHYPayload = MHDR | MACPAYLOAD | MIC
	// MHDR, 1 byte
	// MIC, 4 bytes
	
	// ------------------------------
	// MHDR (Para 4.2), bit 5-7 MType, bit 2-4 RFU, bit 0-1 Major
	LUP.payLoad[0] = 0x40;								// MHDR 0x40 == unconfirmed up message,
														// FRU and major are 0
	
	// -------------------------------
	// FHDR consists of 4 bytes addr, 1 byte Fctrl, 2 byte FCnt, 0-15 byte FOpts
	// We support ABP addresses only for Gateway sensors
	LUP.payLoad[1] = DevAddr[3];						// Last byte[3] of address
	LUP.payLoad[2] = DevAddr[2];
	LUP.payLoad[3] = DevAddr[1];
	LUP.payLoad[4] = DevAddr[0];						// First byte[0] of Dev_Addr
	
	LUP.payLoad[5] = 0x00;								// FCtrl is normally 0
	LUP.payLoad[6] = frameCount % 0x100;				// LSB
	LUP.payLoad[7] = frameCount / 0x100;				// MSB

	// -------------------------------
	// FPort, either 0 or 1 bytes. Must be != 0 for non MAC messages such as user payload
	//
	LUP.payLoad[8] = 0x01;								// FPort must not be 0
	LUP.size  = 9;
	
	// FRMPayload; Payload will be AES128 encoded using AppSKey
	// See LoRa spec para 4.3.2
	// You can add any byte string below based on you personal choice of sensors etc.
	//
	
	// Payload bytes in this example are encoded in the LoRaCode(c) format
	uint8_t PayLength = LoRaSensors((uint8_t *)(LUP.payLoad + LUP.size));

#if _DUSB>=1
	if ((debug>=2) && (pdebug & P_RADIO)) {
		String response="";
		Serial.print(F("old: "));
		for (int i=0; i<PayLength; i++) {
			Serial.print(LUP.payLoad[i],HEX);
			Serial.print(' ');
		}
		Serial.println();
	}
#endif	//_DUSB
	
	// we have to include the AES functions at this stage in order to generate LoRa Payload.
	uint8_t CodeLength = encodePacket(
		(uint8_t *)(LUP.payLoad + LUP.size), 
		PayLength,
		(uint16_t)frameCount, 
		DevAddr, 
		AppSKey, 
		0
	);

#if _DUSB>=1
	if ((debug>=2) && (pdebug & P_RADIO)) {
		Serial.print(F("new: "));
		for (int i=0; i<CodeLength; i++) {
			Serial.print(LUP.payLoad[i],HEX);
			Serial.print(' ');
		}
		Serial.println();
	}
#endif //_DUSB

	LUP.size += CodeLength;								// length inclusive sensor data
	
	// MIC, Message Integrity Code
	// As MIC is used by TTN (and others) we have to make sure that
	// framecount is valid and the message is correctly encrypted.
	// Note: Until MIC is done correctly, TTN does not receive these messages
	//		 The last 4 bytes are MIC bytes.
	//
	LUP.size += micPacket((uint8_t *)(LUP.payLoad), LUP.size, (uint16_t)frameCount, NwkSKey, 0);

#if _DUSB>=1
	if ((debug>=2) && (pdebug & P_RADIO)) {
		Serial.print(F("mic: "));
		for (int i=0; i<LUP.size; i++) {
			Serial.print(LUP.payLoad[i],HEX);
			Serial.print(' ');
		}
		Serial.println();
	}
#endif //_DUSB

	// So now our package is ready, and we can send it up through the gateway interface
	// Note: Be aware that the sensor message (which is bytes) in message will be
	// be expanded if the server expects JSON messages.
	// Note2: We fake this sensor message when sending
	//
	uint16_t buff_index = buildPacket(buff_up, &LUP, true);
	
	frameCount++;
	statc.msg_ttl++;					// XXX Should we count sensor messages as well?
	statc.msg_sens++;
	switch(gwayConfig.ch) {
		case 0: statc.msg_sens_0++; break;
		case 1: statc.msg_sens_1++; break;
		case 2: statc.msg_sens_2++; break;
	}
	
	// In order to save the memory, we only write the framecounter
	// to EEPROM every 10 values. It also means that we will invalidate
	// 10 value when restarting the gateway.
	// NOTE: This means that preferences are NOT saved unless >=10 messages have been received.
	//
	if ((frameCount % 10)==0) writeGwayCfg(_CONFIGFILE, &gwayConfig );
	
	if (buff_index > 512) {
		if (debug>0) 
			mPrint("sensorPacket:: ERROR buffer size too large");
		return(-1);
	}

#ifdef _TTNSERVER	
	if (!sendUdp(ttnServer, _TTNPORT, buff_up, buff_index)) {
		return(-1);
	}
#endif //_TTNSERVER

#ifdef _THINGSERVER
	if (!sendUdp(thingServer, _THINGPORT, buff_up, buff_index)) {
		return(-1);
	}
#endif //_THINGSERVER

#if _DUSB>=1
	// If all is right, we should after decoding (which is the same as encoding) get
	// the original message back again.
	if ((debug>=2) && (pdebug & P_RADIO)) {
		CodeLength = encodePacket(
			(uint8_t *)(LUP.payLoad + 9), 
			PayLength, 
			(uint16_t)frameCount-1, 
			DevAddr, 
			AppSKey, 
			0
		);
		
		Serial.print(F("rev: "));
		for (int i=0; i<CodeLength; i++) {
			Serial.print(LUP.payLoad[i],HEX);
			Serial.print(' ');
		}
		Serial.print(F(", addr="));
		for (int i=0; i<4; i++) {
			Serial.print(DevAddr[i],HEX);
			Serial.print(' ');
		}
		Serial.println();
	}
#endif //_DUSB

	if (gwayConfig.cad) {
		// Set the state to CAD scanning after sending a packet
		_state = S_SCAN;						// Inititialise scanner
		sf = SF7;
		cadScanner();
	}
	else {
		// Reset all RX lora stuff
		_state = S_RX;
		rxLoraModem();	
	}
		
	return(buff_index);
}

#endif //_GATEWAYNODE==1


#if (_GATEWAYNODE==1) || (_LOCALSERVER>=1)
// ----------------------------------------------------------------------------
// ENCODEPACKET
// In Sensor mode, we have to encode the user payload before sending.
// The same applies to decoding packages in the payload for _LOCALSERVER.
// The library files for AES are added to the library directory in AES.
// For the moment we use the AES library made by ideetron as this library
// is also used in the LMIC stack and is small in size.
//
// The function below follows the LoRa spec exactly.
//
// The resulting mumber of Bytes is returned by the functions. This means
// 16 bytes per block, and as we add to the last block we also return 16
// bytes for the last block.
//
// The LMIC code does not do this, so maybe we shorten the last block to only
// the meaningful bytes in the last block. This means that encoded buffer
// is exactly as big as the original message.
//
// NOTE:: Be aware that the LICENSE of the used AES library files 
//	that we call with AES_Encrypt() is GPL3. It is used as-is,
//  but not part of this code.
//
// cmac = aes128_encrypt(K, Block_A[i])
//
// Parameters:
//	Data:		Data to encoide
//	DataLength:	Length of the data field
//	FrameCount:	xx
//	DevAddr:	Device ID
//	AppSKey:	Device specific
//	Direction:	Uplink==0, e.g. receivePakt(), semsorPacket(),
// ----------------------------------------------------------------------------
uint8_t encodePacket(uint8_t *Data, uint8_t DataLength, uint16_t FrameCount, uint8_t *DevAddr, uint8_t *AppSKey, uint8_t Direction)
{
	//unsigned char AppSKey[16] = _APPSKEY ;	// see configGway.h
	uint8_t i, j;
	uint8_t Block_A[16];
	uint8_t bLen=16;						// Block length is 16 except for last block in message
		
	uint8_t restLength = DataLength % 16;	// We work in blocks of 16 bytes, this is the rest
	uint8_t numBlocks  = DataLength / 16;	// Number of whole blocks to encrypt
	if (restLength>0) numBlocks++;			// And add block for the rest if any

	for(i = 1; i <= numBlocks; i++) {
		Block_A[0] = 0x01;
		
		Block_A[1] = 0x00; 
		Block_A[2] = 0x00; 
		Block_A[3] = 0x00; 
		Block_A[4] = 0x00;

		Block_A[5] = Direction;				// 0 is uplink

		Block_A[6] = DevAddr[3];			// Only works for and with ABP
		Block_A[7] = DevAddr[2];
		Block_A[8] = DevAddr[1];
		Block_A[9] = DevAddr[0];

		Block_A[10] = (FrameCount & 0x00FF);
		Block_A[11] = ((FrameCount >> 8) & 0x00FF);
		Block_A[12] = 0x00; 				// Frame counter upper Bytes
		Block_A[13] = 0x00;					// These are not used so are 0

		Block_A[14] = 0x00;

		Block_A[15] = i;

		// Encrypt and calculate the S
		AES_Encrypt(Block_A, AppSKey);
		
		// Last block? set bLen to rest
		if ((i == numBlocks) && (restLength>0)) bLen = restLength;
		
		for(j = 0; j < bLen; j++) {
			*Data = *Data ^ Block_A[j];
			Data++;
		}
	}

	//return(numBlocks*16);			// Do we really want to return all 16 bytes in lastblock
	return(DataLength);				// or only 16*(numBlocks-1)+bLen;
}

#endif //_GATEWAYNODE || _LOCALSERVER