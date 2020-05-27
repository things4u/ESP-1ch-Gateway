// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2020 Maarten Westenberg version for ESP8266
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
// This file contains the LoRa modem specific code enabling to receive
// and transmit packages/messages.
// ========================================================================================


// ------------------------------- DOWN ----------------------------------------
//
// Prepare DOWN a LoRa packet in down buffer. This function does all the 
// decoding of the server message and prepares a Payload buffer.
// The payload is actually transmitted by the sendPkt() function.
// This function is used for regular downstream messages and for JOIN_ACCEPT
// messages.
// NOTE: This is not an interrupt function, but is started by loop().
// The _status is set an the end of the function to TX and in _stateMachine
// function the actual transmission function is executed.
// The LoraDown.tmst contains the timestamp that the tranmission should finish (Node start reading).
// ----------------------------------------------------------------------------
int sendPacket(uint8_t *buf, uint8_t length) 
{
	// Received package with Meta Data (for example):
	// codr	: "4/5"
	// data	: "Kuc5CSwJ7/a5JgPHrP29X9K6kf/Vs5kU6g=="	// for example
	// freq	: 868.1 									// 868100000 default
	// ipol	: true/false
	// modu : "LORA"
	// powe	: 14										// Set by default
	// rfch : 0											// Set by default
	// size	: 21
	// tmst : 1800642 									// for example
	// datr	: "SF7BW125"
	// imme :											// Immediately transfer
	
	// 12-byte header;
	//		HDR (1 byte)
	//		
	// Data Reply for JOIN_ACCEPT as sent by server:
	//		AppNonce (3 byte)
	//		NetID (3 byte)
	//		DevAddr (4 byte) [ 31..25]:NwkID , [24..0]:NwkAddr
 	//		DLSettings (1 byte)
	//		RxDelay (1 byte)
	//		CFList (fill to 16 bytes)
			
	int i=0;
	char * bufPtr = (char *) (buf);
	buf[length] = 0;
	
#	if _MONITOR>=1
	if (( debug>=1) && (pdebug & P_TX)) {
		mPrint("Dwn sendPacket:: " + String((char *)buf));
	}
#	endif //_MONITOR

	// Use JSON to decode the string after the first 4 bytes.
	// The data for the node is in the "data" field. This function destroys original buffer
	auto error = deserializeJson(jsonBuffer, bufPtr);		
	if (error) {
#		if _MONITOR>=1
		if ((debug>=0) && (pdebug & P_TX)) {
			mPrint("Dwn sendPacket:: ERROR: Json Decode: " + String(bufPtr) );
		}
#		endif //_MONITOR
		return(-1);
	}
	
	// Meta Data sent by server (example)
	// {"txpk":{"codr":"4/5","data":"YCkEAgIABQABGmIwYX/kSn4Y","freq":868.1,"ipol":true,"modu":"LORA","powe":14,"rfch":0,"size":18,"tmst":1890991792,"datr":"SF7BW125"}}

	// Used in the protocol of Gateway:

	JsonObject root		= jsonBuffer.as<JsonObject>();	// 191111 Avoid Crashes

	LoraDown.tmst		= (uint32_t) root["txpk"]["tmst"].as<unsigned long>();
	const char * data	= root["txpk"]["data"];			// Downstream Payload
	uint8_t psize		= root["txpk"]["size"];			// Payload size
	bool ipol			= root["txpk"]["ipol"];

	const char * datr	= root["txpk"]["datr"];			// eg "SF7BW125"
	//const char * modu	= root["txpk"]["modu"];			// =="LORA"
	//const char * codr	= root["txpk"]["codr"];			// e.g. "4/5"
	if (root["txpk"].containsKey("imme") ) {
		LoraDown.imme = root["txpk"]["imme"];			// Immediate Transmit (tmst don't care)
	}
	if (root["txpk"].containsKey("powe") ) {
		uint8_t powe	= root["txpk"]["powe"];			// power, e.g. 14 or 27
	}

	if ( data != NULL ) {
#		if _MONITOR>=1
		if ((debug>=2) && (pdebug & P_TX)) { 
			mPrint("sendPacket:: data=" + String(data)); 
		}
#		endif //_MONITOR
	}
	else {												// There is data!
#		if _MONITOR>=1
		if ((debug>=0) && (pdebug & P_TX)) {
			mPrint("sendPacket:: ERROR: data is NULL");
		}
#		endif //_MONITOR
		return(-1);
	}

	LoraDown.sfTx = atoi(datr+2);						// Convert "SF9BW125" or what is received from gateway to number
	LoraDown.iiq = (ipol? 0x40: 0x27);					// if ipol==true 0x40 else 0x27
	LoraDown.crc = 0x00;								// switch CRC off for TX
	LoraDown.payLength = base64_dec_len((char *) data, strlen(data));// Length of the Payload data	
	base64_decode((char *) payLoad, (char *) data, strlen(data));	// Fill payload w decoded message

	// Compute wait time in microseconds
	int32_t w = (int32_t) (LoraDown.tmst - micros());	// Wait Time compute

// _STRICT_1CH determines how we will react on downstream messages.
//
// If _STRICT==1, we will answer (in the RX1 timeslot) on the frequency we receive on.
// We will anser in RX2 in the Spreading Factor set by _RX2_SF (set in configGway.h)
// This way, we can better communicate as a single gateway machine
//
#if _STRICT_1CH == 1
	// RX1 is requested frequency
	// RX2 is SF _RX2_SF probably SF9
	// If possible use RX1 timeslot as this is our frequency.
	// Do not use RX2 or JOIN2 as they contain other frequencies (868.5 MHz)

	LoraDown.powe	= 14;								// On all freqs except 869.5MHz power is limited
	LoraDown.freq	= freqs[gwayConfig.ch].dwnFreq;		// Use the requestor Down frequency (always)

#else
// elif _STRICT_1CH == 0, we will receive messags from the TTN gateway presumably on SF9/869.5MHz
// And since the Gateway is a single channel gateway, and its nodes are probably
// single channel too. They will not listen to that frequency at all.
// Pleae note that this parameter is more for nodes (that cannot change freqs)
// than for gateways.
// We will probably answer in RX with RF==12 and use special answer frequency
//
	LoraDown.powe	= root["txpk"]["powe"];
	const float ff	= root["txpk"]["freq"];				// eg 869.525
	// convert double frequency (MHz) into uint32_t frequency in Hz.
	LoraDown.freq = (uint32_t) ((uint32_t)((ff+0.000035)*1000)) * 1000;

#endif //_STRICT_1CH

	yield();
	
	LoraDown.payLoad = payLoad;				

#	if _MONITOR>=1
	if ((debug>=2) && (pdebug & P_TX)) {
		mPrint("Dwn sendPacket:: TX tmst=" + String(LoraDown.tmst));
	}
#	endif //_MONITOR

	if (LoraDown.payLength != psize) {
#		if _MONITOR>=1
		if (debug>=0) {
			mPrint("Dwn sendPacket:: WARNING payLength=" + String(LoraDown.payLength) + ", psize=" + String(psize) );
		}
#		endif //_MONITOR
	}
#	if _MONITOR>=1
	else if ((debug >= 2) && (pdebug & P_TX)) {
		Serial.print(F("T Payload="));
		for (i=0; i<LoraDown.payLength; i++) {
			Serial.print(payLoad[i],HEX); 
			Serial.print(':'); 
		}
		Serial.println();
	}
#	endif //_MONITOR

	// Update downstream statistics
	statc.msg_down++;
	switch(statr[0].ch) {
		case 0: statc.msg_down_0++; break;
		case 1: statc.msg_down_1++; break;
		case 2: statc.msg_down_2++; break;
	}

	// All data is in Payload and parameters and need to be transmitted.
	// The function is called in user-space
	_state = S_TX;										// _state set to transmit

	return 1;
} //sendPacket DOWN




// --------------------------------- UP ---------------------------------------
//
// Based on the information read from the LoRa transceiver (or fake message)
// build a gateway message to send upstream (to the user somewhere on the web).
//
// parameters:
// 	tmst: Timestamp to include in the upstream message
// 	buff_up: The buffer that is generated for upstream
//	LoraUP: Structure describing the message received from device
// 	internal: Boolean value to indicate whether the local sensor is processed
//
// returns:
//	buff_index:
// ----------------------------------------------------------------------------
int buildPacket(uint8_t *buff_up, struct LoraUp *LoraUp, bool internal) 
{
	int32_t SNR;
    int16_t rssicorr;
	int16_t prssi;										// packet rssi
	
	char cfreq[12] = {0};								// Character array to hold freq in MHz
	uint16_t buff_index=0;
	char b64[256];
	
	uint8_t *message = LoraUp->payLoad;
	char messageLength = LoraUp->payLength;
		
#if _CHECK_MIC==1
	unsigned char NwkSKey[16] = _NWKSKEY;
	checkMic(message, messageLength, NwkSKey);
#endif //_CHECK_MIC

	// Read SNR and RSSI from the register. Note: Not for internal sensors!
	// For internal sensor we fake these values as we cannot read a register
	if (internal) {
		SNR = 12;
		prssi = 50;
		rssicorr = 157;
	}
	else {
		SNR = LoraUp->snr;
		prssi = LoraUp->prssi;								// read register 0x1A, packet rssi
		rssicorr = LoraUp->rssicorr;
	}

#if _STATISTICS >= 1
	// Receive statistics, move old statistics down 1 position
	// and fill the new top line with the latest received sensor values.
	// This works fine for the sensor, EXCEPT when we decode data for _LOCALSERVER
	//
	for (int m=( gwayConfig.maxStat -1); m>0; m--) statr[m]=statr[m-1];
	
	// From now on we can fill statr[0] with sensor data
#if _LOCALSERVER==1
	statr[0].datal=0;
	int index;
	if ((index = inDecodes((char *)(LoraUp->payLoad+1))) >=0 ) {

		uint16_t frameCount=LoraUp->payLoad[7]*256 + LoraUp->payLoad[6];
		
		for (int k=0; (k<LoraUp->payLength) && (k<23); k++) {
			statr[0].data[k] = LoraUp->payLoad[k+9];
		};
		
		// XXX Check that k<23 when leaving the for loop
		// XXX or we can not display in statr
		
		uint8_t DevAddr[4]; 
		DevAddr[0]= LoraUp->payLoad[4];
		DevAddr[1]= LoraUp->payLoad[3];
		DevAddr[2]= LoraUp->payLoad[2];
		DevAddr[3]= LoraUp->payLoad[1];

		statr[0].datal = encodePacket((uint8_t *)(statr[0].data), 
								LoraUp->payLength -9 -4, 
								(uint16_t)frameCount, 
								DevAddr, 
								decodes[index].appKey, 
								0);
	}
#endif //_LOCALSERVER

	statr[0].time = now();								// Not a real timestamp. but the current time
	statr[0].ch =	gwayConfig.ch;
	statr[0].prssi = prssi - rssicorr;
	statr[0].sf =	LoraUp->sf;
	
#	if RSSI==1
		statr[0].rssi = _rssi - rssicorr;
#	endif // RSII

	statr[0].node = ( message[1]<<24 | message[2]<<16 | message[3]<<8 | message[4] );

#if _STATISTICS >= 2
	// Fill in the statistics that we will also need for the GUI.
	// So 
	switch (statr[0].sf) {
		case SF7:  statc.sf7++;  break;
		case SF8:  statc.sf8++;  break;
		case SF9:  statc.sf9++;  break;
		case SF10: statc.sf10++; break;
		case SF11: statc.sf11++; break;
		case SF12: statc.sf12++; break;
	}
#endif //_STATISTICS >= 2



#if _STATISTICS >= 3
	if (statr[0].ch == 0) {
		statc.msg_ttl_0++;								// Increase #message received channel 0
		switch (statr[0].sf) {
			case SF7:  statc.sf7_0++;  break;
			case SF8:  statc.sf8_0++;  break;
			case SF9:  statc.sf9_0++;  break;
			case SF10: statc.sf10_0++; break;
			case SF11: statc.sf11_0++; break;
			case SF12: statc.sf12_0++; break;
		}
	}
	else 
	if (statr[0].ch == 1) {
		statc.msg_ttl_1++;
		switch (statr[0].sf) {
			case SF7:  statc.sf7_1++;  break;
			case SF8:  statc.sf8_1++;  break;
			case SF9:  statc.sf9_1++;  break;
			case SF10: statc.sf10_1++; break;
			case SF11: statc.sf11_1++; break;
			case SF12: statc.sf12_1++; break;
		}
	}
	else 
	if (statr[0].ch == 2) {
		statc.msg_ttl_2++;
		switch (statr[0].sf) {
			case SF7:  statc.sf7_2++;  break;
			case SF8:  statc.sf8_2++;  break;
			case SF9:  statc.sf9_2++;  break;
			case SF10: statc.sf10_2++; break;
			case SF11: statc.sf11_2++; break;
			case SF12: statc.sf12_2++; break;
		}
	}
#endif //_STATISTICS >= 3

#endif //_STATISTICS >= 2

#if _MONITOR>=1	
	if ((debug>=2) && (pdebug & P_RADIO)) {
		Serial.print(F("R buildPacket:: pRSSI="));
		Serial.print(prssi-rssicorr);
		Serial.print(F(" RSSI: "));
		Serial.print(_rssi - rssicorr);
		Serial.print(F(" SNR: "));
		Serial.print(SNR);
		Serial.print(F(" Length: "));
		Serial.print((int)messageLength);
		Serial.print(F(" -> "));
		int i;
		for (i=0; i< messageLength; i++) {
					Serial.print(message[i],HEX);
					Serial.print(' ');
		}
		Serial.println();
		yield();										// only if debug>=2
	}
#endif //_MONITOR

// Show received message status on Oled display
#if _OLED>=1
    char timBuff[20];
    sprintf(timBuff, "%02i:%02i:%02i", hour(), minute(), second());
	
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
	
    display.drawString(0, 0, "Time: " );
    display.drawString(40, 0, timBuff);
	
    display.drawString(0, 16, "RSSI: " );
    display.drawString(40, 16, String(prssi-rssicorr));
	
    display.drawString(70, 16, ",SNR: " );
    display.drawString(110, 16, String(SNR) );
	  
	display.drawString(0, 32, "Addr: " );
	  
    if (message[4] < 0x10) display.drawString( 40, 32, "0"+String(message[4], HEX)); else display.drawString( 40, 32, String(message[4], HEX));
	if (message[3] < 0x10) display.drawString( 61, 32, "0"+String(message[3], HEX)); else display.drawString( 61, 32, String(message[3], HEX));
	if (message[2] < 0x10) display.drawString( 82, 32, "0"+String(message[2], HEX)); else display.drawString( 82, 32, String(message[2], HEX));
	if (message[1] < 0x10) display.drawString(103, 32, "0"+String(message[1], HEX)); else display.drawString(103, 32, String(message[1], HEX));
	  
    display.drawString(0, 48, "LEN: " );
    display.drawString(40, 48, String((int)messageLength) );
    display.display();

#endif //_OLED>=1
			
//	int j;
	
	// XXX Base64 library is nopad. So we may have to add padding characters until
	// 	message Length is multiple of 4!
	// Encode message with messageLength into b64
	int encodedLen = base64_enc_len(messageLength);		// max 341

#	if _MONITOR>=1
	if ((debug>=1) && (encodedLen>255) && (pdebug & P_RADIO)) {
		mPrint("R buildPacket:: b64 err, len=" + String(encodedLen));
		return(-1);
	}
#	endif //_MONITOR

	base64_encode(b64, (char *) message, messageLength);// max 341
	// start composing datagram with the header 
	uint8_t token_h = (uint8_t)rand(); 					// random token
	uint8_t token_l = (uint8_t)rand(); 					// random token
	
	// pre-fill the data buffer with fixed fields
	buff_up[0] = PROTOCOL_VERSION;						// 0x01 still

	buff_up[1] = token_h;
	buff_up[2] = token_l;
	
	buff_up[3] = PKT_PUSH_DATA;							// 0x00
	
	// READ MAC ADDRESS OF ESP8266, and insert 0xFF 0xFF in the middle
	buff_up[4]  = MAC_array[0];
	buff_up[5]  = MAC_array[1];
	buff_up[6]  = MAC_array[2];
	buff_up[7]  = 0xFF;
	buff_up[8]  = 0xFF;
	buff_up[9]  = MAC_array[3];
	buff_up[10] = MAC_array[4];
	buff_up[11] = MAC_array[5];

	buff_index	= 12; 									// 12-byte binary (!) header
	
	// start of JSON structure that will make payload
	memcpy((void *)(buff_up + buff_index), (void *)"{\"rxpk\":[{", 10); buff_index += 10;
	
// More versions are defined for the moment, in order to keep timing as low as [possible. 
// The serializeJson() version hopefully is quicker

#ifdef _JSONENCODE
//------------------
	StaticJsonDocument<400> doc;
	
	//doc["time"] = ""+now();

	doc["chan"] = "0";
	doc["rfch"] = "0";
	doc["freq"] = "" + (freqs[gwayConfig.ch].upFreq / 1000000);
	doc["stat"] = "1";
	doc["modu"] = "LORA";
	doc["datr"] = "SF" + String(LoraUp->sf) + "BW" + String(freqs[gwayConfig.ch].upBW);
	doc["rssi"] = "" +(prssi-rssicorr);
	doc["lsnr"] = "" +(long)SNR;
	doc["codr"] = "4/5";

	// Use gBase64 library to fill in the data string
	encodedLen = base64_enc_len(messageLength);			// max 341	
	doc["size"] = "" + encodedLen;

	int len= base64_encode(doc["data"], (char *)message, messageLength);

	LoraUp->tmst = doc["tmst"] = "" + (uint32_t) micros() + _RXDELAY1;		// Tmst correction							
	
	const char	* p =  (const char *) & (buff_up [buff_index]);				// Start in buff where to put the serializedJson
	int written = serializeJson(doc, (const char *)p, buff_index+20 );		// size is buff_index + encoded data + some closing chars


#else //_JSONENCODE undefined, this is default
// -----------------
	ftoa((double)freqs[gwayConfig.ch].upFreq / 1000000, cfreq, 6);		// XXX This can be done better
	if ((LoraUp->sf<6) || (LoraUp->sf>12)) { 				// Lora datarate & bandwidth SF6-SF12, 16-19 useful chars */
		LoraUp->sf=7;
	}			

//	buff_index += snprintf((char *)(buff_up + buff_index), 
//		TX_BUFF_SIZE-buff_index, 
//		"%04d-%02d-%02d %02d:%02d:%02d CET", 
//		year(),month(),day(),hour(),minute(),second());

	buff_index += snprintf((char *)(buff_up + buff_index), 
		TX_BUFF_SIZE-buff_index, 
		"\"chan\":%1u,\"rfch\":%1u,\"freq\":%s,\"stat\":1,\"modu\":\"LORA\"" , 
		0, 0, cfreq);
	
	buff_index += snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index
		, ",\"datr\":\"SF%uBW%u\",\"codr\":\"4/5\",\"lsnr\":%li,\"rssi\":%d,\"size\":%u,\"data\":\""
		, LoraUp->sf, freqs[gwayConfig.ch].upBW, (long)SNR, prssi-rssicorr, messageLength);

	// Use gBase64 library to fill in the data string
	encodedLen = base64_enc_len(messageLength);				// max 341
	buff_index += base64_encode((char *)(buff_up + buff_index), (char *) message, messageLength);

	
	LoraUp->tmst = (uint32_t) micros()+ _RXDELAY1; 			// Correct timing with defined number,
															// https://github.com/TheThingsNetwork/lorawan-stack/issues/277

	// Get rid of this code when ready	

	buff_index += snprintf((char *)(buff_up + buff_index), 
		TX_BUFF_SIZE-buff_index, "\",\"tmst\":%u", 
		LoraUp->tmst);	

#endif //_JSONENCODE undefined or ==0
// ---------------------


	// End of packet serialization
	buff_up[buff_index]   = '}'; 
	buff_up[buff_index+1] = ']'; 
	buff_up[buff_index+2] = '}'; 
	buff_index += 3;
	
	buff_up[buff_index] = 0; 							// add string terminator, for safety

	// When we have the node address and the SF, fill the listSeen array
	// with the required data. _MAXSEEN must be >0 for this to happen.
	// statr[0] contains the statistics of the node last seen.
#	if  _MAXSEEN>=1
		addSeen(listSeen, statr[0]);
#	endif //_MAXSEEN

#	if _STAT_LOG==1	
		// Do statistics logging. In first version we might only
		// write part of the record to files, later more
		addLog( (unsigned char *)(buff_up), buff_index );
#	endif //_STAT_LOG

#	if _MONITOR>=1
	if ((debug>=1) && (pdebug & P_RX)) {			// debug: display JSON payload
		mPrint("UP RXPK:: "+String((char *)(buff_up + 12))+" , length="+String(buff_index));		
	}
#	endif

	return(buff_index);
	
}// buildPacket()



// --------------------------------- UP ---------------------------------------
//
// Receive a LoRa package over the air, LoRa and deliver to server(s)
//
// Receive a LoRa message and fill the buff_up char buffer.
// returns values:
// - returns the length of string returned in buff_up
// - returns -1 or -2 when no message arrived, depending connection.
//
// This is the "highlevel" read function called by loop(). The receive function 
// is started in the _stateMachine.ini file after CDONE event by interrupt 
// functions.
// However, the actual read from the buffer (filled by interrupt) is done 
// by this function in the main loop() program.
// ----------------------------------------------------------------------------
int receivePacket()
{
	uint8_t buff_up[TX_BUFF_SIZE]; 						// buffer to compose the upstream packet to backend server

	// Regular message received, see SX1276 spec table 18
	// Next statement could also be a "while" to combine several messages received
	// in one UDP message as the Semtech Gateway spec does allow this.
	// XXX Bit ... Not yet supported


		// Handle the physical data read from LoraUp
		if (LoraUp.payLength > 0) {

#			ifdef _PROFILER
				int32_t startTime = micros();
#			endif //_PROFILER

			// externally received packet, so last parameter is false (==LoRa external)
			// Make a buffer to transmit later
            int build_index = buildPacket(buff_up, &LoraUp, false);

			// REPEATER is a special function where we retransmit package received 
			// message on _ICHANN to _OCHANN.
			// Note:: For the moment _OCHANN is not allowed to be same as _ICHANN
#			if _REPEATER==1
			if (!sendLora(LoraUp.payLoad, LoraUp.payLength)) {
				return(-3);
			}
#			endif

#			ifdef _TTNSERVER	
			// This is one of the potential problem areas.
			// If possible, USB traffic should be left out of interrupt routines
			// rxpk PUSH_DATA received from node is rxpk (*2, par. 3.2)

			if (!sendUdp(ttnServer, _TTNPORT, buff_up, build_index)) {
				return(-1); 							// received a message
			}
#			endif //_TTNSERVER

			yield();
			Udp.flush();								// MMM 200419

#			ifdef _PROFILER
			if ((debug>=1) && (pdebug & P_RX)) {
				int32_t endTime = micros();
				String response = "UP receivePacket:: end="; printInt(endTime,response);
				response += ", start="; printInt(startTime, response);
				response += ", diff=" +String(endTime-startTime) + " uSec";
				mPrint(response);
			}
#			endif //_PROFILER

#			ifdef _THINGSERVER
			// Use our own defined server or a second well known server
			if (!sendUdp(thingServer, _THINGPORT, buff_up, build_index)) {
				return(-2); 							// received a message
			}
#			endif //_THINGSERVER

#if _LOCALSERVER==1
			// Or special case, we do not use a local server to receive
			// and decode the server. We use buildPacket() to call decode
			// and use statr[0] information to store decoded message

			//DecodePayload: para 4.3.1 of Lora 1.1 Spec
			// MHDR
			//	1 byte			Payload[0]
			// FHDR
			// 	4 byte Dev Addr Payload[1-4]
			// 	1 byte FCtrl  	Payload[5]
			// 	2 bytes FCnt	Payload[6-7]				
			// 		= Optional 0 to 15 bytes Options
			// FPort
			//	1 bytes, 0x00	Payload[8]
			// ------------
			// +=9 BYTES HEADER
			//
			// FRMPayload
			//	N bytes			(Payload )
			//
			// 4 bytes MIC trailer

			int index=0;
			if ((index = inDecodes((char *)(LoraUp.payLoad+1))) >=0 ) {

				uint8_t DevAddr[4]; 
				DevAddr[0]= LoraUp.payLoad[4];
				DevAddr[1]= LoraUp.payLoad[3];
				DevAddr[2]= LoraUp.payLoad[2];
				DevAddr[3]= LoraUp.payLoad[1];
				//uint16_t frameCount=LoraUp.payLoad[7]*256 + LoraUp.payLoad[6];

#if				_DUSB>=1
				if ((debug>=2) && (pdebug & P_RX)) {
					Serial.print(F("UP receivePacket:: Ind="));
					Serial.print(index);
					Serial.print(F(", Len="));
					Serial.print(LoraUp.payLength);
					Serial.print(F(", A="));
					for (int i=0; i<4; i++) {
						if (DevAddr[i]<0x0F) Serial.print('0');
						Serial.print(DevAddr[i],HEX);
						//Serial.print(' ');
					}
				
					Serial.print(F(", Msg="));
					for (int i=0; (i<statr[0].datal) && (i<23); i++) {
						if (statr[0].data[i]<0x0F) Serial.print('0');
						Serial.print(statr[0].data[i],HEX);
						Serial.print(' ');
					}
				}
#				endif //_DUSB

			}
#			if _MONITOR>=1
			else if ((debug>=2) && (pdebug & P_RX)) {
					mPrint("receivePacket:: No Index");
			}
#			endif //_MONITOR
#endif //_LOCALSERVER

			// Reset the message area
			LoraUp.payLength = 0;
			LoraUp.payLoad[0] = 0x00;

			return(build_index);
        }

	return(0);											// failure no message read

}//receivePacket

