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
// This file contains the utilities for time and other functions
// ========================================================================================





// ======================= STRING STRING STRING ===========================================

// --------------------------------------------------------------------------------
// PRINT INT
// The function printInt prints a number with Thousands seperator
// Paraneters:
//	i:			Integer containing Microseconds
//	response:	String & value containing the converted number
// Return:
//	<none>
// --------------------------------------------------------------------------------
void printInt (uint32_t i, String & response)
{
	response +=(String(i/1000000) + "." + String(i%1000000));
}


// --------------------------------------------------------------------------------
// PRINT REGS
//  Print the register settings
// --------------------------------------------------------------------------------
#define printReg(x) {int i=readRegister(x); if(i<=0x0F) Serial.print('0'); Serial.println(i,HEX);}

void printRegs(struct LoraDown *LoraDown, String & response)
{
	response += "v FIFO (0x00)=0x" + String(readRegister(REG_FIFO),HEX);
	response += "v OPMODE (0x01)=0x" + String(readRegister(REG_OPMODE),HEX);
	
#	if _DUSB>=1
	if (debug>=1) {
		Serial.print("v FIFO                 (0x00)\t=0x"); printReg(REG_FIFO);
		Serial.print("v OPMODE               (0x01)\t=0x"); printReg(REG_OPMODE);
		Serial.print("v FRF                  (0x06)\t=0x"); printReg(REG_FRF_MSB); 
			Serial.print("  "); printReg(REG_FRF_MID); 
			Serial.print("  "); printReg(REG_FRF_LSB);
		Serial.print("v PAC                  (0x09)\t=0x"); printReg(REG_PAC);
		Serial.print("v PARAMP               (0x0A)\t=0x"); printReg(REG_PARAMP);
		Serial.print("v REG_OCP              (0x0B)\t=0x"); printReg(REG_OCP);
		Serial.print("v LNA                  (0x0C)\t=0x"); printReg(REG_LNA);
		Serial.print("v FIFO_ADDR_PTR        (0x0D)\t=0x"); printReg(REG_FIFO_ADDR_PTR);
		Serial.print("v FIFO_TX_BASE_AD      (0x0E)\t=0x"); printReg(REG_FIFO_TX_BASE_AD);
		Serial.print("v FIFO_RX_BASE_AD      (0x0F)\t=0x"); printReg(REG_FIFO_RX_BASE_AD);

		Serial.print("v FIFO_RX_CURRENT_ADDR (0x10)\t=0x"); printReg(REG_FIFO_RX_CURRENT_ADDR);
		Serial.print("v IRQ_FLAGS_MASK       (0x11)\t=0x"); printReg(REG_IRQ_FLAGS_MASK);
		Serial.print("v IRQ_FLAGS            (0x12)\t=0x"); printReg(REG_IRQ_FLAGS);
		Serial.print("v MODEM_CONFIG1        (0x1D)\t=0x"); printReg(REG_MODEM_CONFIG1);
		Serial.print("v MODEM_CONFIG2        (0x1E)\t=0x"); printReg(REG_MODEM_CONFIG2);
		Serial.print("v MODEM_CONFIG3        (0x26)\t=0x"); printReg(REG_MODEM_CONFIG3);

		Serial.print("v PREAMBLE_MSB         (0x20)\t=0x"); printReg(REG_PREAMBLE_MSB);
		Serial.print("v PREAMBLE_LSB         (0x21)\t=0x"); printReg(REG_PREAMBLE_LSB);		
		Serial.print("v PAYLOAD_LENGTH       (0x22)\t="); Serial.println(readRegister(REG_PAYLOAD_LENGTH));
		Serial.print("v MAX_PAYLOAD_LENGTH   (0x23)\t="); Serial.println(readRegister(REG_MAX_PAYLOAD_LENGTH));
		Serial.print("v HOP_PERIOD           (0x24)\t="); Serial.println(readRegister(REG_HOP_PERIOD));
		Serial.print("v FIFO_RX_BYTE_ADDR_PTR(0x25)\t=0x"); printReg(REG_FIFO_RX_BYTE_ADDR_PTR);

		Serial.println("");
	}
#	endif // _DUSB


	return;
}



// --------------------------------------------------------------------------------
// PRINT Down
// In a uniform way, this function prints the LoraDown structure.
// This includes the timstamp, the current time and the time the function 
// must wait to execute. It will print all Downstream data
// Parameters:
//
// Returns:
//	<none>
// --------------------------------------------------------------------------------
void printDwn(struct LoraDown *LoraDown, String & response)
{
	uint32_t i= LoraDown->tmst;
	uint32_t m= micros();
	
	response += "micr=";	printInt(m, response);
	response += ", tmst=";	printInt(i, response);

	response += ", wait=";
	if (i>m) {										// Positive numbers
		response += String(i-m);
	}
	else {											// Negative numbers contain (number)
		response += "(";
		response += String(m-i);
		response += ")";
	}

	response += ", freq="	+String(LoraDown->freq);
	response += ", sf="		+String(LoraDown->sf);
	response += ", bw="		+String(LoraDown->bw);
	response += ", powe="	+String(LoraDown->powe);
	response += ", crc="	+String(LoraDown->crc);
	response += ", imme="	+String(LoraDown->imme);
	response += ", iiq="	+String(LoraDown->iiq, HEX);
	response += ", prea="	+String(LoraDown->prea);
	response += ", rfch="	+String(LoraDown->rfch);
	response += ", ncrc="	+String(LoraDown->ncrc);
	response += ", size="	+String(LoraDown->size);
	response += ", strict="	+String(_STRICT_1CH);

	response += ", a=";
	uint8_t DevAddr [4];
		DevAddr[0] = LoraDown->payLoad[4];
		DevAddr[1] = LoraDown->payLoad[3];
		DevAddr[2] = LoraDown->payLoad[2];
		DevAddr[3] = LoraDown->payLoad[1];
	printHex((IPAddress)DevAddr, ':', response);

	yield();

	return;
} // printDwn


// --------------------------------------------------------------------------------
// PRINT IP
// Output the 4-byte IP address for easy printing.
// As this function is also used by _otaServer.ino do not put in #define
// Parameters:
//	ipa: The Ip Address (input)
//	sep: Separator character (input)
//	response: The response string (output)
// Return:
//	<none>
// --------------------------------------------------------------------------------
void printIP(IPAddress ipa, const char sep, String & response)
{
	response += (String)ipa[0]; response += sep;
	response += (String)ipa[1]; response += sep;
	response += (String)ipa[2]; response += sep;
	response += (String)ipa[3];
}

// ----------------------------------------------------------------------------------------
// Fill a HEXadecimal String  from a 4-byte char array (uint32_t)
//
// ----------------------------------------------------------------------------------------
void printHex(uint32_t hexa, const char sep, String & response) 
{
#	if _MONITOR>=1
	if ((debug>=0) && (hexa==0)) {
		mPrint("printHex:: hexa amount to convert is 0");
	}
#	endif

	uint8_t * h = (uint8_t *)(& hexa);

	if (h[0]<016) response += '0'; response += String(h[0], HEX);  response+=sep;
	if (h[1]<016) response += '0'; response += String(h[1], HEX);  response+=sep;
	if (h[2]<016) response += '0'; response += String(h[2], HEX);  response+=sep;
	if (h[3]<016) response += '0'; response += String(h[3], HEX);  response+=sep;
}

// ----------------------------------------------------------------------------
// Print uint8_t values in HEX with leading 0 when necessary
// ----------------------------------------------------------------------------
void printHexDigit(uint8_t digit, String & response)
{
    // utility function for printing Hex Values with leading 0
    if(digit < 0x10)
        response += '0';
    response += String(digit,HEX);

}

// ========================= MONITOR FUNCTIONS ============================================
// Monitor functions
// These functions write to the Monitor and print the monitor.


// ----------------------------------------------------------------------------------------
// Print one line to the monitor console array.
// This function is used all over the gateway code as a substitute for USB debug code.
// It allows webserver users to view printed/debugging code.
// With initMonitor() we init the index iMoni=0;
//
// Parameters:
//	txt: The text to be printed.
// return:
//	<None>
// ----------------------------------------------------------------------------------------
void mPrint(String txt) 
{

#	if _DUSB>=1
	if (gwayConfig.dusbStat>=1) {
		Serial.println(txt);								// Copy to serial when configured
	}
#	endif //_DUSB

#if _MONITOR>=1
	time_t tt = now();
	
	monitor[iMoni].txt = "";
	stringTime(tt, monitor[iMoni].txt);
	
	monitor[iMoni].txt += "- " + String(txt);
	
	// Use the circular buffer to increment the index

	iMoni = (iMoni+1) % gwayConfig.maxMoni	;				// And goto 0 when skipping over _MAXMONITOR
	
#endif //_MONITOR

	return;
} //mPrint


// ----------------------------------------------------------------------------
// mStat (Monitor-Statistics)
// Print the statistics on Serial (USB) port and/or Monitor
// Depending on setting of _DUSB and _MONITOR.
// Note: This function does not initialise the response var, will only append.
// Parameters:
//	Interrupt:	8-bit
//	Response:	String
// Return:
//	1: If successful
//	0: No Success
// ----------------------------------------------------------------------------
int mStat(uint8_t intr, String & response) 
{
#if _MONITOR>=1

	if (debug>=0) {
	
		response += "I=";

		if (intr & IRQ_LORA_RXTOUT_MASK) response += "RXTOUT ";		// 0x80
		if (intr & IRQ_LORA_RXDONE_MASK) response += "RXDONE ";		// 0x40
		if (intr & IRQ_LORA_CRCERR_MASK) response += "CRCERR ";		// 0x20
		if (intr & IRQ_LORA_HEADER_MASK) response += "HEADER ";		// 0x10
		if (intr & IRQ_LORA_TXDONE_MASK) response += "TXDONE ";		// 0x08
		if (intr & IRQ_LORA_CDDONE_MASK) response += "CDDONE ";		// 0x04
		if (intr & IRQ_LORA_FHSSCH_MASK) response += "FHSSCH ";		// 0x02
		if (intr & IRQ_LORA_CDDETD_MASK) response += "CDDETD ";		// 0x01

		if (intr == 0x00) response += "  --  ";
			
		response += ", F=" + String(gwayConfig.ch);
		
		response += ", SF=" + String(sf);
		
		response += ", E=" + String(_event);
			
		response += ", S=";
		switch (_state) {
			case S_TXDONE:
				response += "TXDONE";
			break;
			case S_INIT:
				response += "INIT ";
			break;
			case S_SCAN:
				response += "SCAN ";
			break;
			case S_CAD:
				response += "CAD  ";
			break;
			case S_RX:
				response += "RX   ";
			break;
			case S_TX:
				response += "TX   ";
			break;
			default:
				response += " -- ";
		}
		response += ", eT=";
		response += String( micros() - eventTime );
		
		response += ", dT=";
		response += String( micros() - doneTime );
	}
#endif //_MONITOR
	return(1);
}




// ============== NUMBER FUNCTIONS ============================================


// ----------------------------------------------------------------------------
// Convert a float to string for printing
// Parameters:
//	f is float value to convert
//	p is precision in decimal digits
//	val is character array for results
// ----------------------------------------------------------------------------
void ftoa(float f, char *val, int p) 
{
	int j=1;
	int ival, fval;
	char b[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	
	for (int i=0; i< p; i++) { j= j*10; }

	ival = (int) f;											// Make integer part
	fval = (int) ((f- ival)*j);								// Make fraction. Has same sign as integer part
	if (fval<0) fval = -fval;								// So if it is negative make fraction positive again.
															// sprintf does NOT fit in memory
	if ((f<0) && (ival == 0)) strcat(val, "-");
	strcat(val,itoa(ival,b,10));							// Copy integer part first, base 10, null terminated
	strcat(val,".");										// Copy decimal point
	
	itoa(fval,b,10);										// Copy fraction part base 10
	for (unsigned int i=0; i<(p-strlen(b)); i++) {
		strcat(val,"0");									// first number of 0 of faction?
	}
	
	// Fraction can be anything from 0 to 10^p , so can have less digits
	strcat(val,b);
}



// ============== SERIAL SERIAL SERIAL ========================================


// ----------------------------------------------------------------------------
// Print leading '0' digits for hours(0) and second(0) when
// printing values less than 10
// ----------------------------------------------------------------------------
void printDigits(uint32_t digits)
{
    // utility function for digital clock display: prints leading 0
    if(digits < 10)
        Serial.print(F("0"));
    Serial.print(digits);
}


// ============================================================================
// NTP TIME functions
// These helper function deal with the Network Time Protool(NTP) functions.
// ============================================================================



// ----------------------------------------------------------------------------------------
// stringTime
// Print the time t into the String reponse. t is of type time_t in seconds.
// Only when RTC is present we print real time values
// t contains number of seconds since system started that the event happened.
// So a value of 100 would mean that the event took place 1 minute and 40 seconds ago
// ----------------------------------------------------------------------------------------
static void stringTime(time_t t, String & response) 
{

	if (t==0) { response += "--"; return; }
	
	// now() gives seconds since 1970
	// as millis() does rotate every 50 days
	// So we need another timing parameter
	time_t eTime = t;
	
	// Rest is standard
	byte _hour   = hour(eTime);
	byte _minute = minute(eTime);
	byte _second = second(eTime);
	
	byte _month = month(eTime);
	byte _day = day(eTime);
	
	switch(weekday(eTime)) {
		case 1: response += "Sun "; break;
		case 2: response += "Mon "; break;
		case 3: response += "Tue "; break;
		case 4: response += "Wed "; break;
		case 5: response += "Thu "; break;
		case 6: response += "Fri "; break;
		case 7: response += "Sat "; break;
	}
	if (_day < 10) response += "0"; response += String(_day) + "-";
	if (_month < 10) response += "0"; response += String(_month) + "-";
	response += String(year(eTime)) + " ";	
	
	if (_hour < 10) response += "0";   response += String(_hour) + ":";
	if (_minute < 10) response += "0"; response += String(_minute) + ":";
	if (_second < 10) response += "0"; response += String(_second);
}

// ----------------------------------------------------------------------------
// Send the time request packet to the NTP server.
//
// ----------------------------------------------------------------------------
int sendNtpRequest(IPAddress timeServerIP) 
{
	const int NTP_PACKET_SIZE = 48;							// Fixed size of NTP record
	byte packetBuffer[NTP_PACKET_SIZE];

	memset(packetBuffer, 0, NTP_PACKET_SIZE);				// Zero the buffer.
	
	packetBuffer[0]  = 0b11100011;   						// LI, Version, Mode
	packetBuffer[1]  = 0;									// Stratum, or type of clock
	packetBuffer[2]  = 6;									// Polling Interval
	packetBuffer[3]  = 0xEC;								// Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;	

	
	if (!sendUdp( (IPAddress) timeServerIP, (int) 123, packetBuffer, NTP_PACKET_SIZE)) {
		gwayConfig.ntpErr++;
		gwayConfig.ntpErrTime = now();
		return(0);	
	}
	return(1);
	
} // sendNtpRequest()


// ----------------------------------------------------------------------------
// Get the NTP time from one of the time servers
// Note: As this function is called from SyncInterval in the background
//	make sure we have no blocking calls in this function
// parameters:
//	t: the resulting time_t 
// return:
//	0: when fail
//	>=1: when success
// ----------------------------------------------------------------------------
int getNtpTime(time_t *t)
{
	gwayConfig.ntps++;
	
    if (!sendNtpRequest(ntpServer))							// Send the request for new time
	{
#		if _MONITOR>=1
		if (debug>=0) {
			mPrint("utils:: ERROR getNtpTime: sendNtpRequest failed");
		}
#		endif //_MONITOR
		return(0);
	}
	
	const int NTP_PACKET_SIZE = 48;							// Fixed size of NTP record
	byte packetBuffer[NTP_PACKET_SIZE];
	memset(packetBuffer, 0, NTP_PACKET_SIZE);				// Set buffer contents to zero

    uint32_t beginWait = millis();
	delay(10);
    while (millis() - beginWait < 1500) 					// Wait for 1500 millisecs
	{
		int size = Udp.parsePacket();
		if ( size >= NTP_PACKET_SIZE ) {
		
			if (Udp.read(packetBuffer, NTP_PACKET_SIZE) < NTP_PACKET_SIZE) {
#				if _MONITOR>=1
				if (debug>=0) {
					mPrint("getNtpTime:: ERROR packetsize too low");
				}
#				endif //_MONITOR
				break;										// Error, or should we use continue
			}
			else {
				// Extract seconds portion.
				uint32_t secs;
				secs  = packetBuffer[40] << 24;
				secs |= packetBuffer[41] << 16;
				secs |= packetBuffer[42] <<  8;
				secs |= packetBuffer[43];
				
				// in NL UTC is 1 TimeZone correction when no daylight saving time
				*t = (time_t)(secs - 2208988800UL + NTP_TIMEZONES * SECS_IN_HOUR);
				Udp.flush();
				return(1);
			}
			Udp.flush();	
		}
		delay(100);											// Wait 100 millisecs, allow kernel to act when necessary
    }

	Udp.flush();

	// If we are here, we could not read the time from internet
	// So increase the counter
	gwayConfig.ntpErr++;
	gwayConfig.ntpErrTime = now();

#	if _MONITOR>=1
	if ((debug>=3) && (pdebug & P_MAIN)) {
		mPrint("getNtpTime:: WARNING read time failed"); 	// but we return 0 to indicate this to caller
	}
#	endif //_MONITOR

	return(0); 												// return 0 if unable to get the time
} //getNtpTime


// ----------------------------------------------------------------------------
// Set up regular synchronization of NTP server and the local time.
// ----------------------------------------------------------------------------
#if NTP_INTR==1
void setupTime() 
{
  time_t t;
  getNtpTime(&t);
  setSyncProvider(t);
  setSyncInterval(_NTP_INTERVAL);
}
#endif //NTP_INTR



	
// ----------------------------------------------------------------------------
// SerialName(id, response)
// Check whether for address a (4 bytes in uint32_t) there is a 
// Trusted Node name. It will return the index of that name in nodex struct.
// Otherwise it returns -1.
// This function only works if _TRUSTED_NODES is set.
// ----------------------------------------------------------------------------

int SerialName(uint32_t a, String & response)
{
#if _TRUSTED_NODES>=1
	uint8_t * in = (uint8_t *)(& a);
	uint32_t id = ((in[0]<<24) | (in[1]<<16) | (in[2]<<8) | in[3]);

	for (unsigned int i=0; i< (sizeof(nodes)/sizeof(nodex)); i++) {

		if (id == nodes[i].id) {
#			if _MONITOR>=1
			if ((debug>=3) && (pdebug & P_MAIN )) {
				mPrint("SerialName:: i="+String(i)+", Name="+String(nodes[i].nm)+". for node=0x"+String(nodes[i].id,HEX));
			}
#			endif //_MONITOR

			response += nodes[i].nm;
			return(i);
		}
	}
#endif //_TRUSTED_NODES

	return(-1);									// If no success OR is TRUSTED NODES not defined
} //SerialName


#if _LOCALSERVER>=1
// ----------------------------------------------------------------------------
// inDecodes(id)
// Find the id in Decodes array, and return the index of the item
// Parameters:
//		id: The first field in the array (normally DevAddr id). Must be char[4]
// Returns:
//		The index of the ID in the Array. Returns -1 if not found
// ----------------------------------------------------------------------------
int inDecodes(char * id) {

	uint32_t ident = ((id[3]<<24) | (id[2]<<16) | (id[1]<<8) | id[0]);

	for (unsigned int i=0; i< (sizeof(decodes)/sizeof(codex)); i++) {
		if (ident == decodes[i].id) {
			return(i);
		}
	}
	return(-1);
}
#endif //_LOCALSERVER





// ============================= GENERAL SKETCH ===============================

// ----------------------------------------------------------------------------
// DIE is not used actively in the source code apart from resolveHost().
// It is replaced by a Serial.print command so we know that we have a problem
// somewhere.
// There are at least 3 other ways to restart the ESP. Pick one if you want.
// ----------------------------------------------------------------------------
void die(String s)
{
#	if _MONITOR>=1
	mPrint(s);
#	endif //_MONITOR

#	if _DUSB>=1
	Serial.println(s);
	if (debug>=2) Serial.flush();
#	endif //_DUSB

	delay(50);
	abort();												// Within a second
}


// ----------------------------------------------------------------------------
// gway_failed is a function called by ASSERT in configGway.h
//
// ----------------------------------------------------------------------------
void gway_failed(const char *file, uint16_t line) {
#if  _MONITOR>=1
	String response = "Program failed in file: ";
	response += String(file);
	response += ", line: ";
	response += String(line);
	mPrint(response);
#endif //_MONITOR
}
