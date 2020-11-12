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



// ----------------------------------------------------------------------------
// LoRa Monitor logging code.
// Define one print function and depending on the logging parameter output
// to _USB of to the www screen function
// ----------------------------------------------------------------------------
int initMonitor(struct moniLine *monitor) 
{
#if _MONITOR>=1
	for (int i=0; i< gwayConfig.maxMoni; i++) {
		monitor[i].txt= "-";						// Make all lines empty
	}
	iMoni=0;										// Init the index
#endif //_MONITOR
	return(1);
}



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
	if ((debug>=0) && (pdebug & P_MAIN)) {
		Serial.print(id);
		Serial.print(F("=\t"));
		Serial.println(val);
	}
#endif //_MONITOR
}


// ============================================================================
// config functions
//

// ----------------------------------------------------------------------------
// INITCONFIG; Init the gateway configuration file
// Espcecially when calling SPIFFS.format() the gateway is left in an init state
// which is not very well defined. This function will init some of the settings
// to well known settings.
// ----------------------------------------------------------------------------
void initConfig(struct espGwayConfig *c)
{
	(*c).ch = _CHANNEL;					// Default channel to listen to (for region)
	(*c).sf = _SPREADING;				// Default Spreading Factor when not using CAD
	(*c).debug = 1;						// debug level is 1
	(*c).pdebug = P_GUI | P_MAIN | P_TX;
	(*c).cad = _CAD;
	(*c).hop = false;
	(*c).seen = true;					// Seen interface is ON
	(*c).expert = false;				// Expert interface is OFF
	(*c).monitor = true;				// Monitoring is ON
	(*c).trusted = 1;
	(*c).txDelay = 0;					// First Value without saving is 0;
	(*c).dusbStat = true;
	
	(*c).maxSeen = _MAXSEEN;
	(*c).maxStat = _MAXSTAT;
	(*c).maxMoni = _MAXMONITOR;
	
	// We clean all the file statistics. Maybe we can leave them in, but after a fornat
	// all data has disappeared.
	(*c).logFileRec = 0;				// In new logFile start with record 0
	(*c).logFileNo = 0;					// Increase file ID

	// Declarations that are dependent on the init settings
	// The structure definitions below make it possible to dynamically size and resize the
	// information in the GUI
	free(statr); 
	delay(5);

	statr = (struct stat_t *) malloc((*c).maxStat * sizeof(struct stat_t));
	for (int i=0; i<(*c).maxStat; i++) {
		statr[i].time=0;						// Time since 1970 in seconds
		statr[i].upDown=0;						// Init for Down traffic
		statr[i].node=0;						// 4-byte DEVaddr (the only one known to gateway)
		statr[i].ch=0;							// Channel index to freqs array
		statr[i].sf=0;							// Init Spreading Factor
	}

	free(listSeen); delay(50);
	listSeen = (struct nodeSeen *) malloc((*c).maxSeen * sizeof(struct nodeSeen));
	for (int i=0; i<(*c).maxSeen; i +=1 ) {
		listSeen[i].idSeen=0;
	}

} // initConfig()

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
	(*c).boots++;										// Increment Boot Counter

#	if _GATEWAYNODE==1
		if (gwayConfig.fcnt != (uint8_t) 0) {
			frameCount = gwayConfig.fcnt+10;			// Assume he is only 10 off
		}
#	endif

	writeGwayCfg(_CONFIGFILE, &gwayConfig );			// And writeback the configuration, not to miss a boot

	return 1;
	
} // readGwayCfg()


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
		if ((debug>=0) && (pdebug & P_MAIN)) {
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
		else if (id == "BOOTS") {								// BOOTS setting
			id_print(id, val);
			(*c).boots = (uint16_t) val.toInt();
		}
		else if (id == "CAD") {									// CAD setting
			id_print(id, val);
			(*c).cad = (bool) val.toInt();
		}
		else if (id == "CH") { 									// Frequency Channel
			id_print(id,val); 
			(*c).ch = (uint8_t) val.toInt();
		}
		else if (id == "DEBUG") {								// Debug Level
			id_print(id, val);
			(*c).debug = (uint8_t) val.toInt();
		}
		else if (id == "DELAY") {								// DELAY setting
			id_print(id, val);
			(*c).txDelay = (int32_t) val.toInt();
		}
		else if (id == "EXPERT") {								// EXPERT button setting
			id_print(id, val);
			(*c).expert = (bool) val.toInt();
		}
		else if (id == "FCNT") {								// Frame Counter
			id_print(id, val);
			(*c).fcnt = (uint16_t) val.toInt();
		}
		else if (id == "FILENO") {								// FILENO setting
			id_print(id, val);
			(*c).logFileNo = (uint16_t) val.toInt();
		}
		else if (id == "FILEREC") {								// FILEREC setting
			id_print(id, val);
			(*c).logFileRec = (uint16_t) val.toInt();
		}
		else if (id == "FORMAT") {								// FORMAT setting
			id_print(id, val);
			(*c).formatCntr= (int8_t) val.toInt();
		}
		else if (id == "HOP") {									// HOP setting
			id_print(id, val);
			(*c).hop = (bool) val.toInt();
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
		else if (id == "RESETS") {								// RESET setting
			id_print(id, val);
			(*c).resets = (uint16_t) val.toInt();
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
		else if (id == "PDEBUG") {								// pDebug Pattern
			id_print(id, val);
			(*c).pdebug = (uint8_t) val.toInt();
		}
		else if (id == "SEEN") {								// SEEN button setting
			id_print(id, val);
			(*c).seen = (bool) val.toInt();
		}
		else if (id == "SF") { 									// Spreading Factor
			id_print(id, val);
			(*c).sf = (uint8_t) val.toInt();
		}
		else if (id == "SHOWDATA") { 							// Show data of node Factor
			id_print(id, val);
			(*c).showdata = (uint8_t) val.toInt();
		}
		else if (id == "TRUSTED") {								// TRUSTED setting
			id_print(id, val);
			(*c).trusted= (int8_t) val.toInt();
		}
		else if (id == "VIEWS") {								// VIEWS setting
			id_print(id, val);
			(*c).views = (uint16_t) val.toInt();
		}
		else if (id == "WAITERR") {								// WAITERR setting
			id_print(id, val);
			(*c).waitErr = (uint16_t) val.toInt();
		}
		else if (id == "WAITOK") {								// WAITOK setting
			id_print(id, val);
			(*c).waitOk = (uint16_t) val.toInt();
		}
		else if (id == "WIFIS") {								// WIFIS setting
			id_print(id, val);
			(*c).wifis = (uint16_t) val.toInt();
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
	
} // readConfig()


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
} // writeGwayCfg


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
		mPrint("writeConfig:: ERROR open file="+String(fn));
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
	f.print("WAITERR");	f.print('='); f.print((*c).waitErr);	f.print('\n');
	f.print("WAITOK");	f.print('='); f.print((*c).waitOk);		f.print('\n');
	f.print("NTPS");	f.print('='); f.print((*c).ntps);		f.print('\n');
	f.print("FILEREC");	f.print('='); f.print((*c).logFileRec); f.print('\n');
	f.print("FILENO");	f.print('='); f.print((*c).logFileNo);	f.print('\n');
	f.print("FORMAT");	f.print('='); f.print((*c).formatCntr); f.print('\n');
	f.print("DELAY");	f.print('='); f.print((*c).txDelay); 	f.print('\n');
	f.print("SHOWDATA");f.print('='); f.print((*c).showdata); 	f.print('\n');
	f.print("TRUSTED");	f.print('='); f.print((*c).trusted); 	f.print('\n');
	f.print("EXPERT");	f.print('='); f.print((*c).expert); 	f.print('\n');
	f.print("SEEN");	f.print('='); f.print((*c).seen);		f.print('\n');
	f.print("MONITOR");	f.print('='); f.print((*c).monitor); 	f.print('\n');
	
	f.close();
	return(1);
} // writeConfig()



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

#if _STAT_LOG==1
	File f;
	char fn[16];

	// If the records does not fit in the file anymore, open a new file
	
	if (gwayConfig.logFileRec > LOGFILEREC) {		// If number of records is ;arger than filesize
	
		gwayConfig.logFileRec = 0;					// In new logFile start with record 0
		gwayConfig.logFileNo++;						// Increase file ID
		
		f.close();									// Close the old file
		
		sprintf(fn,"/log-%d", gwayConfig.logFileNo);// Make the new name

		if (gwayConfig.logFileNo > LOGFILEMAX) {	// If we have too many logfies, delete the oldest
			sprintf(fn,"/log-%d", gwayConfig.logFileNo-LOGFILEMAX );
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_RX)) {
				mPrint("addLog:: Too many logfiles, deleting="+String(fn));
			}
#			endif //_MONITOR
			SPIFFS.remove(fn);
		}
	}

	sprintf(fn,"/log-%d", gwayConfig.logFileNo);	// Make sure we have the right fileno
	
	// If there is no SPIFFS, Error
	// Make sure to write the config record/line also
	if (!SPIFFS.exists(fn)) {
#		if _MONITOR>=1
		if ((debug >= 1) && (pdebug & P_RX)) {
			mPrint("addLog:: WARNING file="+String(fn)+" does not exist. record="+String(gwayConfig.logFileRec) );
		}
#		endif //_MONITOR
	}
	
	f = SPIFFS.open(fn, "a");
	if (!f) {
#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_RX)) {
			mPrint("addLOG:: ERROR file open failed="+String(fn));
		}
#		endif //_MONITOR
		return(0);								// If file open failed, return
	}
#	if _MONITOR>=2
	else {
		mPrint("addLog:: Opening adding file="+String(fn));
	}
#	endif


	int i=0;
#	if _MONITOR>=1
	if ((debug>=2) && (pdebug & P_RX)) {
		char s[256];
		i += 12;								// First 12 chars are non printable
		sprintf(s, "addLog:: fileno=%d, rec=%d : %s",gwayConfig.logFileNo,gwayConfig.logFileRec,&line[i]);
		mPrint(s);
	}
#	endif //_MONITOR

	for (i=0; i< 12; i++) {						// The first 12 bytes contain non printable characters
	//	f.print(line[i],HEX);
		f.print('*');
	}
	f.print(now());
	f.print(':');
	f.write(&(line[i]), cnt-12);				// write/append the line to the file
	f.print('\n');
	
	f.close();									// Close the file after appending to it

#endif //_STAT_LOG

	gwayConfig.logFileRec++;

	return(1);
} //addLog()



// ============================================================================
// Below are the xxxSeen() functions. These functions keep track of the kast
// time a device was seen bij the gateway.
// These functions are not round-robin and they do not need to be.

// ----------------------------------------------------------------------------
// initSeen
// Init the listScreen array
// Return:
//	1: Success
// Parameters:
//	listSeen: array of Seen data
// ----------------------------------------------------------------------------
int initSeen(struct nodeSeen *listSeen) 
{
#if _MAXSEEN>=1
	for (int i=0; i< gwayConfig.maxSeen; i++) {
		listSeen[i].idSeen=0;
		listSeen[i].upDown=0;
		listSeen[i].sfSeen=0;
		listSeen[i].cntSeen=0;
		listSeen[i].chnSeen=0;
		listSeen[i].timSeen=(time_t) 0;					// 1 jan 1970 0:00:00 hrs
	}
	iSeen= 0;											// Init index to 0
#endif //_MAXSEEN
	return(1);

} // initSeen()


// ----------------------------------------------------------------------------
// readSeen
// This function read the information stored by printSeen from the file.
// The file is read as String() values and converted to int after.
// Parameters:
//	fn:			Filename
//	listSeen:	Array of all last seen nodes on the LoRa network
// Return:
//	1:			When successful
// ----------------------------------------------------------------------------
int readSeen(const char *fn, struct nodeSeen *listSeen)
{
#if _MAXSEEN>=1
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
			mPrint("readSeen:: ERROR open file="+String(fn));
#		endif //_MONITOR
		return(-1);
	}

	delay(1000);
	
	for (i=0; i<gwayConfig.maxSeen; i++) {
		delay(200);
		String val="";
		
		if (!f.available()) {
#			if _MONITOR>=2
				mPrint("readSeen:: No more info left in file, i=" + String(i));
#			endif //_MONITOR
			break;
		}
		val=f.readStringUntil('\t'); listSeen[i].timSeen = (time_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].upDown = (uint8_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].idSeen = (int64_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].cntSeen = (uint32_t) val.toInt();
		val=f.readStringUntil('\t'); listSeen[i].chnSeen = (uint8_t) val.toInt();
		val=f.readStringUntil('\n'); listSeen[i].sfSeen = (uint8_t) val.toInt();

		if ((int32_t)listSeen[i].idSeen != 0) {
			iSeen++;									// Increase index, new record read
		}

#		if _MONITOR>=1

		if ((debug>=2) && (pdebug & P_MAIN)) {
				mPrint(" readSeen:: idSeen ="+String(listSeen[i].idSeen,HEX)+", i="+String(i));
		}
#		endif								
	}

	f.close();

#endif //_MAXSEEN

	// So we read iSeen records
	return 1;
	
} // readSeen()


// ----------------------------------------------------------------------------
// printSeen
// Once every few messages, update the SPIFFS file and write the array.
// Parameters:
// - fn contains the filename to write
// - listSeen contains the _MAXSEEN array of list structures 
// Return values:
// - return 1 on success
// ----------------------------------------------------------------------------
int printSeen(const char *fn, struct nodeSeen *listSeen)
{
#if _MAXSEEN>=1
	int i;
	if (!SPIFFS.exists(fn)) {
#		if _MONITOR>=1
			mPrint("WARNING:: printSeen, file not exists="+String(fn));
#		endif //_MONITOR
		//initSeen(listSeen);		// XXX make all initial declarations here if config vars need to have a value
	}
	
	File f = SPIFFS.open(fn, "w");
	if (!f) {
#		if _MONITOR>=1
			mPrint("printSeen:: ERROR open file="+String(fn)+" for writing");
#		endif //_MONITOR
		return(-1);
	}
	delay(500);

	for (i=0; i<iSeen; i++) {							// For all records indexed
		if ((int32_t)listSeen[i].idSeen == 0) break;
		
		f.print((time_t)listSeen[i].timSeen);	f.print('\t');
		f.print((uint8_t)listSeen[i].upDown);	f.print('\t');
		f.print((int32_t)listSeen[i].idSeen);	f.print('\t'); // Typecast to avoid errors in unsigned conversion!
		f.print((uint32_t)listSeen[i].cntSeen);	f.print('\t');
		f.print((uint8_t)listSeen[i].chnSeen);	f.print('\t');
		f.print((uint8_t)listSeen[i].sfSeen);	f.print('\n');

		// MMM
#		if _MONITOR>=1
		if ((debug >= 2) && (pdebug & P_TX)) {
			if (listSeen[i].upDown == 1) {
				mPrint("printSeen:: ERROR f.print, upDown="+String(listSeen[i].upDown)+", i="+String(i) );
			}
		}
#		endif //_MONITOR
	}

	f.close();
#endif //_MAXSEEN
	return(1);
	
} //writeseen()



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

#if _MAXSEEN>=1
	int i;
	
	for (i=0; i<iSeen; i++) {						// For all known records

		// If the record node is equal, we found the record already.
		// So increment cntSeen
		if ((listSeen[i].idSeen==stat.node)	&&
			(listSeen[i].upDown==stat.upDown))
		{
			//listSeen[i].upDown	= stat.upDown;		// Not necessary, is the same
			//listSeen[i].idSeen	= stat.node;		// Not necessary, is the same
			listSeen[i].timSeen		= (time_t)stat.time;
			listSeen[i].chnSeen		= stat.ch;
			listSeen[i].sfSeen		= stat.sf;			// The SF argument
			listSeen[i].cntSeen++;					// Not included on function para
//			printSeen(_SEENFILE, listSeen);
			
#			if _MONITOR>=2
			if ((debug>=3) && (pdebug & P_MAIN)) {
				mPrint("addSeen:: adding i="+String(i)+", node="+String(stat.node,HEX));
			}
#			endif

			return 1;
		}
	}

	// else: We did not find the current record so make a new listSeen entry
	if ((i>=iSeen) && (i<gwayConfig.maxSeen)) {
		listSeen[i].upDown	= stat.upDown;
		listSeen[i].idSeen	= stat.node;
		listSeen[i].chnSeen	= stat.ch;
		listSeen[i].sfSeen	= stat.sf;				// The SF argument
		listSeen[i].timSeen	= (time_t)stat.time;	// Timestamp correctly
		listSeen[i].cntSeen	= 1;					// We see this for the first time	
		iSeen++;
	}

#	if _MONITOR>=1
	if ((debug>=2) && (pdebug & P_MAIN)) {
		String response= "  addSeen:: i=";
		response += i;
		response += ", tim=";
		stringTime(stat.time, response);
		response += ", iSeen=";
		response += String(iSeen);
		response += ", node=";
		response += String(stat.node,HEX);
		response += ", listSeen[0]=";
		printHex(listSeen[0].idSeen,':',response);

		mPrint(response);
	}
#	endif //_MONITOR

#endif //_MAXSEEN>=1 
	return 1;
	
} //addSeen()

// End of File
