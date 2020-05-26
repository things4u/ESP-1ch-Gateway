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
// void sendstat();

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
// Read DOWN a package from UDP socket, can come from any server
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
//	Byte 4 >	Contains payload (or Gateway EUI 8 bytes first)
//
// Returns:
//	-1 or false if not read
//	Or number of characters read if success
//
// ----------------------------------------------------------------------------
int readUdp(int packetSize)
{ 
	uint8_t buff[32]; 						// General buffer to use for UDP, set to 32
	uint8_t buff_down[RX_BUFF_SIZE];		// Buffer for downstream

	// Make sure we are connected over WiFI
	if (WlanConnect(10) < 0) {
#		if _MONITOR>=1
			mPrint("Dwn readUdp:: ERROR connecting to WLAN");
#		endif //_MONITOR
		Udp.flush();
		return(-1);
	}
	
	if (packetSize > RX_BUFF_SIZE) {
#		if _MONITOR>=1
			mPrint("Dwn readUdp:: ERROR package of size: " + String(packetSize));
#		endif //_MONITOR
		Udp.flush();
		return(-1);
	}
	
	// We assume here that we know the originator of the message.
	// In practice however this can be any sender!
	if (Udp.read(buff_down, packetSize) < packetSize) {
#		if _MONITOR>=1
			mPrint("Dwn readUdp:: Reading less chars");
#		endif //_MONITOR
		return(-1);
	}

	yield();								// MMM 200406

	// Remote Address should be known
	IPAddress remoteIpNo = Udp.remoteIP();

	// Remote port is either of the remote TTN server or from NTP server (=123)
	
	unsigned int remotePortNo = Udp.remotePort();
	if (remotePortNo == 123) {				// NTP message arriving, not expected
		// This is an NTP message arriving
#		if _MONITOR>=1
		if (debug>=0) {
			mPrint("Dwn readUdp:: NTP msg rcvd");
		}
#		endif //_MONITOR
		gwayConfig.ntpErr++;
		gwayConfig.ntpErrTime = now();
		Udp.flush();						// MMM 200326 Clear buffer when time response arrives (error)
		return(0);
	}
	
	// If it is not NTP it must be a LoRa message for gateway or node
	
	else {
		uint8_t *data = (uint8_t *) ((uint8_t *)buff_down + 4);
		//uint8_t protocol= buff_down[0];
		//uint16_t token= buff_down[2]*256 + buff_down[1];
		uint8_t ident= buff_down[3];

#		if _MONITOR>=1
		if ((debug>=3) && (pdebug & P_TX)) {
			mPrint("Dwn readUdp:: message ident="+String(ident));
		}
#		endif //_MONITOR

		// now parse the message type from the server (if any)
		switch (ident) {


		// This message is used by the gateway to send sensor data UP to server. 
		// As this function is used for downstream only, this option
		// will never be selected but is included as a reference only
		// Para 5.2.1, Semtech Gateway to Server Interface document
		//	Byte 0:		Version
		//	byte 1+2:	Token
		//	byte 3: 	Command code (0x00)
		//
		case PKT_PUSH_DATA: 							// 0x00 UP
#			if _MONITOR>=1
			if (debug>=1) {
				mPrint("Dwn PKT_PUSH_DATA:: size "+String(packetSize)+" From "+String(remoteIpNo.toString()));
			}
#			endif //_MONITOR
			Udp.flush();
		break;


		// This message is sent DOWN by the server to acknowledge receipt of a
		// (sensor) PKT_PUSH_DATA message sent with the code above.
		// Para 5.2.2, Semtech Gateway to Server Interface document
		// The length of this package is 4 bytes:
		//	byte 0:		Protol version (0x01 or 0x02)
		//	byte 1+2:	Token copied from requestor
		//	byte 3:		0x01, ack PKT_PUSH_ACK
		//
		case PKT_PUSH_ACK:								// 0x01 DOWN
#			if _MONITOR>=1
			if ((debug>=2) && (pdebug & P_TX)) {
				char res[128];				
				sprintf(res, "Dwn PKT_PUSH_ACK:: size=%u, IP=%d.%d.%d.%d, port=%d ", 
					packetSize, 
					remoteIpNo[0], remoteIpNo[1], remoteIpNo[2],remoteIpNo[3], 
					remotePortNo);
				mPrint(res);
			}
#			endif //_MONITOR
			//Udp.flush();
		break;


		// PULL DATA message
		// This is a request/UP message and will not be executed by this function.
		// We have it here as a description only. 
		//	Para 5.2.3, Semtech Gateway to Server Interface document
		//
		case PKT_PULL_DATA:								// 0x02 UP
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_RX)) {
				mPrint("Dwn PKT_PULL_DATA");
			}
#			endif //_MONITOR
			Udp.flush();								// MMM 200419 Added
		break;


		// PULL_ACK message
		// This is the response to PKT_PULL_DATA message
		// Para 5.2.4, Semtech Gateway to Server Interface document
		// With this ACK, the server confirms the gateway that the route is open
		// for further PULL_RESP messages from the server (to the device)
		// The server sends a PULL_ACK to confirm PULL_DATA receipt, no response is needed
		//
		// Byte 0		contains Protocol Version == 2
		// Byte 1-2		Random Token
		// Byte 3		PULL_ACK ident == 0x04
		//
		case PKT_PULL_ACK:								// 0x04 DOWN
#			if _MONITOR>=1
			if ((debug>=2) && (pdebug & P_TX)) {
				char res[128];				
				sprintf(res, "Dwn PKT_PULL_ACK:: size=%u, IP=%d.%d.%d.%d, port=%d ", 
					packetSize, 
					remoteIpNo[0], remoteIpNo[1], remoteIpNo[2],remoteIpNo[3], 
					remotePortNo);
				mPrint(res);
			}
#			endif //_MONITOR
			
			yield();			
			
			Udp.flush();								// MMM 200419 
		break;



		// This message type is used to confirm OTAA message to the node,
		// but this message format will also be used for other downstream communication
		// It's length shall not exceed 1000 Octets. This is:
		//	RECEIVE_DELAY1		1 s 
		//	RECEIVE_DELAY2		2 s (is RECEIVE_DELAY1+1)
		//	JOIN_ACCEPT_DELAY1	5 s
		//	JOIN_ACCEPT_DELAY2	6 s
		// Para 5.2.5, Semtech Gateway to Server Interface document
		// or https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT
		//
		case PKT_PULL_RESP:								// 0x03 DOWN

			// Define when we start with the response to node
#			ifdef _PROFILER
			if ((debug>=2) && (pdebug & P_TX)) {
				mPrint("Dwn PKT_PULL_RESP:: start sendPacket: micros="+String(micros() ));
			}
#			endif //_PROFILER

			// Send to the LoRa Node first (timing) and then do reporting to _Monitor
			_state=S_TX;
			sendTime = micros();						// record when we started sending the message

			// Send the package DOWN to the sensor
			// We just read the packet from the Network Server and it is formatted
			// as described in the specs.
			if (sendPacket(data, packetSize-4) < 0) {
#				if _MONITOR>=1
				if (debug>=0) {
					mPrint("Dwn readUdp:: ERROR: PKT_PULL_RESP sendPacket failed");
				}
#				endif //_MONITOR
				Udp.flush();
				return(-1);
			}


			// We need a timeout for this case. In case there does not come an interrupt,
			// then there will not be a TXDONE but probably another CDDONE/CDDETD before
			// we have a timeout in the main program (Keep Alive)

			loraWait(&LoraDown);

			// Set state to transmit
			// Clear interrupt flags and masks
			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);			// MMM 200407 Reset
			writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);				// reset interrupt flags
			
			// Initiate the transmission of the buffer
			// (We normally react on ALL interrupts if we are in TX state)
			txLoraModem(&LoraDown);

#			if _MONITOR>=1
			if ((debug>=2) && (pdebug & P_TX)) {
				uint8_t flags = readRegister(REG_IRQ_FLAGS);
				uint8_t mask  = readRegister(REG_IRQ_FLAGS_MASK);
				uint8_t intr  = flags & ( ~ mask );


				String response = "Dwn readUdp:: PKT_PULL_RESP from IP="+String(remoteIpNo.toString())
					+", micros=" + String(micros())
					+", wait=";
				if (sendTime < micros()) {
					response += String(micros() - sendTime) ;
				}
				else {
					response += "-" + String(sendTime - micros()) ;
				};


				response+=", stat:: ";
				mStat(intr, response);
				mPrint(response); 

			}
#			endif //_MONITOR

			// After filling the buffer we only react on TXDONE interrupt
			// So, more or less start at the "case TXDONE:"  
			_state=S_TXDONE;
			_event=1;										// Or remove the break below

//			Udp.flush();									// MMM 200509 flush UDP buffer

			// No break!! so next secton will be executed

#		ifdef _PROFILER
		// measure the total time for transmissioon here
			if ((debug>=2) && (pdebug & P_TX)) {
				mPrint("Dwn PKT_PULL_RESP:: finit: micros="+String(micros() ));
			}
#		endif //_PROFILER

		// This is the response to the PKT_PULL_RESP message by the sensor device
		// it is sent by the gateway UP to the server to confirm the PULL_RESP message.
		//	byte 0:		Protocol version
		//	byte 1+2:	Port number of originator
		//	byte 3:		Message ID TX_ACK == 0x06
		//	byte 4-11:	Gateway ident
		//	byte 12-:	Optional Error Data
		//
		case PKT_TX_ACK:									// Message id: 0x05 UP

			if (buff_down[0]== 1) {
#				if _MONITOR>=1
				if ((debug>=3) && (pdebug & P_TX)) {
					mPrint("UP readUdp:: PKT_TX_ACK: protocol version 1");
					
					data = buff_down + 4;
					data[packetSize] = 0;
				}
#				endif
				break;										// return
			}

#			if _MONITOR>=1
			if ((debug>=2) && (pdebug & P_TX)) {
				mPrint("UP readUDP:: TX_ACK protocol version 2+");
			}
#			endif //_MONITOR


			// Now respond with an PKT_TX_ACK; UP 
			// Byte 3 is 0x05; see para 5.2.6 of spec
			buff[0]= buff_down[0];							// As read from the Network Server
			buff[1]= buff_down[1];							// Token 1
			buff[2]= buff_down[2];							// Token 2
			buff[3]= PKT_TX_ACK;							// ident == 0x05;
			buff[4]= MAC_array[0];
			buff[5]= MAC_array[1];
			buff[6]= MAC_array[2];
			buff[7]= 0xFF;
			buff[8]= 0xFF;
			buff[9]= MAC_array[3];
			buff[10]=MAC_array[4];
			buff[11]=MAC_array[5];
			buff[12]=0;										// Not error means "\0"
			// If it is an error, please look at para 6.1.2

			yield();

			// Only send the PKT_PULL_ACK to the UDP socket that just sent the data!!!
			Udp.beginPacket(remoteIpNo, remotePortNo);
			
			if (Udp.write((unsigned char *)buff, 12) != 12) {
#				if _MONITOR>=1
				if (debug>=0) {
					mPrint("UP readUdp:: ERROR: PKT_PULL_ACK write");
				}
#				endif //_MONITOR
			}
			else {
#				if _MONITOR>=1
				if ((debug>=2) && (pdebug & P_TX)) {
					mPrint("UP readUdp:: PKT_TX_ACK: micros="+String(micros()));
				}
#				endif //_MONITOR
			}

			if (!Udp.endPacket()) {
#				if _MONITOR>=1
				if ((debug>=0) && (pdebug & P_TX)) {
					mPrint("UP readUdp:: PKT_PULL_DATALL: ERROR Udp.endPacket");
				}
#				endif //_MONITOR
			}
			
			yield();

			// ONLY NOW WE START TO MONITOR THE PKT_PULL_RESP MESSAGE.
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_TX)) {
				data = buff_down + 4;
				data[packetSize] = 0;
				mPrint("Dwn readUdp:: PKT_PULL_RESP: size="+String(packetSize)+", data="+String((char *)data)); 
			}
#			endif //_MONITOR

		break;

		default:
#			if _GATEWAYMGT==1
				// For simplicity, we send the first 4 bytes too
				gateway_mgt(packetSize, buff_down);
			else

#			endif
#			if _MONITOR>=1
				mPrint(", ERROR ident not recognized="+String(ident));
#			endif //_MONITOR
		break;
		}
		
#		if _MONITOR>=1
		if ((debug>=3) && (pdebug & P_TX)) {
			String response= "Dwn readUdp:: ident="+String(ident,HEX);
			response+= ", tmst=" + String(LoraDown.tmst);
			response+= ", imme=" + String(LoraDown.imme);
			response+= ", sfTx=" + String(LoraDown.sfTx);
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
}//readUdp


// ----------------------------------- UP -------------------------------------
//
// sendUdp()
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
// Send UDP periodic Pull_DATA message UP to server to keepalive the connection
// and to invite the server to send downstream messages when these are available
// *2, par. 5.2
//	- Protocol Version (1 byte)
//	- Random Token (2 bytes)
//	- PULL_DATA identifier (1 byte) = 0x02
//	- Gateway unique identifier (8 bytes) = MAC address
// ----------------------------------------------------------------------------
void pullData()
{
    uint8_t pullDataReq[12]; 								// status report as a JSON object
    int pullIndex=0;
	
	uint8_t token_h = (uint8_t)rand(); 						// random token
    uint8_t token_l = (uint8_t)rand();						// random token
	
    // pre-fill the data buffer with fixed fields
    pullDataReq[0]  = PROTOCOL_VERSION;						// 0x01
    pullDataReq[1]  = token_h;
    pullDataReq[2]  = token_l;
    pullDataReq[3]  = PKT_PULL_DATA;						// 0x02
	// READ MAC ADDRESS OF ESP8266, and return unique Gateway ID consisting of MAC address and 2bytes 0xFF
    pullDataReq[4]  = MAC_array[0];
    pullDataReq[5]  = MAC_array[1];
    pullDataReq[6]  = MAC_array[2];
    pullDataReq[7]  = 0xFF;
    pullDataReq[8]  = 0xFF;
    pullDataReq[9]  = MAC_array[3];
    pullDataReq[10] = MAC_array[4];
    pullDataReq[11] = MAC_array[5];
    //pullDataReq[12] = 0/00; 								// add string terminator, for safety
	
    pullIndex = 12;											// 12-byte header
	
    //send the update
	
	uint8_t *pullPtr;
	pullPtr = pullDataReq,
#ifdef _TTNSERVER
    sendUdp(ttnServer, _TTNPORT, pullDataReq, pullIndex);
	yield();
#endif

#	if _MONITOR>=1
	if (pullPtr != pullDataReq) {
		mPrint("pullPtr != pullDatReq");
	}
#	endif //_MONITOR

#ifdef _THINGSERVER
	sendUdp(thingServer, _THINGPORT, pullDataReq, pullIndex);
#endif

#if _MONITOR>=1
    if ((debug>=2) && (pdebug & P_MAIN)) {
		yield();
		mPrint("M PKT_PULL_DATA request, len=" + String(pullIndex) );
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
// sendstat()
// Send UP periodic status message to server even when we do not receive any
// data. 
// Parameters:
//	- <none>
// ----------------------------------------------------------------------------
void sendstat()
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
    status_report[3]  = PKT_PUSH_DATA;						// 0x00
	
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
			mPrint("sendstat:: ERROR buffer too big");
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

} // sendstat()


#endif //_UDPROUTER
