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
// _udpSemtech.ino: This file contains the UDP specific code enabling to receive
// and transmit packages/messages to the server usig Semtech protocol.
// ========================================================================================

// Also referred to as Semtech code

#if defined(_UDPROUTER)

//	If _UDPROUTER is defined, _TTNROUTER should NOT be defined. So...
#if defined(_TTNROUTER)
#error "Please make sure that either _UDPROUTER or _TTNROUTER are defined but not both"
#endif

// The following functions ae defined in this module:
// int readUdp(int Packetsize)
// int sendUdp(IPAddress server, int port, uint8_t *msg, uint16_t length)
// bool connectUdp();
// void pullData();
// void sendStat();

// ----------------------------------------------------------------------------
// connectUdp()
//	connect to UDP (which is a local thing, after all UDP 
// connections do not exist.
// Parameters:
//	<None>
// Returns
//	Boollean indicating success or not
// ----------------------------------------------------------------------------
bool connectUdp() 
{

	bool ret = false;
	unsigned int localPort = _LOCUDPPORT;			// To listen to return messages from WiFi
#	if _MONITOR>=1
	if (debug>=1) {
		mPrint("Local UDP port=" + String(localPort));
	}
#	endif //_MONITOR

	if (Udp.begin(localPort) == 1) {
#		if _MONITOR>=1
		if (debug>=1) {
			mPrint("UDP Connection successful");
		}
#		endif //_MONITOR
		ret = true;
	}
	else{
#		if _MONITOR>=1
		if (debug>=0) {
			mPrint("Connection failed");
		}
#		endif //_MONITOR
	}
	return(ret);
}// connectUdp


// ----------------------------------- DOWN -----------------------------------
//
// readUdp()
// Read DOWN a package from UDP socket from server (it can come from any server).
// Messages are received when server responds to gateway requests from LoRa nodes 
// (e.g. JOIN requests etc.) or when server has downstream data.
// We respond only to the server that sents us a message!
//
// Note: So normally we can forget here about codes that do upstream
//
// Parameters:
//	Packetsize: size of the buffer to read, as read by loop() calling function
//
//	Byte 0: 	Contains Protocol Version
//	Byte 1+2:	Contain Token
//	Byte 3:		Contains PULL_RESP or other identifier
//	Byte 4-n: 	Contains payload (or Gateway EUI 8 bytes first)
//
// Returns:
//	-1 or false if not read
//	Or number of characters read if success
//
// ----------------------------------------------------------------------------
int readUdp(int packetSize)
{ 

	// Make sure we are connected over WiFI
	if (WlanConnect(10) < 0) {
#		if _MONITOR>=1
			mPrint("v readUdp:: ERROR connecting to WLAN");
#		endif //_MONITOR
		Udp.flush();
		return(-1);
	}
	
	if (packetSize > TX_BUFF_SIZE) {
#		if _MONITOR>=1
			mPrint("v readUdp:: ERROR package of size: " + String(packetSize));
#		endif //_MONITOR
		Udp.flush();
		return(-1);
	}
	
	// We assume here that we know the originator of the message.
	// In practice however this can be any sender!
	if (Udp.read(buff_down, packetSize) < packetSize) {
#		if _MONITOR>=1
			mPrint("v readUdp:: Reading less chars");
#		endif //_MONITOR
		return(-1);
	}

	yield();								// MMM 200406

	// Remote Address should be known
	remoteIpNo = Udp.remoteIP();

	// Remote port is either of the remote TTN server or from NTP server (=123)
	remotePortNo = Udp.remotePort();
	
	if (remotePortNo == 123) {				// NTP message arriving, not expected
		// This is an NTP message arriving
#		if _MONITOR>=1
		if (debug>=0) {
			mPrint("v readUdp:: NTP msg rcvd");
		}
#		endif //_MONITOR
		gwayConfig.ntpErr++;
		gwayConfig.ntpErrTime = now();
		Udp.flush();						// 200326 Clear buffer when time response arrives (error)
		return(0);
	}
	
	// If it is not NTP it must be a LoRa message for gateway or node
	
	else {
		// First 4 butes are very important, rest is data
		// Especially the 2 token bytes should be watched.
		protocol= buff_down[0];
		uint16_t token= buff_down[2]*256 + buff_down[1];			// LSB first [1], MSB [2] comes after
		uint8_t ident= buff_down[3];
		// uint8_t *data = (uint8_t *) ((uint8_t *)buff_down + 4);
		
#		if _MONITOR>=1
		if ((debug>=3) && (pdebug & P_TX)) {
			mPrint("v readUdp:: message ident="+String(ident));
		}
#		endif //_MONITOR

		// now parse the message type from the server (if any)
		switch (ident) {


		// This message is used by the gateway to send sensor data UP to server. 
		// As this function is used for downstream only, this option will never 
		// be executed by this function but is included as a reference only
		// Para 5.2.1, Semtech Gateway to Server Interface document
		//	Byte 0:		Protocol version (0x01 or 0x02)
		//	byte 1+2:	Token, random
		//	byte 3: 	Command code (=0x00)
		//	Byte 4-11:	Gateway EUI
		//	Byte 12-n:	JSON data
		//
		case PUSH_DATA: 								// 0x00 UP, never activated
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_RX)) {
				mPrint("v PUSH_DATA:: size "+String(packetSize)+" From "+String(remoteIpNo.toString()));
			}
#			endif //_MONITOR
			Udp.flush();
		break;


		// This message is sent DOWN by the server to acknowledge receipt of a
		// (sensor) PUSH_DATA message sent with the code above.
		// Para 5.2.2, Semtech Gateway to Server Interface document
		// The length of this package is 4 bytes:
		//	byte 0:		Protol version (0x01 or 0x02)
		//	byte 1+2:	Token copied from requestor
		//	byte 3:		ident = 0x01, ack PUSH_ACK
		//
		case PUSH_ACK:									// 0x01 DOWN
#			if _MONITOR>=1
			if ((debug>=2) && (pdebug & P_TX)) {
				char res[128];				
				sprintf(res, "v PUSH_ACK:: token=%u, size=%u, IP=%d.%d.%d.%d, port=%d, protocol=%u ", 
					(buff_down[2]*256+buff_down[1]),
					packetSize, 
					remoteIpNo[0], remoteIpNo[1], remoteIpNo[2],remoteIpNo[3], 
					remotePortNo,
					protocol);
				mPrint(res);
			}
#			endif //_MONITOR
			//Udp.flush();
		break;


		// PULL DATA message (Up)						// UP, never activated
		// This is a request/UP message and is never executed by this function.
		// We have it here as a description only. 
		//	Para 5.2.3, Semtech Gateway to Server Interface document
		//
		// Byte 0		contains Protocol Version (0x01 or 0x02)
		// Byte 1-2		Random Token
		// Byte 3		PULL_DATA ident == 0x02
		// Byte 4-11	Gateway EUI
		//
		case PULL_DATA:									// 0x02 UP
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_RX)) {
				mPrint("v PULL_DATA");
			}
#			endif //_MONITOR
			Udp.flush();								// 200419 Added, probably never executed
		break;


		// PULL_ACK message (Down)						// Called to confirm receipt of pull-data
		// This is the (immediate!) response to PULL_DATA message
		// Para 5.2.4, Semtech Gateway to Server Interface document
		// With this ACK, the server confirms the gateway that the route is open
		// for further PULL_RESP messages from the server (to the device)
		// The server sends a PULL_ACK to confirm PULL_DATA receipt, no response is needed
		//
		// Byte 0		contains Protocol Version == 2
		// Byte 1-2		Token as issued from the gatway when requesting
		// Byte 3		PULL_ACK ident == 0x04
		// Byte 4-11:	Gateway EUI
		//
		case PULL_ACK:									// 0x04 DOWN
#			if _MONITOR>=1
			if ((debug>=2) && (pdebug & P_TX)) {
				char res[128];				
				sprintf(res, "v PULL_ACK:: token=%u, size=%u, IP=%d.%d.%d.%d, port=%d, protocol=%u ", 
					(buff_down[2]*256+buff_down[1]),
					packetSize, 
					remoteIpNo[0], remoteIpNo[1], remoteIpNo[2],remoteIpNo[3], 
					remotePortNo,
					protocol);
				mPrint(res);
			}
#			endif //_MONITOR
			
			yield();				
			Udp.flush();								// 200419
			// No response is needed
		break;


		// PULL_RESP (Down)
		// This message type is used to confirm OTAA message to the node,
		// but this message format will also be used for other downstream communication
		// It's length shall not exceed 1000 Octets. (TTN 51 octets) 
		// This is:
		//	RECEIVE_DELAY1		1 s 
		//	RECEIVE_DELAY2		2 s (is RECEIVE_DELAY1+1)
		//	JOIN_ACCEPT_DELAY1	5 s
		//	JOIN_ACCEPT_DELAY2	6 s
		//
		// buff_down[0]:		Version number (==PROTOCOL_VERSION)
		// buff_down[1-2]:		Token: If Protocol version==0, make 0. If version==1 arbitrary?
		// buff_down[3]:		PULL_RESP: ident = 0x03
		// buff_down[4-n]:		payLoad data
		//
		// Para 5.2.5, Semtech Gateway to Server Interface document
		// or https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT
		//
		case PULL_RESP:									// 0x03 DOWN

			if (protocol==0x01) {						// If protocol version is 0x01
				token = 0;								// Use token 0 in that case
				buff_down[2]=0;
				buff_down[1]=0;
			}
			
			// Define when we start with the response to node
#			ifdef _PROFILER
			if ((debug>=1) && (pdebug & P_TX)) {
				mPrint("v PULL_RESP:: start sendPacket: micros="+String(micros() ));
				char res[128];				
				sprintf(res, "v PULL_RESP:: token=%u, size=%u, IP=%d.%d.%d.%d, port=%d, protocol=%u, micros=%lu", 
					token,
					packetSize, 
					remoteIpNo[0], remoteIpNo[1], remoteIpNo[2],remoteIpNo[3], 
					remotePortNo,
					protocol,
					(unsigned long) micros() 
				);
				mPrint(res);
			}
#			endif//_PROFILER

			// Send to the LoRa Node first (timing) and then do reporting to _Monitor
			//_state=S_TX;
			sendTime = micros();						// record when we started sending the message

			// Prepare to send the buffre package DOWN to the sensor
			// We just read the packet from the Network Server and it is formatted
			// as described in the specs. This function fills LoraDown struct.
			if (sendPacket(buff_down, packetSize) < 0) {
#				if _MONITOR>=1
				if (debug>=0) {
					mPrint("v readUdp:: ERROR: PULL_RESP sendPacket failed");
				}
#				endif //_MONITOR
				Udp.flush();
				return(-1);
			}


			// We need a timeout for this case. During transmission we should not accept
			// another package receiving/sending

			if (loraWait(&LoraDown) == 0) {
				_state=S_CAD;										// Maybe call TXDONE and wait 1 sec
				_event=1;
				break;
			}
			
			// Initiate the transmission of the buffer
			// (We normally react on ALL interrupts if we are in TX state)
			txLoraModem(&LoraDown);									// Calling sendPkt() in turn

			// Copy the lastSeen data down, making room on first entry
			for (int m=( gwayConfig.maxStat -1); m>0; m--) statr[m]= statr[m-1];
			
			// If transmission is finished, print statistics
#			if _MONITOR>=1

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
				
#			  if _LOCALSERVER>=2				
				uint8_t DevAddr[4];
				uint16_t frameCount;
				int index;

				if ((index = inDecodes((char *)(LoraDown.payLoad+1))) >= 0 ) {
#	ifdef _FCNT
					frameCount= LoraDown.fcnt;
#	else
					frameCount= LoraDown.payLoad[7]*256 + LoraDown.payLoad[6];
#	endif

					// Only if _LOCALSERVER >= 2 for downstream
					strncpy ((char *)statr[0].data, (char *)LoraDown.payLoad+9,  LoraDown.size-9-4);

					DevAddr[0]= LoraDown.payLoad[4];
					DevAddr[1]= LoraDown.payLoad[3];
					DevAddr[2]= LoraDown.payLoad[2];
					DevAddr[3]= LoraDown.payLoad[1];

					statr[0].datal = encodePacket(
						(uint8_t *)(statr[0].data), 
						LoraDown.size -9 -4, 
						(uint16_t)frameCount, 
						DevAddr, 
						decodes[index].appKey, 
						1
					);													// Down
				}
				else {
#					if _MONITOR >= 1				
						//
#					endif //_MONITOR
				}
#			  elif _LOCALSERVER==1
				// If we should not print data for downlink
				statr[0].datal = 0;
#			  endif //_LOCALSERVER

			// If _MONITOR set print the statistics
			if ((debug>=1) && (pdebug & P_TX)) {

				String response ="v txLoraModem hi:: ";
				printDwn(&LoraDown, response);
				mPrint(response);

				response = "v txLoraModem reg:: ";
				printRegs(&LoraDown, response);
				mPrint(response);
			
				delay(1);
				response = "v txLoraModem lo:: ";

#				if _LOCALSERVER>=2

					if (statr[0].datal>24) {
						response+= ", statr[0].datal=" + String(statr[0].datal);
						statr[0].datal=24;
					}
					
					response+= "new=<";
					
					if (statr[0].datal < 0) {
						mPrint("ERROR datal=0");
						statr[0].datal=0;
					}
					for (int i=0; i<statr[0].datal; i++) {
						response += String(statr[0].data[i],HEX) + " ";
					}
					response += ">";
					
					response += ", addr=";
					printHex((IPAddress)DevAddr, ':', response);
					
					response+= ", fcnt=" + String(frameCount);
#				endif //_LOCALSERVER

				response += ", size=" + String(LoraDown.size);
				response += ", old=<";
				for(int i=0; i<LoraDown.size; i++) {
					response += String(LoraDown.payLoad[i],HEX) + " ";
				}
				response += ">";				
				mPrint(response);
			}

#			endif //_MONITOR

			statr[0].time	= now();
			statr[0].ch		= gwayConfig.ch;
			statr[0].sf		= LoraDown.sf;
			statr[0].upDown	= 1;							// Down
			statr[0].node	= ( 
					LoraDown.payLoad[1]<<24 | 
					LoraDown.payLoad[2]<<16 | 
					LoraDown.payLoad[3]<<8 | 
					LoraDown.payLoad[4] 
			);

			addSeen(listSeen, statr[0]);
			
#			if RSSI==1
				statr[0].rssi	= _rssi - rssicorr;
#			endif // RSSI

			//statr[0].datal	= 3;
			//statr[0].data[0]	= 0x86;
			//statr[0].data[1]	= 0xC4;
			//statr[0].data[2]	= 0x0A;
			//statr[0].data[3]	= 0x00;
			//strncpy((char *)statr[0].data, "Down", 4);	// MMMM

			// After filling the buffer we only react on TXDONE interrupt
			// So, more or less start at the "case TXDONE:"  
			_state=S_TXDONE;
			_event=1;										// Or remove the break below

			yield();										// MMM 200925

		break;


		// TX_ACK (Up)										// Never activated by this function
		// This is the response to the PULL_RESP message by the sensor device
		// it is sent by the gateway UP to the server to confirm the PULL_RESP message.
		//	byte 0:		Protocol version (0x01 or 0x02)
		//	byte 1+2:	Token number of UP sender
		//	byte 3:		Message ID TX_ACK == 0x05
		//	byte 4-n:	Optional Error Data, {"errno":xxxx}
		//
		case TX_ACK:										// Message id: 0x05 UP

			if (protocol == 1) {							// Got from the downstream message
#				if _MONITOR>=1
				if ((debug>=1) && (pdebug & P_TX)) {
					mPrint("^ TX_ACK:: readUdp: protocol version 1");
//					uint8_t *data;
//					data = buff_down + 4;
//					data[packetSize-4] = 0;
				}
#				endif
				break;										// return
			}

#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_TX)) {
				mPrint("^ TX_ACK:: readUDP: protocol version 2+");
			}
#			endif //_MONITOR
		break;


		default:
#			if _GATEWAYMGT==1
				// For simplicity, we send the first 4 bytes too
				gateway_mgt(packetSize, buff_down);
#			endif
#			if _MONITOR>=1
				mPrint(", ERROR ident not recognized="+String(ident));
#			endif //_MONITOR
		break;
		}
		
#		if _MONITOR>=1
		if ((debug>=3) && (pdebug & P_TX)) {
			String response= "v readUdp:: ident="+String(ident,HEX);
			response+= ", tmst=" + String(LoraDown.tmst);
			response+= ", imme=" + String(LoraDown.imme);
			response+= ", sf=" + String(LoraDown.sf);
			response+= ", freq=" + String(LoraDown.freq);
			if (debug>=3) {
				if (packetSize > 4) {
					response+= ", size=" + String(packetSize) + ", data=";
					buff_down[packetSize] = 0;
					response+=String((char *)(buff_down+4));
				}
			}
			mPrint(response); 
		}
#		endif //_MONITOR

		// For downstream messages
		return packetSize;
	}
} //readUdp()


// ----------------------------------- UP -------------------------------------
// sendUdp()
//
// Send UP an UDP/DGRAM message to the MQTT server
// If we send to more than one host (not sure why) then we need to set sockaddr 
// before sending.
// Parameters:
//	IPAddress
//	port
//	msg *
//	length (of msg)
// return values:
//	0: Error
//	1: Success
// ----------------------------------------------------------------------------
int sendUdp(IPAddress server, int port, uint8_t *msg, uint16_t length) 
{
	// Check whether we are conected to Wifi and the internet
	if (WlanConnect(3) < 0) {
#		if _MONITOR>=1
		if (pdebug & P_MAIN) {
			mPrint("sendUdp: ERROR not connected to WiFi");
		}
#		endif //_MONITOR
		Udp.flush();
		return(0);
	}


	//send the update
#	if _MONITOR>=1
	if ((debug>=2) && (pdebug & P_MAIN)) {
		mPrint("sendUdp: WlanConnect connected to="+WiFi.SSID()+". Server IP="+ String(WiFi.localIP().toString()) );
	}
#	endif //_MONITOR

	if (!Udp.beginPacket(server, (int) port)) {
#		if _MONITOR>=1
		if ( debug>=0 ) {
			mPrint("M sendUdp:: ERROR Udp.beginPacket");
		}
#		endif //_MONITOR
		return(0);
	}
	
	yield();									// MMM 200316 May not be necessary

	if (Udp.write((unsigned char *)msg, length) != length) {
#		if _MONITOR>=1
		if ( debug>=0 ) {
			mPrint("sendUdp:: ERROR Udp write");
		}
#		endif //_MONITOR
		Udp.endPacket();						// Close UDP
		return(0);								// Return error
	}
	
	yield();
	
	if (!Udp.endPacket()) {
#	if _MONITOR>=1
		if (debug>=0) {
			mPrint("sendUdp:: ERROR Udp.endPacket");
		}
#	endif //_MONITOR
		return(0);
	}
	return(1);
}//sendUDP




// --------------------------------- UP ---------------------------------------
// pullData()
// Sending a PULL_DATA request to the server
// Send UDP periodic Pull_DATA message UP to server to keepalive the connection
// and to invite the server to send downstream messages when these are available
// *2, par. 5.2.3
//
//	Byte 0:		Protocol Version (1 byte)
//	Byte 1-2:	Random Token (2 bytes)
//	Byte 3:		PULL_DATA identifier (1 byte) = 0x02
//	Byte 4-11:	8 bytes Gateway unique identifier (8 bytes) = MAC address
// ----------------------------------------------------------------------------
void pullData()
{
    uint8_t pullDataReq[13]; 								// status report as a JSON object
    int pullIndex	=0;
	
	uint8_t token_h = (uint8_t)rand(); 						// random token
    uint8_t token_l = (uint8_t)rand();						// random token
	
    // pre-fill the data buffer with fixed fields
    pullDataReq[0]  = PROTOCOL_VERSION;						// 0x01
    pullDataReq[1]  = token_l;								// random
    pullDataReq[2]  = token_h;								// random
    pullDataReq[3]  = PULL_DATA;							// 0x02
	
	// READ MAC ADDRESS OF ESP8266, and return unique Gateway ID consisting of MAC address and 2bytes 0xFF
    pullDataReq[4]  = MAC_array[0];
    pullDataReq[5]  = MAC_array[1];
    pullDataReq[6]  = MAC_array[2];
    pullDataReq[7]  = 0xFF;
    pullDataReq[8]  = 0xFF;
    pullDataReq[9]  = MAC_array[3];
    pullDataReq[10] = MAC_array[4];
    pullDataReq[11] = MAC_array[5];
	
    pullDataReq[12] = 0; 									// add string terminator, for safety
	
    pullIndex = 12;											// 12-byte header

	uint8_t *pullPtr = pullDataReq;
	
    //send the update
#	ifdef _TTNSERVER
		sendUdp(ttnServer, _TTNPORT, pullDataReq, pullIndex);
		yield();
#	endif //_TTNSERVER

#	ifdef _THINGSERVER
		sendUdp(thingServer, _THINGPORT, pullDataReq, pullIndex);
#	endif


#	if _MONITOR>=1
	if (pullPtr != pullDataReq) {
		mPrint("pullPtr != pullDatReq");
	}

    if ((debug>=1) && (pdebug & P_RX)) {
		yield();
		mPrint("^ PULL_DATA:: token=" +String(token_h*256+token_l) +", len=" + String(pullIndex) );
		Serial.print("v Gateway EUI=");
		for (int i=0; i<pullIndex; i++) {
			Serial.print(pullDataReq[i],HEX);				// debug: display JSON stat
			Serial.print(':');
		}
		Serial.println();
	}
#endif //_MONITOR

	return;
	
} // pullData()


// ---------------------------------- UP --------------------------------------
// sendStat()
// Send UP periodic status message to server even when we do not receive any
// data. 
// Parameters:
//	- <none>
// ----------------------------------------------------------------------------
void sendStat()
{

    uint8_t status_report[STATUS_SIZE]; 					// status report as a JSON object
    char stat_timestamp[32];								// 
	char clat[10]={0};
	char clon[10]={0};

    int stat_index=0;
	uint8_t token_h   = (uint8_t)rand(); 					// random token
    uint8_t token_l   = (uint8_t)rand();					// random token
	
    // pre-fill the data buffer with fixed fields
    status_report[0]  = PROTOCOL_VERSION;					// 0x01
	status_report[1]  = token_h;
    status_report[2]  = token_l;
    status_report[3]  = PUSH_DATA;							// 0x00
	
	// READ MAC ADDRESS OF ESP8266, and return unique Gateway ID consisting of MAC address and 2bytes 0xFF
    status_report[4]  = MAC_array[0];
    status_report[5]  = MAC_array[1];
    status_report[6]  = MAC_array[2];
    status_report[7]  = 0xFF;
    status_report[8]  = 0xFF;
    status_report[9]  = MAC_array[3];
    status_report[10] = MAC_array[4];
    status_report[11] = MAC_array[5];

    stat_index = 12;										// 12-byte header
	
	// XXX Using CET as the current timezone. Change to your timezone	
	sprintf(stat_timestamp, "%04d-%02d-%02d %02d:%02d:%02d CET", year(),month(),day(),hour(),minute(),second());
	//sprintf(stat_timestamp, "%04d-%02d-%02dT%02d:%02d:%02d.00000000Z", year(),month(),day(),hour(),minute(),second());

	ftoa(lat,clat,5);										// Convert lat to char array with 5 decimals
	ftoa(lon,clon,5);										// As IDE CANNOT prints floats

	// Build the Status message in JSON format, XXX Split this one up...
	yield();

    int j = snprintf((char *)(status_report + stat_index), STATUS_SIZE-stat_index, 
		"{\"stat\":{\"time\":\"%s\",\"lati\":%s,\"long\":%s,\"alti\":%i,\"rxnb\":%u,\"rxok\":%u,\"rxfw\":%u,\"ackr\":%u.0,\"dwnb\":%u,\"txnb\":%u,\"pfrm\":\"%s\",\"mail\":\"%s\",\"desc\":\"%s\"}}", 
		stat_timestamp, clat, clon, (int)alt, statc.msg_ttl, statc.msg_ok, statc.msg_down, 0, 0, 0, platform, email, description);

	yield();												// Give way to the internal housekeeping of the ESP8266

    stat_index += j;
    status_report[stat_index] = 0; 							// add string terminator, for safety

#	if _MONITOR>=1
    if ((debug>=2) && (pdebug & P_RX)) {
		mPrint("RX stat update: <"+String(stat_index)+"> "+String((char *)(status_report+12)) );
	}
#	endif //_MONITOR

	if (stat_index > STATUS_SIZE) {
#		if _MONITOR>=1
			mPrint("sendStat:: ERROR buffer too big");
#		endif //_MONITOR
		return;
	}

    //send the update
#	ifdef _TTNSERVER
		sendUdp(ttnServer, _TTNPORT, status_report, stat_index);
#	endif

#	ifdef _THINGSERVER
		yield();
		sendUdp(thingServer, _THINGPORT, status_report, stat_index);
#	endif
	return;

} // sendStat()


#endif //_UDPROUTER
