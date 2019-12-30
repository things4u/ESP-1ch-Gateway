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
// This file contains the utilities for time and other functions
// ========================================================================================


// ==================== STRING STRING STRING ==============================================


// ----------------------------------------------------------------------------------------
// Print to the monitor console.
// This function is used all over the gateway code as a substitue for USB debug code.
// It allows webserver users to view printed/debugging code.
//
// Parameters:
//	txt: The text to be printed.
// return:
//	<None>
// ----------------------------------------------------------------------------------------

static void mPrint(String txt) 
{
	
#	if DUSB>=1
		Serial.println(F(txt));
		if (debug>=2) Serial.flush();
#	endif //_DUSB

#	if _MONITOR>=1
	time_t tt = now();
	
	for (int i=_MONITOR; i>0; i--) {		// Do only for values present....
		monitor[i]= monitor[i-1];
	}
	
	monitor[0].txt  = String(day(tt))	+ "-";
	monitor[0].txt += String(month(tt))	+ "-";
	monitor[0].txt += String(year(tt))	+ " ";
	
	byte _hour   = hour(tt);
	byte _minute = minute(tt);
	byte _second = second(tt);

	if (_hour   < 10) monitor[0].txt += "0"; monitor[0].txt += String( _hour )+ ":";
	if (_minute < 10) monitor[0].txt += "0"; monitor[0].txt += String(_minute) + ":";
	if (_second < 10) monitor[0].txt += "0"; monitor[0].txt += String(_second) + "- ";
	
	monitor[0].txt += String(txt);
	
#	endif //_MONITOR
	return;
}


// ----------------------------------------------------------------------------
// mStat
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

		if (intr & IRQ_LORA_RXTOUT_MASK) response += String("RXTOUT ");		// 0x80
		if (intr & IRQ_LORA_RXDONE_MASK) response += String("RXDONE ");		// 0x40
		if (intr & IRQ_LORA_CRCERR_MASK) response += "CRCERR ";		// 0x20
		if (intr & IRQ_LORA_HEADER_MASK) response += "HEADER ";		// 0x10
		if (intr & IRQ_LORA_TXDONE_MASK) response += "TXDONE ";		// 0x08
		if (intr & IRQ_LORA_CDDONE_MASK) response += String("CDDONE ");		// 0x04
		if (intr & IRQ_LORA_FHSSCH_MASK) response += "FHSSCH ";		// 0x02
		if (intr & IRQ_LORA_CDDETD_MASK) response += "CDDETD ";		// 0x01

		if (intr == 0x00) response += "  --  ";
			
		response += ", F=" + String(ifreq);
		response += ", SF=" + String(sf);
		response += ", E=" + String(_event);
			
		response += ", S=";

		switch (_state) {
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
				response += String("RX   ");
			break;
			case S_TX:
				response += "TX   ";
			break;
			case S_TXDONE:
				response += "TXDONE";
			break;
			default:
				response += " -- ";
		}
		response += ", eT=";
		response += String( micros() - eventTime );
		response += ", dT=";
		response += String( micros() - doneTime );

		//mPrint(response);							// Do actual printing
	}
#endif //_MONITOR
	return(1);
}


// ----------------------------------------------------------------------------------------
// Fill a HEXadecimal String  from a 4-byte char array
//
// ----------------------------------------------------------------------------------------
static void printHEX(char * hexa, const char sep, String& response) 
{
	char m;
	m = hexa[0]; if (m<016) response+='0'; response += String(m, HEX);  response+=sep;
	m = hexa[1]; if (m<016) response+='0'; response += String(m, HEX);  response+=sep;
	m = hexa[2]; if (m<016) response+='0'; response += String(m, HEX);  response+=sep;
	m = hexa[3]; if (m<016) response+='0'; response += String(m, HEX);  response+=sep;
}



// ----------------------------------------------------------------------------------------
// stringTime
// Print the time t into the String reponse. t is of type time_t in seconds.
// Only when RTC is present we print real time values
// t contains number of seconds since system started that the event happened.
// So a value of 100 would mean that the event took place 1 minute and 40 seconds ago
// ----------------------------------------------------------------------------------------
static void stringTime(time_t t, String& response) {

	if (t==0) { response += "--"; return; }
	
	// now() gives seconds since 1970
	// as millis() does rotate every 50 days
	// So we need another timing parameter
	time_t eTime = t;
	
	// Rest is standard
	byte _hour   = hour(eTime);
	byte _minute = minute(eTime);
	byte _second = second(eTime);
	
	switch(weekday(eTime)) {
		case 1: response += "Sunday "; break;
		case 2: response += "Monday "; break;
		case 3: response += "Tuesday "; break;
		case 4: response += "Wednesday "; break;
		case 5: response += "Thursday "; break;
		case 6: response += "Friday "; break;
		case 7: response += "Saturday "; break;
	}
	response += String(day(eTime)) + "-";
	response += String(month(eTime)) + "-";
	response += String(year(eTime)) + " ";	
	if (_hour < 10) response += "0";   response += String(_hour) + ":";
	if (_minute < 10) response += "0"; response += String(_minute) + ":";
	if (_second < 10) response += "0"; response += String(_second);
}


// ============== SERIAL SERIAL SERIAL ========================================

// ----------------------------------------------------------------------------
// Print utin8_t values in HEX with leading 0 when necessary
// ----------------------------------------------------------------------------
void printHexDigit(uint8_t digit)
{
    // utility function for printing Hex Values with leading 0
    if(digit < 0x10)
        Serial.print('0');
    Serial.print(digit,HEX);
}

// ----------------------------------------------------------------------------
// Print leading '0' digits for hours(0) and second(0) when
// printing values less than 10
// ----------------------------------------------------------------------------
void printDigits(unsigned long digits)
{
    // utility function for digital clock display: prints leading 0
    if(digits < 10)
        Serial.print(F("0"));
    Serial.print(digits);
}


// ----------------------------------------------------------------------------
// Print the current time
// ----------------------------------------------------------------------------
static void printTime() {
	switch (weekday())
	{
		case 1: Serial.print(F("Sunday")); break;
		case 2: Serial.print(F("Monday")); break;
		case 3: Serial.print(F("Tuesday")); break;
		case 4: Serial.print(F("Wednesday")); break;
		case 5: Serial.print(F("Thursday")); break;
		case 6: Serial.print(F("Friday")); break;
		case 7: Serial.print(F("Saturday")); break;
		default: Serial.print(F("ERROR")); break;
	}
	Serial.print(F(" "));
	printDigits(hour());
	Serial.print(F(":"));
	printDigits(minute());
	Serial.print(F(":"));
	printDigits(second());
	return;
}


// ----------------------------------------------------------------------------
// SerialTime
// Print the current time on the Serial (USB), with leading 0.
// ----------------------------------------------------------------------------
void SerialTime() 
{

	uint32_t thrs = hour();
	uint32_t tmin = minute();
	uint32_t tsec = second();
			
	if (thrs<10) Serial.print('0'); Serial.print(thrs);
	Serial.print(':');
	if (tmin<10) Serial.print('0'); Serial.print(tmin);
	Serial.print(':');
	if (tsec<10) Serial.print('0'); Serial.print(tsec);
			
	if (debug>=2) Serial.flush();
		
	return;
}

// ----------------------------------------------------------------------------
// SerialStat
// Print the statistics on Serial (USB) port
// ----------------------------------------------------------------------------

void SerialStat(uint8_t intr) 
{
#if _DUSB>=1
	if (debug>=0) {
		Serial.print(F("I="));

		if (intr & IRQ_LORA_RXTOUT_MASK) Serial.print(F("RXTOUT "));		// 0x80
		if (intr & IRQ_LORA_RXDONE_MASK) Serial.print(F("RXDONE "));		// 0x40
		if (intr & IRQ_LORA_CRCERR_MASK) Serial.print(F("CRCERR "));		// 0x20
		if (intr & IRQ_LORA_HEADER_MASK) Serial.print(F("HEADER "));		// 0x10
		if (intr & IRQ_LORA_TXDONE_MASK) Serial.print(F("TXDONE "));		// 0x08
		if (intr & IRQ_LORA_CDDONE_MASK) Serial.print(F("CDDONE "));		// 0x04
		if (intr & IRQ_LORA_FHSSCH_MASK) Serial.print(F("FHSSCH "));		// 0x02
		if (intr & IRQ_LORA_CDDETD_MASK) Serial.print(F("CDDETD "));		// 0x01

		if (intr == 0x00) Serial.print(F("  --  "));
			
		Serial.print(F(", F="));
		Serial.print(ifreq);
		Serial.print(F(", SF="));
		Serial.print(sf);
		Serial.print(F(", E="));
		Serial.print(_event);
			
		Serial.print(F(", S="));
		//Serial.print(_state);
		switch (_state) {
			case S_INIT:
				Serial.print(F("INIT "));
			break;
			case S_SCAN:
				Serial.print(F("SCAN "));
			break;
			case S_CAD:
				Serial.print(F("CAD  "));
			break;
			case S_RX:
				Serial.print(F("RX   "));
			break;
			case S_TX:
				Serial.print(F("TX   "));
			break;
			case S_TXDONE:
				Serial.print(F("TXDONE"));
			break;
			default:
				Serial.print(F(" -- "));
		}
		Serial.print(F(", eT="));
		Serial.print( micros() - eventTime );
		Serial.print(F(", dT="));
		Serial.print( micros() - doneTime );
		Serial.println();
	}
#endif
}


	
// ----------------------------------------------------------------------------
// SerialName(id, response)
// Check whether for address a (4 bytes in Unsigned Long) there is a name
// This function only works if _TRUSTED_NODES is set
// ----------------------------------------------------------------------------

int SerialName(char * a, String& response)
{
#if _TRUSTED_NODES>=1
	uint32_t id = ((a[0]<<24) | (a[1]<<16) | (a[2]<<8) | a[3]);

	int i;
	for ( i=0; i< (sizeof(nodes)/sizeof(nodex)); i++) {

		if (id == nodes[i].id) {
#if _DUSB >=1
			if (( debug>=3 ) && ( pdebug & P_GUI )) {
				Serial.print(F("G Name="));
				Serial.print(nodes[i].nm);
				Serial.print(F(" for node=0x"));
				Serial.print(nodes[i].id,HEX);
				Serial.println();
			}
#endif // _DUSB
			response += nodes[i].nm;
			return(i);
		}
	}

#endif // _TRUSTED_NODES
	return(-1);									// If no success OR is TRUSTED NODES not defined
}

#if _LOCALSERVER==1
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

	int i;
	for ( i=0; i< (sizeof(decodes)/sizeof(codex)); i++) {
		if (ident == decodes[i].id) {
			return(i);
		}
	}
	return(-1);
}
#endif
