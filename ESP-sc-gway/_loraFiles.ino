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
// This file contains the LoRa filesystem specific code


#if _MONITOR>=1
// ----------------------------------------------------------------------------
// LoRa Monitor logging code.
// Define one print function and depending on the loggin parameter output
// to _USB of to the www screen function
// ----------------------------------------------------------------------------
int initMonitor(struct moniLine *monitor) {
	for (int i=0; i< _MONITOR; i++) {
		//monitor[i].txt[0]=0;
		monitor[i].txt="";
	}
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
void id_print (String id, String val) {
#if _DUSB>=1
	if (( debug>=0 ) && ( pdebug & P_MAIN )) {
		Serial.print(id);
		Serial.print(F("=\t"));
		Serial.println(val);
	}
#endif //_DUSB
}




// ----------------------------------------------------------------------------
// INITCONFIG; Init the gateway configuration file
// Espcecially when calling SPIFFS.format() the gateway is left in an init
// which is not very well defined. This function will init some of the settings
// to well known settings.
// ----------------------------------------------------------------------------
int initConfig(struct espGwayConfig *c) {
	(*c).ch = 0;
	(*c).sf = _SPREADING;
	(*c).debug = 1;
	(*c).pdebug = P_GUI;
	(*c).cad = _CAD;
	(*c).hop = false;
	(*c).seen = false;
	(*c).expert = false;
	(*c).monitor = false;
	(*c).txDelay = 0;					// First Value without saving is 0;
	(*c).trusted = 1;
}


// ----------------------------------------------------------------------------
// Read the gateway configuration file
// ----------------------------------------------------------------------------
int readConfig(const char *fn, struct espGwayConfig *c) {
	
	int tries = 0;
#if _DUSB>=1
	Serial.println(F("readConfig:: Starting "));
#endif //_DUSB

	if (!SPIFFS.exists(fn)) {
#if _DUSB>=1
		Serial.print(F("readConfig ERR:: file="));
		Serial.print(fn);
		Serial.println(F(" does not exist .. Formatting"));
#endif //_DUSB
		SPIFFS.format();
		initConfig(c);					// If we cannot read teh config, at least init known values
		return(-1);
	}

	File f = SPIFFS.open(fn, "r");
	if (!f) {
#if _DUSB>=1
		Serial.println(F("ERROR:: SPIFFS open failed"));
#endif //_DUSB
		return(-1);
	}

	while (f.available()) {
		
#if _DUSB>=1
		if (( debug>=0 ) && ( pdebug & P_MAIN )) {
			Serial.print('.');
		}
#endif //_DUSB
		// If we wait for more than 10 times, reformat the filesystem
		// We do this so that the system will be responsive (over OTA for example).
		//
		if (tries >= 10) {
			f.close();
#if _DUSB>=1
			if (( debug>=0 ) && ( pdebug & P_MAIN )) {
				Serial.println(F("Formatting"));
			}
#endif //_DUSB
			SPIFFS.format();
			initConfig(c);
			f = SPIFFS.open(fn, "r");
			tries = 0;
		}

		String id =f.readStringUntil('=');						// Read keyword until '=', C++ thing
		String val=f.readStringUntil('\n');						// Read value until End of Line (EOL)

#if _DUSB>=2
		Serial.print(F("readConfig:: reading line="));
		Serial.print(id);
		Serial.print(F("="));
		Serial.print(val);
		Serial.println();
#endif //_DUSB

		if (id == "SSID") {										// WiFi SSID
			id_print(id, val);
			(*c).ssid = val;									// val contains ssid, we do NO check
		}
		else if (id == "PASS") { 								// WiFi Password
			id_print(id, val); 
			(*c).pass = val;
		}
		else if (id == "CH") { 									// Frequency Channel
			id_print(id,val); 
			(*c).ch = (uint32_t) val.toInt();
		}
		else if (id == "SF") { 									// Spreading Factor
			id_print(id, val);
			(*c).sf = (uint32_t) val.toInt();
		}
		else if (id == "FCNT") {								// Frame Counter
			id_print(id, val);
			(*c).fcnt = (uint32_t) val.toInt();
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
			(*c).cad = (uint8_t) val.toInt();
		}
		else if (id == "HOP") {									// HOP setting
			id_print(id, val);
			(*c).hop = (uint8_t) val.toInt();
		}
		else if (id == "BOOTS") {								// BOOTS setting
			id_print(id, val);
			(*c).boots = (uint8_t) val.toInt();
		}
		else if (id == "RESETS") {								// RESET setting
			id_print(id, val);
			(*c).resets = (uint8_t) val.toInt();
		}
		else if (id == "WIFIS") {								// WIFIS setting
			id_print(id, val);
			(*c).wifis = (uint8_t) val.toInt();
		}
		else if (id == "VIEWS") {								// VIEWS setting
			id_print(id, val);
			(*c).views = (uint8_t) val.toInt();
		}
		else if (id == "NODE") {								// NODE setting
			id_print(id, val);
			(*c).isNode = (uint8_t) val.toInt();
		}
		else if (id == "REFR") {								// REFR setting
			id_print(id, val);
			(*c).refresh = (uint8_t) val.toInt();
		}
		else if (id == "REENTS") {								// REENTS setting
			id_print(id, val);
			(*c).reents = (uint8_t) val.toInt();
		}
		else if (id == "NTPERR") {								// NTPERR setting
			id_print(id, val);
			(*c).ntpErr = (uint8_t) val.toInt();
		}
		else if (id == "NTPETIM") {								// NTPERR setting
			id_print(id, val);
			(*c).ntpErrTime = (uint32_t) val.toInt();
		}
		else if (id == "NTPS") {								// NTPS setting
			id_print(id, val);
			(*c).ntps = (uint8_t) val.toInt();
		}
		else if (id == "FILENO") {								// FILENO setting
			id_print(id, val);
			(*c).logFileNo = (uint8_t) val.toInt();
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
			(*c).expert = (uint8_t) val.toInt();
		}
		else if (id == "SEEN") {								// SEEN button setting
			id_print(id, val);
			(*c).seen = (uint8_t) val.toInt();
		}
		else if (id == "MONITOR") {								// MONITOR button setting
			id_print(id, val);
			(*c).monitor = (uint8_t) val.toInt();
		}
		else if (id == "DELAY") {								// DELAY setting
			id_print(id, val);
			(*c).txDelay = (int32_t) val.toInt();
		}
		else if (id == "TRUSTED") {								// TRUSTED setting
			id_print(id, val);
			(*c).trusted= (int32_t) val.toInt();
		}
		else {
#if _DUSB>=1
			Serial.print(F("readConfig:: tries++"));
#endif //_DUSB
			tries++;
		}
	}
	f.close();
	
#if _DUSB>=1
	if (debug>=1) {
		Serial.println(F("readConfig:: Fini"));
	}
	Serial.println();
#endif //_DUSB

	return(1);
}//readConfig


// ----------------------------------------------------------------------------
// Write the current gateway configuration to SPIFFS. First copy all the
// separate data items to the gwayConfig structure
//
// 	Note: gwayConfig.expert contains the expert setting already
//				gwayConfig.txDelay
// ----------------------------------------------------------------------------
int writeGwayCfg(const char *fn) {

	gwayConfig.ssid = WiFi.SSID();
	gwayConfig.pass = WiFi.psk();						// XXX We should find a way to store the password too
	gwayConfig.ch = ifreq;								// Frequency Index
	gwayConfig.sf = (uint8_t) sf;						// Spreading Factor
	gwayConfig.debug = debug;
	gwayConfig.pdebug = pdebug;
	gwayConfig.cad = _cad;
	gwayConfig.hop = _hop;

#if GATEWAYNODE==1
	gwayConfig.fcnt = frameCount;
#endif //GATEWAYNODE
	return(writeConfig(fn, &gwayConfig));
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
int writeConfig(const char *fn, struct espGwayConfig *c) {

	// Assuming the cibfug file is the first we write...
	// If it is not there we should format first.
	// NOTE: Do not format for other files!
	if (!SPIFFS.exists(fn)) {
#if _DUSB>=1
		Serial.print("WARNING:: writeConfig, file not exists, formatting ");
#endif //_DUSB
		SPIFFS.format();
		initConfig(c);		// XXX make all initial declarations here if config vars need to have a value
#if _DUSB>=1		
		Serial.println(fn);
#endif //_DUSB
	}
	File f = SPIFFS.open(fn, "w");
	if (!f) {
#if _DUSB>=1	
		Serial.print("writeConfig: ERROR open file=");
		Serial.print(fn);
		Serial.println();
#endif //_DUSB
		return(-1);
	}

	f.print("SSID"); f.print('='); f.print((*c).ssid); f.print('\n'); 
	f.print("PASS"); f.print('='); f.print((*c).pass); f.print('\n');
	f.print("CH"); f.print('='); f.print((*c).ch); f.print('\n');
	f.print("SF");   f.print('='); f.print((*c).sf);   f.print('\n');
	f.print("FCNT"); f.print('='); f.print((*c).fcnt); f.print('\n');
	f.print("DEBUG"); f.print('='); f.print((*c).debug); f.print('\n');
	f.print("PDEBUG"); f.print('='); f.print((*c).pdebug); f.print('\n');
	f.print("CAD");  f.print('='); f.print((*c).cad); f.print('\n');
	f.print("HOP");  f.print('='); f.print((*c).hop); f.print('\n');
	f.print("NODE");  f.print('='); f.print((*c).isNode); f.print('\n');
	f.print("BOOTS");  f.print('='); f.print((*c).boots); f.print('\n');
	f.print("RESETS");  f.print('='); f.print((*c).resets); f.print('\n');
	f.print("WIFIS");  f.print('='); f.print((*c).wifis); f.print('\n');
	f.print("VIEWS");  f.print('='); f.print((*c).views); f.print('\n');
	f.print("REFR");  f.print('='); f.print((*c).refresh); f.print('\n');
	f.print("REENTS");  f.print('='); f.print((*c).reents); f.print('\n');
	f.print("NTPETIM");  f.print('='); f.print((*c).ntpErrTime); f.print('\n');
	f.print("NTPERR");  f.print('='); f.print((*c).ntpErr); f.print('\n');
	f.print("NTPS");  f.print('='); f.print((*c).ntps); f.print('\n');
	f.print("FILEREC");  f.print('='); f.print((*c).logFileRec); f.print('\n');
	f.print("FILENO");  f.print('='); f.print((*c).logFileNo); f.print('\n');
	f.print("FILENUM");  f.print('='); f.print((*c).logFileNum); f.print('\n');
	f.print("DELAY");  f.print('='); f.print((*c).txDelay); f.print('\n');
	f.print("TRUSTED");  f.print('='); f.print((*c).trusted); f.print('\n');
	f.print("EXPERT");  f.print('='); f.print((*c).expert); f.print('\n');
	f.print("SEEN");  f.print('='); f.print((*c).seen); f.print('\n');
	f.print("MONITOR");  f.print('='); f.print((*c).monitor); f.print('\n');
	
	f.close();
	return(1);
}

// ----------------------------------------------------------------------------
// Add a line with statistics to the log.
//
// We put the check in the function to protect against calling 
// the function without STAT_LOG being proper defined
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
#if STAT_LOG==1
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
#if _DUSB>=1
		if (( debug>=0 ) && ( pdebug & P_GUI )) {
			Serial.print(F("G addLog:: Too many logfile, deleting="));
			Serial.println(fn);
		}
#endif //_DUSB
		SPIFFS.remove(fn);
		gwayConfig.logFileNum--;
	}
	
	// Make sure we have the right fileno
	sprintf(fn,"/log-%d", gwayConfig.logFileNo); 
	
	// If there is no SPIFFS, Error
	// Make sure to write the config record/line also
	if (!SPIFFS.exists(fn)) {
#if _DUSB>=1
		if (( debug >= 1 ) && ( pdebug & P_GUI )) {
			Serial.print(F("G ERROR:: addLog:: file="));
			Serial.print(fn);
			Serial.print(F(" does not exist .. rec="));
			Serial.print(gwayConfig.logFileRec);
			Serial.println();
		}
#endif //_DUSB
	}
	
	File f = SPIFFS.open(fn, "a");
	if (!f) {
#if _DUSB>=1
		if (( debug>=1 ) && ( pdebug & P_GUI )) {
			Serial.println("G file open failed=");
			Serial.println(fn);
		}
#endif //_DUSB
		return(0);								// If file open failed, return
	}
	
	int i;
#if _DUSB>=1
	if (( debug>=1 ) && ( pdebug & P_GUI )) {
		Serial.print(F("G addLog:: fileno="));
		Serial.print(gwayConfig.logFileNo);
		Serial.print(F(", rec="));
		Serial.print(gwayConfig.logFileRec);

		Serial.print(F(": "));
#if _DUSB>=2
		for (i=0; i< 12; i++) {				// The first 12 bytes contain non printable characters
			Serial.print(line[i],HEX);
			Serial.print(' ');
		}
#else
		i+=12;
#endif //_DUSB>=2
		Serial.print((char *) &line[i]);	// The rest if the buffer contains ascii

		Serial.println();
	}
#endif //_DUSB


	for (i=0; i< 12; i++) {					// The first 12 bytes contain non printable characters
	//	f.print(line[i],HEX);
		f.print('*');
	}
	f.write(&(line[i]), cnt-12);			// write/append the line to the file
	f.print('\n');
	f.close();								// Close the file after appending to it

#endif //STAT_LOG
	return(1);
}

// ----------------------------------------------------------------------------
// Print (all) logfiles
//
// ----------------------------------------------------------------------------
void printLog()
{
	char fn[16];
	int i=0;
#if _DUSB>=1
	while (i< LOGFILEMAX ) {
		sprintf(fn,"/log-%d", gwayConfig.logFileNo - i);
		if (!SPIFFS.exists(fn)) break;		// break the loop

		// Open the file for reading
		File f = SPIFFS.open(fn, "r");
		
		int j;
		for (j=0; j<LOGFILEREC; j++) {
			
			String s=f.readStringUntil('\n');
			if (s.length() == 0) break;

			Serial.println(s.substring(12));			// Skip the first 12 Gateway specific binary characters
			yield();
		}
		i++;
	}
#endif //_DUSB
} //printLog


#if _SEENMAX >= 1


// ----------------------------------------------------------------------------
// initSeen
// Init the lisrScreen array
// ----------------------------------------------------------------------------
int initSeen(struct nodeSeen *listSeen) {
	int i;
	for (i=0; i< _SEENMAX; i++) {
		listSeen[i].idSeen=0;
		listSeen[i].sfSeen=0;
		listSeen[i].cntSeen=0;
		listSeen[i].chnSeen=0;
		listSeen[i].timSeen=0;
	}
	return(1);
}


// ----------------------------------------------------------------------------
// readSeen
// This function read the stored information from writeSeen.
//
// Parameters:
// Return:
// ----------------------------------------------------------------------------
int readSeen(const char *fn, struct nodeSeen *listSeen) {
	int i;
	if (!SPIFFS.exists(fn)) {
#if _DUSB>=1
		Serial.print("WARNING:: readSeen, history file not exists ");
#endif //_DUSB
		initSeen(listSeen);		// XXX make all initial declarations here if config vars need to have a value
		Serial.println(fn);
		return(-1);
	}
	File f = SPIFFS.open(fn, "r");
	if (!f) {
#if _DUSB>=1
		Serial.print("readConfig:: ERROR open file=");
		Serial.print(fn);
		Serial.println();
#endif //_DUSB
		return(-1);
	}
	for (i=0; i<_SEENMAX; i++) {
		String val;
		if (!f.available()) {
#if _DUSB>=1
			Serial.println(F("readSeen:: No more info left in file"));
#endif //_DUSB
		}
		val=f.readStringUntil('\t'); listSeen[i].timSeen = (uint32_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].idSeen = (uint32_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].cntSeen = (uint32_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].chnSeen = (uint8_t) val.toInt();
		val=f.readStringUntil('\n'); listSeen[i].sfSeen = (uint8_t) val.toInt();
	}
	f.close();
#if _DUSB>=1
	printSeen(listSeen);
#endif //_DUSB
}


// ----------------------------------------------------------------------------
// writeSeen
// Once every few messages, update the SPIFFS file and write the array.
// Parameters:
// - fn contains the filename to write
// - listSeen contains the _SEENMAX array of list structures 
// Return values:
// - return 1 on success
// ----------------------------------------------------------------------------
int writeSeen(const char *fn, struct nodeSeen *listSeen) {
	int i;
	if (!SPIFFS.exists(fn)) {
#if _DUSB>=1
		Serial.print("WARNING:: writeSeen, file not exists, formatting ");
#endif //_DUSB
		initSeen(listSeen);		// XXX make all initial declarations here if config vars need to have a value
#if _DUSB>=1
		Serial.println(fn);
#endif //_DUSB
	}
	File f = SPIFFS.open(fn, "w");
	if (!f) {
#if _DUSB>=1
		Serial.print("writeConfig:: ERROR open file=");
		Serial.print(fn);
		Serial.println();
#endif //_DUSB
		return(-1);
	}

	for (i=0; i<_SEENMAX; i++) {

			unsigned long timSeen;
			f.print(listSeen[i].timSeen);		f.print('\t');
			// Typecast to long to avoid errors in unsigned conversion!
			f.print((long) listSeen[i].idSeen);	f.print('\t');
			f.print(listSeen[i].cntSeen);		f.print('\t');
			f.print(listSeen[i].chnSeen);		f.print('\t');
			f.print(listSeen[i].sfSeen);		f.print('\n');
			//f.print(listSeen[i].rssiSeen);	f.print('\t');
			//f.print(listSeen[i].datSeen);		f.print('\n');			
	}
	
	f.close();
	
	return(1);
}

// ----------------------------------------------------------------------------
// printSeen
// - This function writes the last seen array to the USB
// ----------------------------------------------------------------------------
int printSeen(struct nodeSeen *listSeen) {
    int i;
#if _DUSB>=1
	if (( debug>=2 ) && ( pdebug & P_MAIN )) {
		Serial.println(F("printSeen:: "));
		for (i=0; i<_SEENMAX; i++) {
			if (listSeen[i].idSeen != 0) {
				String response;
				
				Serial.print(i);
				Serial.print(F(", TM="));

				stringTime(listSeen[i].timSeen, response);
				Serial.print(response);
								
				Serial.print(F(", addr=0x"));
				Serial.print(listSeen[i].idSeen,HEX);
				Serial.print(F(", SF=0x"));
				Serial.print(listSeen[i].sfSeen,HEX);
				Serial.println();
			}
		}
	}
#endif //_DUSB
	return(1);
}

// ----------------------------------------------------------------------------
// addSeen
//	With every new message received:
// - Look whether the message is already in the array, if so update existing 
//	message. If not, create new record.
// - With this record, update the SF settings
// ----------------------------------------------------------------------------
int addSeen(struct nodeSeen *listSeen, struct stat_t stat) {
	int i;
	
//	( message[4]<<24 | message[3]<<16 | message[2]<<8 | message[1] )

	for (i=0; i< _SEENMAX; i++) {
		if ((listSeen[i].idSeen==stat.node) ||
			(listSeen[i].idSeen==0))
		{
#if _DUSB>=2
			if (( debug>=0 ) && ( pdebug & P_MAIN )) {
				Serial.print(F("addSeen:: index="));
				Serial.print(i);
			}
#endif //_DUSB
			listSeen[i].idSeen = stat.node;
			listSeen[i].chnSeen = stat.ch;
			listSeen[i].sfSeen |= stat.sf;			// Or the argument
			listSeen[i].timSeen = stat.tmst;
			listSeen[i].cntSeen++;					// Not included on functiin paras
	
			writeSeen(_SEENFILE, listSeen);
			break;
		}
	}
	
	if (i>=_SEENMAX) {
#if _DUSB>=1
		if (( debug>=0 ) && ( pdebug & P_MAIN )) {
			Serial.print(F("addSeen:: exit=0, index="));
			Serial.println(i);
		}
#endif //_DUSB
		return(0);
	}

	return(1);
}



// ----------------------------------------------------------------------------
// listDir
//	List the directory and put it in
// ----------------------------------------------------------------------------
void listDir(char * dir) 
{
#if _DUSB>=1
	// Nothing here
#endif //_DUSB
}



#endif //_SEENMAX>=1 End of File
