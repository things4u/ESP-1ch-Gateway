// 1-channel LoRa Gateway for ESP8266
// Copyright (c) 2016, 2017, 2018, 2019 Maarten Westenberg version for ESP8266
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
// int sendUdp(IPAddress server, int port, uint8_t *msg, int length)
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


// ----------------------------------------------------------------------------
// DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN 
// readUdp()
// Read DOWN a package from UDP socket, can come from any server
// Messages are received when server responds to gateway requests from LoRa nodes 
// (e.g. JOIN requests etc.) or when server has downstream data.
// We respond only to the server that sent us a message!
//
// Note: So normally we can forget here about codes that do upstream
//
// Parameters:
//	Packetsize: size of the buffer to read, as read by loop() calling function
//
// Returns:
//	-1 or false if not read
//	Or number of characters read is success
//
// ----------------------------------------------------------------------------
int readUdp(int packetSize)
{
	uint8_t protocol;
	uint16_t token;
	uint8_t ident; 
	uint8_t buff[32]; 						// General buffer to use for UDP, set to 64
	uint8_t buff_down[RX_BUFF_SIZE];		// Buffer for downstream

	if (WlanConnect(10) < 0) {
#		if _MONITOR>=1
			mPrint("readUdp: ERROR connecting to WLAN");
#		endif //_MONITOR
		Udp.flush();
		yield();
		return(-1);
	}

	yield();
	
	if (packetSize > RX_BUFF_SIZE) {
#		if _MONITOR>=1
			mPrint("readUdp:: ERROR package of size: " + String(packetSize));
#		endif //_MONITOR
		Udp.flush();
		return(-1);
	}
  
	// We assume here that we know the originator of the message.
	// In practice however this can be any sender!
	if (Udp.read(buff_down, packetSize) < packetSize) {
#		if _MONITOR>=1
			mPrint("A readUdp:: Reading less chars");
#		endif //_MONITOR
		return(-1);
	}

	// Remote Address should be known
	IPAddress remoteIpNo = Udp.remoteIP();

	// Remote port is either of the remote TTN server or from NTP server (=123)
	unsigned int remotePortNo = Udp.remotePort();

	if (remotePortNo == 123) {
		// This is an NTP message arriving
#		if _MONITOR>=1
		if ( debug>=0 ) {
			mPrint("A readUdp:: NTP msg rcvd");
		}
#		endif //_MONITOR
		gwayConfig.ntpErr++;
		gwayConfig.ntpErrTime = now();
		return(0);
	}
	
	// If it is not NTP it must be a LoRa message for gateway or node
	else {
		uint8_t *data = (uint8_t *) ((uint8_t *)buff_down + 4);
		protocol = buff_down[0];
		token = buff_down[2]*256 + buff_down[1];
		ident = buff_down[3];

#		if _MONITOR>=1
		if ((debug>1) && (pdebug & P_MAIN)) {
			mPrint("M readUdp:: message waiting="+String(ident));
		}
#		endif //_MONITOR

		// now parse the message type from the server (if any)
		switch (ident) {

		// This message is used by the gateway to send sensor data to the server. 
		// As this function is used for downstream only, this option
		// will never be selected but is included as a reference only
		case PKT_PUSH_DATA: // 0x00 UP
#			if _MONITOR>=1
			if (debug >=1) {
				mPrint("PKT_PUSH_DATA:: size "+String(packetSize)+" From "+String(remoteIpNo.toString()));

//				Serial.print(F(", port ")); Serial.print(remotePortNo);
//				Serial.print(F(", data: "));
//				for (int i=0; i<packetSize; i++) {
//					Serial.print(buff_down[i],HEX);
//					Serial.print(':');
//				}
//				Serial.println();
//				if (debug>=2) Serial.flush();
			}
#		endif //_MONITOR
		break;
	
		// This message is sent by the server to acknowledge receipt of a
		// (sensor) message sent with the code above.
		case PKT_PUSH_ACK:	// 0x01 DOWN
#if _MONITOR>=1
			if (( debug>=2) && (pdebug & P_MAIN )) {
				mPrint("M PKT_PUSH_ACK:: size="+String(packetSize)+" From "+String(remoteIpNo.toString())); 
//				Serial.print(F(", port ")); 
//				Serial.print(remotePortNo);
//				Serial.print(F(", token: "));
//				Serial.println(token, HEX);
//				Serial.println();
			}
#endif //_MONITOR
		break;
	
		case PKT_PULL_DATA:	// 0x02 UP
#if _DUSB>=1
			Serial.print(F(" Pull Data"));
			Serial.println();
#endif
		break;
	
		// This message type is used to confirm OTAA message to the node
		// XXX This message format may also be used for other downstream communication
		case PKT_PULL_RESP:	// 0x03 DOWN
#			if _MONITOR>=1
			if (( debug>=0 ) && ( pdebug & P_MAIN )) {
				mPrint("readUdp:: PKT_PULL_RESP received");
			}
#			endif //_MONITOR
//			lastTmst = micros();					// Store the tmst this package was received
			
			// Send to the LoRa Node first (timing) and then do reporting to Serial
			_state=S_TX;
			sendTime = micros();					// record when we started sending the message
			
			if (sendPacket(data, packetSize-4) < 0) {
#				if _MONITOR>=1
				if ( debug>=0 ) {
					mPrint("A readUdp:: ERROR: PKT_PULL_RESP sendPacket failed");
				}
#				endif //_MONITOR
				return(-1);
			}

			// Now respond with an PKT_TX_ACK; 0x04 UP
			buff[0]=buff_down[0];
			buff[1]=buff_down[1];
			buff[2]=buff_down[2];
			//buff[3]=PKT_PULL_ACK;					// Pull request/Change of Mogyi
			buff[3]=PKT_TX_ACK;
			buff[4]=MAC_array[0];
			buff[5]=MAC_array[1];
			buff[6]=MAC_array[2];
			buff[7]=0xFF;
			buff[8]=0xFF;
			buff[9]=MAC_array[3];
			buff[10]=MAC_array[4];
			buff[11]=MAC_array[5];
			buff[12]=0;
#			if _MONITOR>=1
			if (( debug >= 2 ) && ( pdebug & P_MAIN )) {
				mPrint("M readUdp:: TX buff filled");
			}
#			endif //_MONITOR
			// Only send the PKT_PULL_ACK to the UDP socket that just sent the data!!!
			Udp.beginPacket(remoteIpNo, remotePortNo);
			if (Udp.write((unsigned char *)buff, 12) != 12) {
#if _DUSB>=1
				if (debug>=0)
					Serial.println("A readUdp:: Error: PKT_PULL_ACK UDP write");
#endif
			}
			else {
#if _DUSB>=1
				if (( debug>=0 ) && ( pdebug & P_TX )) {
					Serial.print(F("M PKT_TX_ACK:: micros="));
					Serial.println(micros());
				}
#endif
			}

			if (!Udp.endPacket()) {
#if _DUSB>=1
				if (( debug>=0 ) && ( pdebug & P_MAIN )) {
					Serial.println(F("M PKT_PULL_DATALL Error Udp.endpaket"));
				}
#endif
			}
			
			yield();
#if _DUSB>=1
			if (( debug >=1 ) && (pdebug & P_MAIN )) {
				Serial.print(F("M PKT_PULL_RESP:: size ")); 
				Serial.print(packetSize);
				Serial.print(F(" From ")); 
				Serial.print(remoteIpNo);
				Serial.print(F(", port ")); 
				Serial.print(remotePortNo);	
				Serial.print(F(", data: "));
				data = buff_down + 4;
				data[packetSize] = 0;
				Serial.print((char *)data);
				Serial.println(F("..."));
			}
#endif		
		break;
	
		case PKT_PULL_ACK:	// 0x04 DOWN; the server sends a PULL_ACK to confirm PULL_DATA receipt
#if _DUSB>=1
			if (( debug >= 2 ) && (pdebug & P_MAIN )) {
				Serial.print(F("M PKT_PULL_ACK:: size ")); Serial.print(packetSize);
				Serial.print(F(" From ")); Serial.print(remoteIpNo);
				Serial.print(F(", port ")); Serial.print(remotePortNo);	
				Serial.print(F(", data: "));
				for (int i=0; i<packetSize; i++) {
					Serial.print(buff_down[i],HEX);
					Serial.print(':');
				}
				Serial.println();
			}
#endif
		break;
	
		default:
#if GATEWAYMGT==1
			// For simplicity, we send the first 4 bytes too
			gateway_mgt(packetSize, buff_down);
#else

#endif
#			if _MONITOR>=1
				mPrint(", ERROR ident not recognized="+String(ident));
#			endif //_MONITOR
		break;
		}
#		if _MONITOR>=2
		if (debug>=2) {
			mPrint("readUdp:: returning=" + String(packetSize)); 
		}
#		endif //_MONITOR
		// For downstream messages
		return packetSize;
	}
}//readUdp


// ----------------------------------------------------------------------------
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
int sendUdp(IPAddress server, int port, uint8_t *msg, int length) {

	// Check whether we are conected to Wifi and the internet
	if (WlanConnect(3) < 0) {
#		if _MONITOR>=1
		if (( debug>=0 ) && ( pdebug & P_MAIN )) {
			mPrint("sendUdp: ERROR connecting to WiFi");
		}
#		endif //_MONITOR
		Udp.flush();
		yield();
		return(0);
	}

	yield();

	//send the update
#	if _MONITOR>=1
	if (( debug>=3 ) && ( pdebug & P_MAIN )) {
		mPrint("M WiFi connected");
	}
#	endif //_MONITOR

	if (!Udp.beginPacket(server, (int) port)) {
#		if _MONITOR>=1
		if (( debug>=1 ) && ( pdebug & P_MAIN )) {
			mPrint("M sendUdp:: Error Udp.beginPacket");
		}
#		endif //_MONITOR
		return(0);
	}
	
	yield();

	if (Udp.write((unsigned char *)msg, length) != length) {
#		if _MONITOR>=1
		if (( debug<=1 ) && ( pdebug & P_MAIN )) {
			mPrint("M sendUdp:: Error write");
		}
#		endif //_MONITOR
		Udp.endPacket();						// Close UDP
		return(0);								// Return error
	}
	
	yield();
	
	if (!Udp.endPacket()) {
#	if _MONITOR>=1
		if (debug>=1) {
			mPrint("sendUdp:: Error Udp.endPacket");
		}
#	endif //_MONITOR
		return(0);
	}
	return(1);
}//sendUDP




// ----------------------------------------------------------------------------
// pullData()
// Send UDP periodic Pull_DATA message to server to keepalive the connection
// and to invite the server to send downstream messages when these are available
// *2, par. 5.2
//	- Protocol Version (1 byte)
//	- Random Token (2 bytes)
//	- PULL_DATA identifier (1 byte) = 0x02
//	- Gateway unique identifier (8 bytes) = MAC address
// ----------------------------------------------------------------------------
void pullData() {

    uint8_t pullDataReq[12]; 								// status report as a JSON object
    int pullIndex=0;
	int i;
	
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

#if _DUSB>=1
    if (( debug>=2 ) && ( pdebug & P_MAIN )) {
		yield();
		Serial.print(F("M PKT_PULL_DATA request, len=<"));
		Serial.print(pullIndex);
		Serial.print(F("> "));
		for (i=0; i<pullIndex; i++) {
			Serial.print(pullDataReq[i],HEX);				// debug: display JSON stat
			Serial.print(':');
		}
		Serial.println();
		if (debug>=2) Serial.flush();
	}
#endif
	return;
}//pullData


// ----------------------------------------------------------------------------
// sendstat()
// Send UP periodic status message to server even when we do not receive any
// data. 
// Parameters:
//	- <none>
// ----------------------------------------------------------------------------
void sendstat() {

    uint8_t status_report[STATUS_SIZE]; 					// status report as a JSON object
    char stat_timestamp[32];								// XXX was 24
    time_t t;
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
	
    t = now();												// get timestamp for statistics
	
	// XXX Using CET as the current timezone. Change to your timezone	
	sprintf(stat_timestamp, "%04d-%02d-%02d %02d:%02d:%02d CET", year(),month(),day(),hour(),minute(),second());
	yield();
	
	ftoa(lat,clat,5);										// Convert lat to char array with 5 decimals
	ftoa(lon,clon,5);										// As IDE CANNOT prints floats
	
	// Build the Status message in JSON format, XXX Split this one up...
	delay(1);
	
    int j = snprintf((char *)(status_report + stat_index), STATUS_SIZE-stat_index, 
		"{\"stat\":{\"time\":\"%s\",\"lati\":%s,\"long\":%s,\"alti\":%i,\"rxnb\":%u,\"rxok\":%u,\"rxfw\":%u,\"ackr\":%u.0,\"dwnb\":%u,\"txnb\":%u,\"pfrm\":\"%s\",\"mail\":\"%s\",\"desc\":\"%s\"}}", 
		stat_timestamp, clat, clon, (int)alt, statc.msg_ttl, statc.msg_ok, statc.msg_down, 0, 0, 0, platform, email, description);
		
	yield();												// Give way to the internal housekeeping of the ESP8266

    stat_index += j;
    status_report[stat_index] = 0; 							// add string terminator, for safety

#if _DUSB>=1
    if (( debug>=2 ) && ( pdebug & P_MAIN )) {
		Serial.print(F("M stat update: <"));
		Serial.print(stat_index);
		Serial.print(F("> "));
		Serial.println((char *)(status_report+12));			// DEBUG: display JSON stat
	}
#endif	
	if (stat_index > STATUS_SIZE) {
#		if _MONITOR>=1
			mPrint("A sendstat:: ERROR buffer too big");
#		endif //_MONITOR
		return;
	}
	
    //send the update
#ifdef _TTNSERVER
    sendUdp(ttnServer, _TTNPORT, status_report, stat_index);
	yield();
#endif

#ifdef _THINGSERVER
	sendUdp(thingServer, _THINGPORT, status_report, stat_index);
#endif
	return;
}//sendstat


#endif //_UDPROUTER
