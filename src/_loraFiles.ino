// 1-channel LoRa Gateway for ESP8266
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
// This file contains the LoRa filesystem specific code


#if _MONITOR>=1
// ----------------------------------------------------------------------------
// LoRa Monitor logging code.
// Define one print function and depending on the logging parameter output
// to _USB of to the www screen function
// ----------------------------------------------------------------------------
int initMonitor(struct moniLine *monitor) 
{
	for (int i=0; i< _MAXMONITOR; i++) {
		monitor[i].txt= "-";						// Make all lines empty
	}
	iMoni=0;										// Init the index
	return(1);
}

#endif //_MONITOR


// ============================================================================
// LORA SPIFFS FILESYSTEM FUNCTIONS
//
// The LoRa supporting functions are in the section below

// ----------------------------------------------------------------------------
// Supporting function to readConfig
// ----------------------------------------------------------------------------
void id_print (String id, String val) 
{
#if _MONITOR>=1
	if (( debug>=0 ) && ( pdebug & P_MAIN )) {
		Serial.print(id);
		Serial.print(F("=\t"));
		Serial.println(val);
	}
#endif //_MONITOR
}




// ----------------------------------------------------------------------------
// INITCONFIG; Init the gateway configuration file
// Espcecially when calling SPIFFS.format() the gateway is left in an init state
// which is not very well defined. This function will init some of the settings
// to well known settings.
// ----------------------------------------------------------------------------
void initConfig(struct espGwayConfig *c)
{
	(*c).ch = 0;
	(*c).sf = _SPREADING;
	(*c).debug = 1;						// debug level is 1
	(*c).pdebug = P_GUI | P_MAIN;
	(*c).cad = _CAD;
	(*c).hop = false;
	(*c).seen = true;					// Seen interface is ON
	(*c).expert = false;				// Expert interface is OFF
	(*c).monitor = true;				// Monitoring is ON
	(*c).trusted = 1;
	(*c).txDelay = 0;					// First Value without saving is 0;
}

// ----------------------------------------------------------------------------
// Read the config file and fill the (copied) variables
// ----------------------------------------------------------------------------
int readGwayCfg(const char *fn, struct espGwayConfig *c)
{

	if (readConfig(fn, c)<0) {
#		if _MONITOR>=1
			mPrint("readConfig:: Error reading config file");
			return 0;
#		endif //_MONITOR
	}

	if (gwayConfig.sf != (uint8_t) 0) { 
		sf = (sf_t) gwayConfig.sf; 
	}
	debug	= (*c).debug;
	pdebug	= (*c).pdebug;

#	if _GATEWAYNODE==1
		if (gwayConfig.fcnt != (uint8_t) 0) {
			frameCount = gwayConfig.fcnt+10;
		}
#	endif

	return 1;
}


// ----------------------------------------------------------------------------
// Read the gateway configuration file
// ----------------------------------------------------------------------------
int readConfig(const char *fn, struct espGwayConfig *c)
{
	
	int tries = 0;

	if (!SPIFFS.exists(fn)) {	
#		if _MONITOR>=1
			mPrint("readConfig ERR:: file="+String(fn)+" does not exist ..");
#		endif //_MONITOR
		initConfig(c);					// If we cannot read the config, at least init known values
		return(-1);
	}

	File f = SPIFFS.open(fn, "r");
	if (!f) {
#		if _MONITOR>=1
			Serial.println(F("ERROR:: SPIFFS open failed"));
#		endif //_MONITOR
		return(-1);
	}

	while (f.available()) {
		
#		if _MONITOR>=1
		if (( debug>=0 ) && ( pdebug & P_MAIN )) {
			Serial.print('.');
		}
#		endif //_MONITOR

		// If we wait for more than 15 times, reformat the filesystem
		// We do this so that the system will be responsive (over OTA for example).
		//
		if (tries >= 15) {
			f.close();
#			if _MONITOR>=1
			if (debug>=0) {
				mPrint("readConfig:: Formatting");
			}
#			endif //_MONITOR
			SPIFFS.format();
			f = SPIFFS.open(fn, "r");
			tries = 0;
			initSeen(listSeen);
		}
		initConfig(c);											// Even if we do not read a value, give a default
		
		String id =f.readStringUntil('=');						// Read keyword until '=', C++ thing
		String val=f.readStringUntil('\n');						// Read value until End of Line (EOL)

		if (id == "MONITOR") {									// MONITOR button setting
			id_print(id, val);
			(*c).monitor = (bool) val.toInt();
		}
		else if (id == "CH") { 									// Frequency Channel
			id_print(id,val); 
			(*c).ch = (uint8_t) val.toInt();
		}
		else if (id == "SF") { 									// Spreading Factor
			id_print(id, val);
			(*c).sf = (uint8_t) val.toInt();
		}
		else if (id == "FCNT") {								// Frame Counter
			id_print(id, val);
			(*c).fcnt = (uint16_t) val.toInt();
		}
		else if (id == "DEBUG") {								// Debug Level
			id_print(id, val);
			(*c).debug = (uint8_t) val.toInt();
		}
		else if (id == "PDEBUG") {								// pDebug Pattern
			id_print(id, val);
			(*c).pdebug = (uint8_t) val.toInt();
		}
		else if (id == "CAD") {									// CAD setting
			id_print(id, val);
			(*c).cad = (bool) val.toInt();
		}
		else if (id == "HOP") {									// HOP setting
			id_print(id, val);
			(*c).hop = (bool) val.toInt();
		}
		else if (id == "BOOTS") {								// BOOTS setting
			id_print(id, val);
			(*c).boots = (uint16_t) val.toInt();
		}
		else if (id == "RESETS") {								// RESET setting
			id_print(id, val);
			(*c).resets = (uint16_t) val.toInt();
		}
		else if (id == "WIFIS") {								// WIFIS setting
			id_print(id, val);
			(*c).wifis = (uint16_t) val.toInt();
		}
		else if (id == "VIEWS") {								// VIEWS setting
			id_print(id, val);
			(*c).views = (uint16_t) val.toInt();
		}
		else if (id == "NODE") {								// NODE setting
			id_print(id, val);
			(*c).isNode = (bool) val.toInt();
		}
		else if (id == "REFR") {								// REFR setting
			id_print(id, val);
			(*c).refresh = (bool) val.toInt();
		}
		else if (id == "REENTS") {								// REENTS setting
			id_print(id, val);
			(*c).reents = (uint16_t) val.toInt();
		}
		else if (id == "NTPERR") {								// NTPERR setting
			id_print(id, val);
			(*c).ntpErr = (uint16_t) val.toInt();
		}
		else if (id == "NTPETIM") {								// NTPERR setting
			id_print(id, val);
			(*c).ntpErrTime = (uint32_t) val.toInt();
		}
		else if (id == "NTPS") {								// NTPS setting
			id_print(id, val);
			(*c).ntps = (uint16_t) val.toInt();
		}
		else if (id == "FILENO") {								// FILENO setting
			id_print(id, val);
			(*c).logFileNo = (uint16_t) val.toInt();
		}
		else if (id == "FILEREC") {								// FILEREC setting
			id_print(id, val);
			(*c).logFileRec = (uint16_t) val.toInt();
		}
		else if (id == "FILENUM") {								// FILEREC setting
			id_print(id, val);
			(*c).logFileNum = (uint16_t) val.toInt();
		}
		else if (id == "EXPERT") {								// EXPERT button setting
			id_print(id, val);
			(*c).expert = (bool) val.toInt();
		}
		else if (id == "SEEN") {								// SEEN button setting
			id_print(id, val);
			(*c).seen = (bool) val.toInt();
		}
		else if (id == "DELAY") {								// DELAY setting
			id_print(id, val);
			(*c).txDelay = (int32_t) val.toInt();
		}
		else if (id == "TRUSTED") {								// TRUSTED setting
			id_print(id, val);
			(*c).trusted= (int8_t) val.toInt();
		}
		else {
#			if _MONITOR>=1
				mPrint(F("readConfig:: tries++"));
#			endif //_MONITOR
			tries++;
		}
	}
	f.close();

	return(1);
}//readConfig


// ----------------------------------------------------------------------------
// Write the current gateway configuration to SPIFFS. First copy all the
// separate data items to the gwayConfig structure
//
// 	Note: gwayConfig.expert contains the expert setting already
//				gwayConfig.txDelay
// ----------------------------------------------------------------------------
int writeGwayCfg(const char *fn, struct espGwayConfig *c)
{
	

	(*c).sf = (uint8_t) sf;						// Spreading Factor
	(*c).debug = debug;
	(*c).pdebug = pdebug;

#	if _GATEWAYNODE==1
		(*c).fcnt = frameCount;
#	endif //_GATEWAYNODE

	return(writeConfig(fn, c));
}

// ----------------------------------------------------------------------------
// Write the configuration as found in the espGwayConfig structure
// to SPIFFS
// Parameters:
//		fn; Filename
//		c; struct config
// Returns:
//		1 when successful, -1 on error
// ----------------------------------------------------------------------------
int writeConfig(const char *fn, struct espGwayConfig *c)
{
	// Assuming the config file is the first we write...
	
	File f = SPIFFS.open(fn, "w");
	if (!f) {
#if _MONITOR>=1	
		mPrint("writeConfig: ERROR open file="+String(fn));
#endif //_MONITOR
		return(-1);
	}

	f.print("CH");		f.print('='); f.print((*c).ch);			f.print('\n');
	f.print("SF");		f.print('='); f.print((*c).sf);			f.print('\n');
	f.print("FCNT");	f.print('='); f.print((*c).fcnt);		f.print('\n');
	f.print("DEBUG");	f.print('='); f.print((*c).debug);		f.print('\n');
	f.print("PDEBUG");	f.print('='); f.print((*c).pdebug);		f.print('\n');
	f.print("CAD");		f.print('='); f.print((*c).cad);		f.print('\n');
	f.print("HOP");		f.print('='); f.print((*c).hop);		f.print('\n');
	f.print("NODE");	f.print('='); f.print((*c).isNode);		f.print('\n');
	f.print("BOOTS");	f.print('='); f.print((*c).boots);		f.print('\n');
	f.print("RESETS");	f.print('='); f.print((*c).resets);		f.print('\n');
	f.print("WIFIS");	f.print('='); f.print((*c).wifis);		f.print('\n');
	f.print("VIEWS");	f.print('='); f.print((*c).views);		f.print('\n');
	f.print("REFR");	f.print('='); f.print((*c).refresh);	f.print('\n');
	f.print("REENTS");	f.print('='); f.print((*c).reents);		f.print('\n');
	f.print("NTPETIM");	f.print('='); f.print((*c).ntpErrTime); f.print('\n');
	f.print("NTPERR");	f.print('='); f.print((*c).ntpErr);		f.print('\n');
	f.print("NTPS");	f.print('='); f.print((*c).ntps);		f.print('\n');
	f.print("FILEREC");	f.print('='); f.print((*c).logFileRec); f.print('\n');
	f.print("FILENO");	f.print('='); f.print((*c).logFileNo);	f.print('\n');
	f.print("FILENUM");	f.print('='); f.print((*c).logFileNum); f.print('\n');
	f.print("DELAY");	f.print('='); f.print((*c).txDelay); 	f.print('\n');
	f.print("TRUSTED");	f.print('='); f.print((*c).trusted); 	f.print('\n');
	f.print("EXPERT");	f.print('='); f.print((*c).expert); 	f.print('\n');
	f.print("SEEN");	f.print('='); f.print((*c).seen);		f.print('\n');
	f.print("MONITOR");	f.print('='); f.print((*c).monitor); 	f.print('\n');
	
	f.close();
	return(1);
}



// ----------------------------------------------------------------------------
// Add a line with statistics to the log.
//
// We put the check in the function to protect against calling 
// the function without _STAT_LOG being proper defined
// ToDo: Store the fileNo and the fileRec in the status file to save for 
// restarts
//
// Parameters:
//		line; char array with characters to write to log
//		cnt;
// Returns:
//		<none>
// ----------------------------------------------------------------------------
int addLog(const unsigned char * line, int cnt) 
{
#	if _STAT_LOG==1
	char fn[16];
	
	if (gwayConfig.logFileRec > LOGFILEREC) {		// Have to make define for this
		gwayConfig.logFileRec = 0;					// In new logFile start with record 0
		gwayConfig.logFileNo++;						// Increase file ID
		gwayConfig.logFileNum++;					// Increase number of log files
	}
	gwayConfig.logFileRec++;
	
	// If we have too many logfies, delete the oldest
	//
	if (gwayConfig.logFileNum > LOGFILEMAX){
		sprintf(fn,"/log-%d", gwayConfig.logFileNo - LOGFILEMAX);
#		if _MONITOR>=1
		if (( debug>=2 ) && ( pdebug & P_GUI )) {
			mPrint("addLog:: Too many logfiles, deleting="+String(fn));
		}
#		endif //_MONITOR
		SPIFFS.remove(fn);
		gwayConfig.logFileNum--;
	}
	
	// Make sure we have the right fileno
	sprintf(fn,"/log-%d", gwayConfig.logFileNo); 
	
	// If there is no SPIFFS, Error
	// Make sure to write the config record/line also
	if (!SPIFFS.exists(fn)) {
#		if _MONITOR>=1
		if (( debug >= 2 ) && ( pdebug & P_GUI )) {
			mPrint("addLog:: WARNING file="+String(fn)+" does not exist .. rec="+String(gwayConfig.logFileRec) );
		}
#		endif //_MONITOR
	}
	
	File f = SPIFFS.open(fn, "a");
	if (!f) {
#		if _MONITOR>=1
		if (( debug>=1 ) && ( pdebug & P_GUI )) {
			mPrint("addLOG:: ERROR file open failed="+String(fn));
		}
#		endif //_MONITOR
		return(0);								// If file open failed, return
	}
	
	int i=0;
#	if _MONITOR>=1
	if (( debug>=2 ) && ( pdebug & P_GUI )) {
		Serial.print(F("addLog:: fileno="));
		Serial.print(gwayConfig.logFileNo);
		Serial.print(F(", rec="));
		Serial.print(gwayConfig.logFileRec);

		Serial.print(F(": "));
#		if _DUSB>=2
			for (i=0; i< 12; i++) {				// The first 12 bytes contain non printable characters
				Serial.print(line[i],HEX);
				Serial.print(' ');
			}
#		else //_DUSB>=2
			i+=12;
#		endif //_DUSB>=2
		Serial.print((char *) &line[i]);	// The rest if the buffer contains ascii
		Serial.println();
	}
#	endif //_MONITOR

	for (i=0; i< 12; i++) {					// The first 12 bytes contain non printable characters
	//	f.print(line[i],HEX);
		f.print('*');
	}
	f.write(&(line[i]), cnt-12);			// write/append the line to the file
	f.print('\n');
	f.close();								// Close the file after appending to it

#	endif //_STAT_LOG

	return(1);
} //addLog


// ----------------------------------------------------------------------------
// Print (all) logfiles
// Return:
//	<none>
// Parameters:
//	<none>
// ----------------------------------------------------------------------------
void printLog()
{
#if _STAT_LOG==1
	char fn[16];

#	if _DUSB>=1
	for (int i=0; i< LOGFILEMAX; i++ ) {
		sprintf(fn,"/log-%d", gwayConfig.logFileNo - i);
		if (!SPIFFS.exists(fn)) break;		// break the loop

		// Open the file for reading
		File f = SPIFFS.open(fn, "r");

		for (int j=0; j<LOGFILEREC; j++) {
			
			String s=f.readStringUntil('\n');
			if (s.length() == 0) break;

			Serial.println(s.substring(12));			// Skip the first 12 Gateway specific binary characters
			yield();
		}
	}
#	endif //_DUSB
#endif //_STAT_LOG
} //printLog


#if _MAXSEEN >= 1

// ----------------------------------------------------------------------------
// initSeen
// Init the lisrScreen array
// Return:
//	1: Success
// Parameters:
//	listSeen: array of Seen data
// ----------------------------------------------------------------------------
int initSeen(struct nodeSeen *listSeen) 
{
	for (int i=0; i< _MAXSEEN; i++) {
		listSeen[i].idSeen=0;
		listSeen[i].sfSeen=0;
		listSeen[i].cntSeen=0;
		listSeen[i].chnSeen=0;
		listSeen[i].timSeen=(time_t) 0;					// 1 jan 1970 0:00:00 hrs
	}
	iSeen= 0;											// Init index to 0
	return(1);
}


// ----------------------------------------------------------------------------
// readSeen
// This function read the information stored by writeSeen from the file.
// The file is read as String() values and converted to int after.
// Parameters:
//	fn:			Filename
//	listSeen:	Array of all last seen nodes on the LoRa network
// Return:
//	1:			When successful
// ----------------------------------------------------------------------------
int readSeen(const char *fn, struct nodeSeen *listSeen)
{
	int i;
	iSeen= 0;												// Init the index at 0
	
	if (!SPIFFS.exists(fn)) {								// Does listSeen file exist
#		if _MONITOR>=1
			mPrint("WARNING:: readSeen, history file not exists "+String(fn) );
#		endif //_MONITOR
		initSeen(listSeen);		// XXX make all initial declarations here if config vars need to have a value
		return(-1);
	}
	
	File f = SPIFFS.open(fn, "r");
	if (!f) {
#		if _MONITOR>=1
			mPrint("readSeen:: ERROR open file=" + String(fn));
#		endif //_MONITOR
		return(-1);
	}

	delay(1000);
	
	for (i=0; i<_MAXSEEN; i++) {
		delay(200);
		String val="";
		
		if (!f.available()) {
#			if _MONITOR>=1
				String response="readSeen:: No more info left in file, i=";
				response += i;
				mPrint(response);
#			endif //_MONITOR
			break;
		}
		val=f.readStringUntil('\t'); listSeen[i].timSeen = (time_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].idSeen = (int64_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].cntSeen = (uint32_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].chnSeen = (uint8_t) val.toInt();
		val=f.readStringUntil('\n'); listSeen[i].sfSeen = (uint8_t) val.toInt();
		
#		if _MONITOR>=1
		if ((debug>=2) && (pdebug & P_MAIN)) {
				mPrint("readSeen:: idSeen ="+String(listSeen[i].idSeen,HEX)+", i="+String(i));
		}
#		endif
		iSeen++;											// Increase index, new record read
	}
	f.close();
	
#	if _MONITOR>=1
	if ((debug >= 2) && (pdebug & P_MAIN)) {
		Serial.print("readSeen:: ");
		printSeen(listSeen);
	}
#	endif //_MONITOR
	// So we read iSeen records
	return 1;
}


// ----------------------------------------------------------------------------
// writeSeen
// Once every few messages, update the SPIFFS file and write the array.
// Parameters:
// - fn contains the filename to write
// - listSeen contains the _MAXSEEN array of list structures 
// Return values:
// - return 1 on success
// ----------------------------------------------------------------------------
int writeSeen(const char *fn, struct nodeSeen *listSeen)
{
	int i;
	if (!SPIFFS.exists(fn)) {
#		if _MONITOR>=1
			mPrint("WARNING:: writeSeen, file not exists="+String(fn));
#		endif //_MONITOR
		//initSeen(listSeen);		// XXX make all initial declarations here if config vars need to have a value
	}
	
	File f = SPIFFS.open(fn, "w");
	if (!f) {
#		if _MONITOR>=1
			mPrint("writeSeen:: ERROR open file="+String(fn)+" for writing");
#		endif //_MONITOR
		return(-1);
	}
	delay(500);

	for (i=0; i<iSeen; i++) {							// For all records indexed
			f.print((time_t)listSeen[i].timSeen);	f.print('\t');
			f.print((int32_t)listSeen[i].idSeen);	f.print('\t'); // Typecast to avoid errors in unsigned conversion!
			f.print((uint32_t)listSeen[i].cntSeen);	f.print('\t');
			f.print((uint8_t)listSeen[i].chnSeen);	f.print('\t');
			f.print((uint8_t)listSeen[i].sfSeen);	f.print('\n');			
	}
	
	f.close();
#	if _DUSB>=1
	if ((debug >= 2) && (pdebug & P_MAIN)) {
		Serial.print("writeSeen:: ");
		printSeen(listSeen);
	}
#	endif //_DUSB
	return(1);
}

// ----------------------------------------------------------------------------
// printSeen
// - This function writes the last seen array to the USB !!
// ----------------------------------------------------------------------------
int printSeen(struct nodeSeen *listSeen) {

#	if _DUSB>=1
    int i;
	if (( debug>=2 ) && ( pdebug & P_MAIN )) {
	
		Serial.println(F("printSeen:: "));
		
		for (i=0; i<iSeen; i++) {
			if (listSeen[i].idSeen != 0) {
				String response;
				
				response = i;
				response += ", Tim=";

				stringTime(listSeen[i].timSeen, response);

				response += ", addr=0x";
				printHex(listSeen[i].idSeen,' ', response);
				
				response += ", SF=0x";
				response += listSeen[i].sfSeen;
				Serial.println(response);
			}
		}
	}
#	endif //_DUSB
	return(1);
}

// ----------------------------------------------------------------------------
// addSeen
//	With every message received:
// - Look whether message is already in the array, if so update existing message. 
// - If not, create new record.
// - With this record, update the SF settings
//
// Parameters:
//	listSeen: The array of records of nodes we have seen
//	stat: one record
// Returns:
//	1 when successful
// ----------------------------------------------------------------------------
int addSeen(struct nodeSeen *listSeen, struct stat_t stat) 
{

	int i=0;
	for (i=0; i<iSeen; i++) {						// For all known records

		if (listSeen[i].idSeen==stat.node) {

			listSeen[i].timSeen = (time_t)stat.tmst;
			listSeen[i].cntSeen++;					// Not included on function para
			listSeen[i].idSeen = stat.node;
			listSeen[i].chnSeen = stat.ch;
			listSeen[i].sfSeen = stat.sf;			// Or the argument
	
			writeSeen(_SEENFILE, listSeen);

			return 1;
		}
	}
	
	// Else: We did not find the current record so make a new Seen entry
	if ((i>=iSeen) && (i<_MAXSEEN)) {
		listSeen[i].idSeen = stat.node;
		listSeen[i].chnSeen = stat.ch;
		listSeen[i].sfSeen = stat.sf;			// Or the argument
		listSeen[i].timSeen = (time_t)stat.tmst;
		listSeen[i].cntSeen = 1;					// Not included on function para	
		iSeen++;
		//return(1);
	}

#	if _MONITOR>=2
	if ((debug>=1) && (pdebug & P_MAIN)) {
		String response= "addSeen:: New i=";
		response += i;
		response += ", tim=";
		stringTime(stat.tmst, response);
		response += ", iSeen=";
		response += String(iSeen,HEX);
		response += ", node=";
		response += String(stat.node,HEX);
		response += ", listSeen[0]=";
		
		printHex(listSeen[0].idSeen,' ',response);
		Serial.print("<"); Serial.print(listSeen[0].idSeen, HEX); Serial.print(">");

		mPrint(response);
		
		response += ", listSeen[0]=";
		printHex(listSeen[0].idSeen,' ',response);

		mPrint(response);
	}
#	endif

	// USB Only
#	if _DUSB>=2
	if ((debug>=2) && (pdebug & P_MAIN)) {
		Serial.print("addSeen i=");
		Serial.print(i);
		Serial.print(", id=");
		Serial.print(listSeen[i].idSeen,HEX);
		Serial.println();
	}
#	endif // _DUSB	

	return 1;
}

#endif //_MAXSEEN>=1 End of File
