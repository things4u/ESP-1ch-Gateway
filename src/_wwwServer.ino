// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2020 Maarten Westenberg version for ESP8266
//
// 	based on work done by many people and making use of several libraries.
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
// This file contains the webserver code for the ESP Single Channel Gateway.

// Note:
// The ESP Webserver works with Strings to display HTML content. 
// Care must be taken that not all data is output to the webserver in one string
// as this will use a LOT of memory and possibly kill the heap (cause system
// crash or other unreliable behaviour.
// Instead, output of the various tables on the webpage should be displayed in
// chunks so that strings are limited in size.
// Be aware that using no strings but only sendContent() calls has its own
// disadvantage that these calls take a lot of time and cause the page to be
// displayed like an old typewriter.
// So, the trick is to make chucks that are sent to the website by using
// a response String but not make those Strings too big.
//
// Also, selecting too many options for Statistics, display, Hopping channels
// etc makes the gateway more sluggish and may impact the available memory and 
// thus its performance and reliability. It's up to the user to select wisely!
//


//
// The remainder of the file ONLY works if _SERVER=1 is set.
//
#if _SERVER==1

// ================================================================================
// WEBSERVER DECLARATIONS 
// ================================================================================

// None at the moment


// ================================================================================
// WEBSERVER FUNCTIONS 
// ================================================================================


// --------------------------------------------------------------------------------
// Used by all functions requiring user confirmation
// Displays a menu by user and two buttons "OK" and "CANCEL"
// The function returns true for OK and false for CANCEL
// Usage: Call this function ONCE during startup to declare and init
// the ynDialog JavaScript function, and call the function
// from the program when needed.
// Paramters:
//	s: Th strig contining the question to be answered
//	o: The OK tab for the webpage where to go to
//	c: The Cancel string (optional)
// Return:
//	Boolean when success
// --------------------------------------------------------------------------------
boolean YesNo()
{
	boolean ret = false;
	String response = "";
	response += "<script>";
	
	response += "var ch = \"\"; ";								// Init choice
	response += "function ynDialog(s,y) {";
	response += "  try { adddlert(s); }";
	response += "  catch(err) {";
	response += "    ch  = \" \" + s + \".\\n\\n\"; ";
	response += "    ch += \"Click OK to continue,\\n\"; ";
	response += "    ch += \"or Cancel\\n\\n\"; ";
	response += "    if(!confirm(ch)) { ";
	response += "      javascript:window.location.reload(true);";
	response += "    } ";
	response += "    else { ";
	response += "      document.location.href = '/'+y; ";
	response += "    } ";
	response += "  }";
	response += "}";
	response += "</script>";
	server.sendContent(response);
	
	return(ret);
}


// --------------------------------------------------------------------------------
// WWWFILE
// This function will open a pop-up in the browser and then 
//	display the contents of a file in that window
// Output is sent to server.sendContent()
//	Note: The output is not in a variable, its size would be too large
// NOTE: Only for _STAT_LOG at the moment
// Parameters:
//		fn; String with filename
// Returns:
//		<none>
// --------------------------------------------------------------------------------
void wwwFile(String fn)
{
#if _STAT_LOG == 1

	if (!SPIFFS.exists(fn)) {
#		if _MONITOR>=1
			mPrint("wwwFile:: ERROR: SPIFFS file not found="+fn);
#		endif //_MONITOR
		return;
	} 
#	if _MONITOR>=1
	else if (debug>=1) {
		mPrint("wwwFile:: SPIFFS Filesystem exists= " + String(fn));
	}
	uint16_t readFile = 0;
#	endif //_MONITOR

#	if _MONITOR>=1

	File f = SPIFFS.open(fn, "r");					// Open the file for reading

	// If file is available
	while (f.available()) {

		String s=f.readStringUntil('\n');
		if (s.length() == 0) {
			Serial.print(F("wwwFile:: String length 0"));
			break;
		}
		server.sendContent(s.substring(12));		// Skip the first 12 Gateway specific binary characters
		server.sendContent("\n");

		readFile++;

		yield();
	}

	f.close();

#	endif //_MONITOR

#endif //_STAT_LOG

	return;

}

// --------------------------------------------------------------------------------
// Button function Docu, display the documentation pages.
// This is a button on the top of the GUI screen.
// --------------------------------------------------------------------------------
void buttonDocu()
{

	String response = "";
	response += "<script>";
	
	response += "var txt = \"\";";
	response += "function showDocu() {";
	response += "  try { adddlert(\"Welcome,\"); }";
	response += "  catch(err) {";
	response += "    txt  = \"Do you want the documentation page.\\n\\n\"; ";
	response += "    txt += \"Click OK to continue viewing documentation,\\n\"; ";
	response += "    txt += \"or Cancel to return to the home page.\\n\\n\"; ";
	response += "    if(confirm(txt)) { ";
	response += "      document.location.href = \"https://things4u.github.io/Projects/SingleChannelGateway/index.html\"; ";
	response += "    }";
	response += "  }";
	response += "}";
	
	response += "</script>";
	server.sendContent(response);
	
	return;
}




// --------------------------------------------------------------------------------
// Button function Log displays  logfiles.
// This is a button on the top of the GUI screen
// --------------------------------------------------------------------------------
void buttonLog() 
{
#if _STAT_LOG == 1	

	int startFile = (gwayConfig.logFileNo > LOGFILEMAX ? (gwayConfig.logFileNo - LOGFILEMAX) : 0);
	
#	if _MONITOR>=1
		mPrint("buttonLog:: gwayConfig.logFileNo="+String(gwayConfig.logFileNo)+", LOGFILEMAX="+String(LOGFILEMAX)+", startFile="+String(startFile)+", recs="+String(gwayConfig.logFileRec) );
#	endif //_MONITOR

	for (int i=startFile; i<= gwayConfig.logFileNo; i++ ) {
		String fn = "/log-" + String(i);
		wwwFile(fn);									// Display the file contents in the browser
	}
	
#endif //_STAT_LOG

	return;
}


// --------------------------------------------------------------------------------
// Navigate webpage by buttons. This method has some advantages:
// - Less time/cpu usage
// - Less memory usage		<a href=\"SPEED=160\">
// --------------------------------------------------------------------------------
static void wwwButtons()
{
	String response = "";
	String mode = (gwayConfig.expert ? "Basic Mode" : "Expert Mode");
	String moni = (gwayConfig.monitor ? "Hide Monitor" : "Monitor ON");
	String seen = (gwayConfig.seen ? "Hide Seen" : "Last seen ON");

	YesNo();												// Init the Yes/No function
	buttonDocu();

	response += "<input type=\"button\" value=\"Documentation\" onclick=\"showDocu()\" >";
#	if _STAT_LOG == 1
	response += "<a href=\"LOG\" download><button type=\"button\">Log Files</button></a>";
#	endif //__STAT_LOG

	response += "<a href=\"EXPERT\" download><button type=\"button\">" + mode + "</button></a>";
#	if _MONITOR>=1
	response += "<a href=\"MONITOR\" download><button type=\"button\">" +moni+ "</button></a>";
#	endif //_MONITOR

#	if _MAXSEEN>=1
	response += "<a href=\"SEEN\" download><button type=\"button\">" +seen+ "</button></a>";
#	endif //_MAXSEEN
	server.sendContent(response);							// Send to the screen
	
	return;
}


// --------------------------------------------------------------------------------
// SET ESP8266/ESP32 WEB SERVER VARIABLES
//
// This funtion implements the WiFi Webserver (very simple one). The purpose
// of this server is to receive simple admin commands, and execute these
// results which are sent back to the web client.
// Commands: DEBUG, ADDRESS, IP, CONFIG, GETTIME, SETTIME
// The webpage is completely built response and then printed on screen.
//
// Parameters:
//		cmd: Contains a character array with the command to execute
//		arg: Contains the parameter value of that command
// Returns:
//		<none>
// --------------------------------------------------------------------------------
static void setVariables(const char *cmd, const char *arg)
{

	// DEBUG settings; These can be used as a single argument
	if (strcmp(cmd, "DEBUG")==0) {									// Set debug level 0-2
		if (atoi(arg) == 1) {
			debug = (debug+1)%4;
		}	
		else if (atoi(arg) == -1) {
			debug = (debug+3)%4;
		}
		writeGwayCfg(_CONFIGFILE, &gwayConfig );					// Save configuration to file
	}
	
	if (strcmp(cmd, "CAD")==0) {									// Set -cad on=1 or off=0
		gwayConfig.cad=(bool)atoi(arg);
		writeGwayCfg(_CONFIGFILE, &gwayConfig );					// Save configuration to file
	}
	
	if (strcmp(cmd, "HOP")==0) {									// Set -hop on=1 or off=0
		gwayConfig.hop=(bool)atoi(arg);
		if (! gwayConfig.hop) { 
			setFreq(freqs[gwayConfig.ch].upFreq);			
			rxLoraModem();
			sf = SF7;
			cadScanner();
		}
		writeGwayCfg(_CONFIGFILE, &gwayConfig );					// Save configuration to file
	}
	
	// DELAY, write txDelay for transmissions
	//
	if (strcmp(cmd, "DELAY")==0) {									// Set delay usecs
		gwayConfig.txDelay+=atoi(arg)*1000;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );					// Save configuration to file
	}

	// TRUSTED, write node trusted value 
	//
	if (strcmp(cmd, "TRUSTED")==0) {								// Set delay usecs
		gwayConfig.trusted=atoi(arg);
		if (atoi(arg) == 1) {
			gwayConfig.trusted = (gwayConfig.trusted +1)%4;
		}	
		else if (atoi(arg) == -1) {
			gwayConfig.trusted = (gwayConfig.trusted -1)%4;
		}
		writeGwayCfg(_CONFIGFILE, &gwayConfig );					// Save configuration to file
	}
	
	// SF; Handle Spreading Factor Settings
	//
	if (strcmp(cmd, "SF")==0) {
		if (atoi(arg) == 1) {
			if (sf>=SF12) sf=SF7; else sf= (sf_t)((int)sf+1);
		}	
		else if (atoi(arg) == -1) {
			if (sf<=SF7) sf=SF12; else sf= (sf_t)((int)sf-1);
		}
		rxLoraModem();												// Reset the radio with the new spreading factor
		writeGwayCfg(_CONFIGFILE, &gwayConfig );					// Save configuration to file
	}
	
	// FREQ; Handle Frequency  Settings
	//
	if (strcmp(cmd, "FREQ")==0) {
		uint8_t nf = sizeof(freqs)/ sizeof(freqs[0]);				// Number of frequency elements in array
		
		// Compute frequency index
		if (atoi(arg) == 1) {
			if (gwayConfig.ch==(nf-1)) gwayConfig.ch=0; else gwayConfig.ch++;
		}
		else if (atoi(arg) == -1) {
			Serial.println("down");
			if (gwayConfig.ch==0) gwayConfig.ch=(nf-1); else gwayConfig.ch--;
		}
		setFreq(freqs[gwayConfig.ch].upFreq);
		rxLoraModem();												// Reset the radio with the new frequency
		writeGwayCfg(_CONFIGFILE, &gwayConfig );						// Save configuration to file
	}

	if (strcmp(cmd, "GETTIME")==0) { 								// Get the local time
		Serial.println(F("gettime tbd")); 
	}	
	
	//if (strcmp(cmd, "SETTIME")==0) { Serial.println(F("settime tbd")); }	// Set the local time
	
	// Help
	if (strcmp(cmd, "HELP")==0)    { Serial.println(F("Display Help Topics")); }
	
	// Node
#if _GATEWAYNODE==1
	if (strcmp(cmd, "NODE")==0) {									// Set node on=1 or off=0
		gwayConfig.isNode =(bool)atoi(arg);
		writeGwayCfg(_CONFIGFILE, &gwayConfig );					// Save configuration to file
	}
	
	// File Counter//
	if (strcmp(cmd, "FCNT")==0)   { 
		frameCount=0; 
		rxLoraModem();												// Reset the radio with the new frequency
		writeGwayCfg(_CONFIGFILE, &gwayConfig );
	}
#endif
	
	// WiFi Manager
	//
#if _WIFIMANAGER==1
	if (strcmp(cmd, "NEWSSID")==0) {
	
		//and goes into a blocking loop awaiting configuration
		WiFiManager wifiManager;
#		if _MONITOR>=1		
		if (!wifiManager.autoConnect()) {
			Serial.println("failed to connect and hit timeout");
			ESP.restart();
			delay(1000);
		}
#		endif //_MONITOR
		
		//WiFi.disconnect();
		
		// Add the ID to the SSID of the WiFiManager
		String ssid = String(AP_NAME) + "-" + String(ESP_getChipId(), HEX);
		
#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_MAIN)) {
			mPrint("Set Variables:: ssid="+ ssid );
		}
#		endif //_MONITOR

		// wifiManager.startConfigPortal( ssid.c_str(), AP_PASSWD );
	}
#endif //_WIFIMANAGER

	// Update the software (from User Interface)
#if _OTA==1
	if (strcmp(cmd, "UPDATE")==0) {
		if (atoi(arg) == 1) {
			updateOtaa();
			writeGwayCfg(_CONFIGFILE, &gwayConfig );
		}
	}
#endif

#if _REFRESH==1
	if (strcmp(cmd, "REFR")==0) {									// Set refresh on=1 or off=0
		gwayConfig.refresh =(bool)atoi(arg);
		writeGwayCfg(_CONFIGFILE, &gwayConfig );					// Save configuration to file
	}
#endif

}


// --------------------------------------------------------------------------------
// OPEN WEB PAGE
//	This is the init function for opening the webpage
//
// --------------------------------------------------------------------------------
static void openWebPage()
{
	++gwayConfig.views;											// increment number of views
	String response="";	

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	
	// init webserver, fill the webpage
	// NOTE: The page is renewed every _WWW_INTERVAL seconds, please adjust in configGway.h
	//
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text/html", "");
	String tt=""; printIP((IPAddress)WiFi.localIP(),'.',tt);	// Time with separator
	
#if _REFRESH==1
	if (gwayConfig.refresh) {
		response += String() + "<!DOCTYPE HTML><HTML><HEAD><meta http-equiv='refresh' content='"+_WWW_INTERVAL+";http://";
		response += tt;
		response += "'><TITLE>1ch Gateway " + String(tt) + "</TITLE>";
	}
	else {
		response += String("<!DOCTYPE HTML><HTML><HEAD><TITLE>1ch Gateway " + String(tt) + "</TITLE>");
	}
#else
	response += String("<!DOCTYPE HTML><HTML><HEAD><TITLE>1ch Gateway " + String(tt) + "</TITLE>");
#endif
	response += "<META HTTP-EQUIV='CONTENT-TYPE' CONTENT='text/html; charset=UTF-8'>";
	response += "<META NAME='AUTHOR' CONTENT='M. Westenberg (mw1554@hotmail.com)'>";

	response += "<style>.thead {background-color:green; color:white;} ";
	response += ".cell {border: 1px solid black;}";
	response += ".config_table {max_width:100%; min-width:300px; width:98%; border:1px solid black; border-collapse:collapse;}";
	response += "</style></HEAD><BODY>";
	
	response +="<h1>ESP Gateway Config</h1>";

	response +="<p style='font-size:10px;'>";
	response +="Version: "; response+=VERSION;
	response +="<br>ESP alive since "; 					// STARTED ON
	stringTime(startTime, response);

	response +=", Uptime: ";							// UPTIME
	uint32_t secs = millis()/1000;
	uint16_t days = secs / 86400;						// Determine number of days
	uint8_t _hour   = hour(secs);
	uint8_t _minute = minute(secs);
	uint8_t _second = second(secs);
	response += String(days) + "-";
	
	if (_hour < 10) response += "0"; response += String(_hour) + ":";
	if (_minute < 10) response += "0"; response += String(_minute) + ":";
	if (_second < 10) response += "0"; 	response += String(_second);
	
	response +="<br>Current time    "; 					// CURRENT TIME
	stringTime(now(), response);
	response +="<br>";
	response +="</p>";
	
	server.sendContent(response);
}


// --------------------------------------------------------------------------------
// H2 Gateway Settings
//
// Display the configuration and settings data. This is an interactive setting
// allowing the user to set CAD, HOP, Debug and several other operating parameters
//
// --------------------------------------------------------------------------------
static void gatewaySettings()
{
	String response="";
	String bg="";

	response +="<h2>Gateway Settings</h2>";

	response +="<table class=\"config_table\">";
	response +="<tr>";
	response +="<th class=\"thead\">Setting</th>";
	response +="<th colspan=\"2\" style=\"background-color: green; color: white; width:120px;\">Value</th>";
	response +="<th colspan=\"2\" style=\"background-color: green; color: white; width:100px;\">Set</th>";
	response +="</tr>";

	bg = " background-color: ";
	bg += ( gwayConfig.cad ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">CAD</td>";
	response +="<td colspan=\"2\" style=\"border: 1px solid black;"; response += bg; response += "\">";
	response += ( gwayConfig.cad ? "ON" : "OFF" );
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"CAD=0\"><button>OFF</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"CAD=1\"><button>ON</button></a></td>";
	response +="</tr>";

	bg = " background-color: ";
	bg += ( gwayConfig.hop ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">HOP</td>";
	response +="<td colspan=\"2\" style=\"border: 1px solid black;"; response += bg; response += "\">";
	response += ( gwayConfig.hop ? "ON" : "OFF" );
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"HOP=0\"><button>OFF</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"HOP=1\"><button>ON</button></a></td>";
	response +="</tr>";


	response +="<tr><td class=\"cell\">SF Setting</td><td class=\"cell\" colspan=\"2\">";
	if (gwayConfig.cad) {
		response += "AUTO</td>";
	}
	else {
		response += sf;
		response +="<td class=\"cell\"><a href=\"SF=-1\"><button>-</button></a></td>";
		response +="<td class=\"cell\"><a href=\"SF=1\"><button>+</button></a></td>";
	}
	response +="</tr>";

	// Channel
	response +="<tr><td class=\"cell\">Channel</td>";
	response +="<td class=\"cell\" colspan=\"2\">"; 
	if (gwayConfig.hop) {
		response += "AUTO</td>";
	}
	else {
		response += String(gwayConfig.ch); 
		response +="</td>";
		response +="<td class=\"cell\"><a href=\"FREQ=-1\"><button>-</button></a></td>";
		response +="<td class=\"cell\"><a href=\"FREQ=1\"><button>+</button></a></td>";
	}
	response +="</tr>";

	// Trusted options, when TRUSTED_NODE is set
#ifdef _TRUSTED_NODES
	response +="<tr><td class=\"cell\">Trusted Nodes</td><td class=\"cell\" colspan=\"2\">"; 
	response +=gwayConfig.trusted; 
	response +="</td>";
	response +="<td class=\"cell\"><a href=\"TRUSTED=-1\"><button>-</button></a></td>";
	response +="<td class=\"cell\"><a href=\"TRUSTED=1\"><button>+</button></a></td>";
	response +="</tr>";
#endif //_TRUSTED_NODES

	// Debugging options, only when _DUSB or _MONITOR is set, otherwise no
	// serial or minotoring activity
#if _DUSB>=1 || _MONITOR>=1
	response +="<tr><td class=\"cell\">Debug Level</td><td class=\"cell\" colspan=\"2\">"; 
	response +=debug; 
	response +="</td>";
	response +="<td class=\"cell\"><a href=\"DEBUG=-1\"><button>-</button></a></td>";
	response +="<td class=\"cell\"><a href=\"DEBUG=1\"><button>+</button></a></td>";
	response +="</tr>";

	// Debug Pattern
	response +="<tr><td class=\"cell\">Debug pattern</td>"; 

	bg = ( (pdebug & P_SCAN) ? "LightGreen" : "orange" ); 
	response +="<td class=\"cell\" style=\"border: 1px solid black; width:20px; background-color: ";
	response += bg;	response += "\">";
	response +="<a href=\"PDEBUG=SCAN\">";
	response +="<button>SCN</button></a></td>";

	bg = ( (pdebug & P_CAD) ? "LightGreen" : "orange" ); 
	response +="<td class=\"cell\" style=\"border: 1px solid black; width:20px; background-color: ";
	response += bg;	response += "\">";
	response +="<a href=\"PDEBUG=CAD\">";
	response +="<button>CAD</button></a></td>";

	bg = ( (pdebug & P_RX) ? "LightGreen" : "orange" ); 
	response +="<td class=\"cell\" style=\"border: 1px solid black; width:20px; background-color: ";
	response += bg;	response += "\">";
	response +="<a href=\"PDEBUG=RX\">";
	response +="<button>RX</button></a></td>";

	bg = ( (pdebug & P_TX) ? "LightGreen" : "orange" ); 
	response +="<td class=\"cell\" style=\"border: 1px solid black; width:20px; background-color: ";
	response += bg;	response += "\">";
	response +="<a href=\"PDEBUG=TX\">";
	response +="<button>TX</button></a></td>";
	response += "</tr>";

	// Use a second Line
	response +="<tr><td class=\"cell\"></td>";
	bg = ( (pdebug & P_PRE) ? "LightGreen" : "orange" ); 
	response +="<td class=\"cell\" style=\"border: 1px solid black; width:20px; background-color: ";
	response += bg;	response += "\">";
	response +="<a href=\"PDEBUG=PRE\">";
	response +="<button>PRE</button></a></td>";

	bg = ( (pdebug & P_MAIN) ? "LightGreen" : "orange" ); 
	response +="<td class=\"cell\" style=\"border: 1px solid black; width:20px; background-color: ";
	response += bg;	response += "\">";
	response +="<a href=\"PDEBUG=MAIN\">";
	response +="<button>MAI</button></a></td>";

	bg = ( (pdebug & P_GUI) ? "LightGreen" : "orange" ); 
	response +="<td class=\"cell\" style=\"border: 1px solid black; width:20px; background-color: ";
	response += bg;	response += "\">";
	response +="<a href=\"PDEBUG=GUI\">";
	response +="<button>GUI</button></a></td>";

	bg = ( (pdebug & P_RADIO) ? "LightGreen" : "orange" ); 
	response +="<td class=\"cell\" style=\"border: 1px solid black; width:20px; background-color: ";
	response += bg;	response += "\">";
	response +="<a href=\"PDEBUG=RADIO\">";
	response +="<button>RDIO</button></a></td>";
	response +="</tr>";
#endif

	// USB Debug, Serial Debugging
	bg = " background-color: ";
	bg += ( (gwayConfig.dusbStat == 1) ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">Usb Debug</td>";
	response +="<td class=\"cell\" colspan=\"2\" style=\"border: 1px solid black; " + bg + "\">";
	response += ( (gwayConfig.dusbStat == true) ? "ON" : "OFF" );
	response +="</td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"DUSB=0\"><button>OFF</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"DUSB=1\"><button>ON</button></a></td>";
	response +="</tr>";

#if _LOCALSERVER>=1
	// Show Node Data
	bg = " background-color: ";
	bg += ( (gwayConfig.showdata == 1) ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">Show Node Data</td>";
	response +="<td class=\"cell\" colspan=\"2\" style=\"border: 1px solid black; " + bg + "\">";
	response += ( (gwayConfig.showdata == true) ? "ON" : "OFF" );
	response +="</td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"SHOWDATA=0\"><button>OFF</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"SHOWDATA=1\"><button>ON</button></a></td>";
	response +="</tr>";

#endif //_LOCALSERVER

	/// WWW Refresh
#if _REFRESH==1
	bg = " background-color: ";
	bg += ( (gwayConfig.refresh == 1) ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">WWW Refresh</td>";
	response +="<td class=\"cell\" colspan=\"2\" style=\"border: 1px solid black; " + bg + "\">";
	response += ( (gwayConfig.refresh == 1) ? "ON" : "OFF" );
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"REFR=0\"><button>OFF</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"REFR=1\"><button>ON</button></a></td>";
	response +="</tr>";
#endif

#if _GATEWAYNODE==1
	response +="<tr><td class=\"cell\">Framecounter Internal Sensor</td>";
	response +="<td class=\"cell\" colspan=\"2\">";
	response +=frameCount;
	response +="</td><td colspan=\"2\" style=\"border: 1px solid black;\">";
	response +="<button><a href=\"/FCNT\">RESET   </a></button></td>";
	response +="</tr>";

	bg = " background-color: ";
	bg += ( (gwayConfig.isNode == 1) ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">Gateway Node</td>";
	response +="<td class=\"cell\" colspan=\"2\"  style=\"border: 1px solid black;" + bg + "\">";
	response += ( (gwayConfig.isNode == true) ? "ON" : "OFF" );
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"NODE=1\"><button>ON</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"NODE=0\"><button>OFF</button></a></td>";
	response +="</tr>";
#endif

#if _REPEATER>=0
	// Show repeater setting
	bg = " background-color: ";
	bg += ( _REPEATER==1 ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">Repeater</td>";
	response +="<td colspan=\"2\" style=\"border: 1px solid black;"; response += bg; response += "\">";
	response += ( _REPEATER==1 ? "ON" : "OFF" );
	//response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"REPT=0\"><button>OFF</button></a></td>";
	//response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"REPT=1\"><button>ON</button></a></td>";
	response +="</tr>";
#endif

	// Format the Filesystem
	response +="<tr><td class=\"cell\">Format SPIFFS</td>";
	response +=String() + "<td class=\"cell\" colspan=\"2\" >"+gwayConfig.formatCntr+"</td>";
	response +="<td style=\"width:30px;\" colspan=\"2\" class=\"cell\"><input type=\"button\" value=\"FORMAT\" onclick=\"ynDialog(\'Do you really want to format?\',\'FORMAT\')\" /></td></tr>";

	// Reset all statistics
#if _STATISTICS >= 1
	response +="<tr><td class=\"cell\">Statistics</td>";
	response +=String() + "<td class=\"cell\" colspan=\"2\" >"+statc.resets+"</td>";
	response +="<td style=\"width:30px;\" colspan=\"2\" class=\"cell\"><input type=\"button\" value=\"RESET   \" onclick=\"ynDialog(\'Do you really want to reset statistics?\',\'RESET\')\" /></td></tr>";

	// Reset Node
	response +="<tr><td class=\"cell\">Boots and Resets</td>";
	response +=String() + "<td class=\"cell\" colspan=\"2\" >"+gwayConfig.boots+"</td>";
	response +="<td style=\"width:30px;\" colspan=\"2\" class=\"cell\" ><input type=\"button\" value=\"BOOT    \" onclick=\"ynDialog(\'Do you want to reset boots?\',\'BOOT\')\" /></td></tr>";
#endif //_STATISTICS

	response +="</table>";

	server.sendContent(response);
}


// --------------------------------------------------------------------------------
// H2 Package Statistics
//
// This section display a matrix on the screen where everay channel and spreading
// factor is displayed.
// --------------------------------------------------------------------------------
static void statisticsData()
{
	String response="";
	//
	// Header Row
	//
	response +="<h2>Package Statistics</h2>";
	response +="<table class=\"config_table\">";
	response +="<tr><th class=\"thead\">Counter</th>";
#	if _STATISTICS == 3
		response +="<th class=\"thead\">C 0</th>";
		response +="<th class=\"thead\">C 1</th>";
		response +="<th class=\"thead\">C 2</th>";
#	endif //_STATISTICS==3
	response +="<th class=\"thead\">Pkgs</th>";
	response +="<th class=\"thead\">Pkgs/hr</th>";
	response +="</tr>";

	//
	// Table rows
	//
	response +="<tr><td class=\"cell\">Packages Downlink</td>";
#	if _STATISTICS == 3
		response +="<td class=\"cell\">" + String(statc.msg_down_0) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_down_1) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_down_2) + "</td>"; 
#	endif
	response += "<td class=\"cell\">" + String(statc.msg_down) + "</td>";
	response +="<td class=\"cell\"></td></tr>";
		
	response +="<tr><td class=\"cell\">Packages Uplink Total</td>";
#	if	 _STATISTICS == 3
		response +="<td class=\"cell\">" + String(statc.msg_ttl_0) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_ttl_1) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_ttl_2) + "</td>";
#	endif //_STATISTICS==3
	response +="<td class=\"cell\">" + String(statc.msg_ttl) + "</td>";
	response +="<td class=\"cell\">" + String((statc.msg_ttl*3600)/(now() - startTime)) + "</td></tr>";

#	if _GATEWAYNODE==1
		response +="<tr><td class=\"cell\">Packages Internal Sensor</td>";
#		if	 _STATISTICS == 3
			response +="<td class=\"cell\">" + String(statc.msg_sens_0) + "</td>";
			response +="<td class=\"cell\">" + String(statc.msg_sens_1) + "</td>";
			response +="<td class=\"cell\">" + String(statc.msg_sens_2) + "</td>";
#		endif //_STATISTICS==3
		response +="<td class=\"cell\">" + String(statc.msg_sens) + "</td>";
		response +="<td class=\"cell\">" + String((statc.msg_sens*3600)/(now() - startTime)) + "</td></tr>";
#	endif //_GATEWAYNODE

	response +="<tr><td class=\"cell\">Packages Uplink OK </td>";
#if	 _STATISTICS == 3
		response +="<td class=\"cell\">" + String(statc.msg_ok_0) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_ok_1) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_ok_2) + "</td>";
#	endif //_STATISTICS==3
	response +="<td class=\"cell\">" + String(statc.msg_ok) + "</td>";
	response +="<td class=\"cell\">" + String((statc.msg_ok*3600)/(now() - startTime)) + "</td></tr>";
		

	// Provide a table with all the SF data including percentage of messsages
	#if _STATISTICS == 2
	response +="<tr><td class=\"cell\">SF7 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf7; 
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf7/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
	response +="<tr><td class=\"cell\">SF8 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf8;
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf8/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
	response +="<tr><td class=\"cell\">SF9 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf9;
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf9/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
	response +="<tr><td class=\"cell\">SF10 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf10; 
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf10/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
	response +="<tr><td class=\"cell\">SF11 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf11; 
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf11/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
	response +="<tr><td class=\"cell\">SF12 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf12; 
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf12/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
#	endif //_STATISTICS==2

#	if _STATISTICS == 3
	response +="<tr><td class=\"cell\">SF7 rcvd</td>";
		response +="<td class=\"cell\">"; response +=statc.sf7_0; 
		response +="<td class=\"cell\">"; response +=statc.sf7_1; 
		response +="<td class=\"cell\">"; response +=statc.sf7_2; 
		response +="<td class=\"cell\">"; response +=statc.sf7; 
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf7/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
		
	response +="<tr><td class=\"cell\">SF8 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf8_0;
		response +="<td class=\"cell\">"; response +=statc.sf8_1;
		response +="<td class=\"cell\">"; response +=statc.sf8_2;
		response +="<td class=\"cell\">"; response +=statc.sf8;
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf8/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
		
	response +="<tr><td class=\"cell\">SF9 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf9_0;
		response +="<td class=\"cell\">"; response +=statc.sf9_1;
		response +="<td class=\"cell\">"; response +=statc.sf9_2;
		response +="<td class=\"cell\">"; response +=statc.sf9;
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf9/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
		
	response +="<tr><td class=\"cell\">SF10 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf10_0; 
		response +="<td class=\"cell\">"; response +=statc.sf10_1; 
		response +="<td class=\"cell\">"; response +=statc.sf10_2; 
		response +="<td class=\"cell\">"; response +=statc.sf10; 
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf10/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
		
	response +="<tr><td class=\"cell\">SF11 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf11_0;
		response +="<td class=\"cell\">"; response +=statc.sf11_1; 
		response +="<td class=\"cell\">"; response +=statc.sf11_2; 
		response +="<td class=\"cell\">"; response +=statc.sf11; 
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf11/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
		
	response +="<tr><td class=\"cell\">SF12 rcvd</td>"; 
		response +="<td class=\"cell\">"; response +=statc.sf12_0;
		response +="<td class=\"cell\">"; response +=statc.sf12_1;
		response +="<td class=\"cell\">"; response +=statc.sf12_1;
		response +="<td class=\"cell\">"; response +=statc.sf12;		
		response +="<td class=\"cell\">"; response += String(statc.msg_ttl>0 ? 100*statc.sf12/statc.msg_ttl : 0)+" %"; 
		response +="</td></tr>";
#	endif //_STATISTICS==3

	response +="</table>";
	server.sendContent(response);
}




// --------------------------------------------------------------------------------
// Message History
// If enabled, display the sensor messageHistory on the current webserver Page.
// In this GUI section a number of statr[x] records are displayed such as:
//
// Time, The time the sensor message was received
// Node, the DevAddr or even Node name for Trusted nodes,
// Data (Localserver), when _LOCALSERVER is enabled contains decoded data
// C, Channel frequency on which the sensor was received
// Freq, The frequency of the channel
// SF, Spreading Factor
// pRSSI, Packet RSSI
//
// Parameters:
//	- <none>
// Returns:
//	- <none>
//
// As we make the TRUSTED_NODE a dynamic parameter, it can be set/unset in the user 
// interface. It will allow the user to only see known nodes (with a name) in the 
// list or also nodes with only an address (Unknown nodes).
// As a result, the size of the list will NOT be as large when only known nodes are 
// selected, as in can be deselcted in the GUI and we have only so much space on 
// th screen.
// --------------------------------------------------------------------------------
static void messageHistory() 
{
#if _STATISTICS >= 1
	String response="";

	// PRINT HEADERS
	response += "<h2>Message History</h2>";
	response += "<table class=\"config_table\">";
	response += "<tr>";
	response += "<th class=\"thead\">Time</th>";
	response += "<th class=\"thead\" style=\"width: 20px;\">Up/Dwn</th>";
	response += "<th class=\"thead\">Node</th>";
#if _LOCALSERVER>=1
	if (gwayConfig.showdata) {
		response += "<th class=\"thead\">Data</th>";
	}
#endif //_LOCALSERVER
	response += "<th class=\"thead\" style=\"width: 20px;\">C</th>";
	response += "<th class=\"thead\">Freq</th>";
	response += "<th class=\"thead\" style=\"width: 40px;\">SF</th>";
	response += "<th class=\"thead\" style=\"width: 50px;\">pRSSI</th>";
#if RSSI==1
	if (debug => 1) {
		response += "<th class=\"thead\" style=\"width: 50px;\">RSSI</th>";
	}
#endif

	// Print of Heads is ober.Now print all the rows
	response += "</tr>";
	server.sendContent(response);

	// PRINT NODE CONTENT
	for (int i=0; i< gwayConfig.maxStat; i++) {								// For every Node in the list
		if (statr[i].sf == 0) break;
		
		response = "" + String();
		
		response += "<tr><td class=\"cell\">";								// Tmst
		stringTime(statr[i].time, response);								// XXX Change tmst not to be millis() dependent
		response += "</td>";

		response += String() + "<td class=\"cell\">"; 						// Up or Downlink
		response += String(statr[i].upDown ? "v" : "^");
		response += "</td>";

		response += String() + "<td class=\"cell\">"; 						// Node
		
#ifdef  _TRUSTED_NODES														// DO nothing with TRUSTED NODES
		switch (gwayConfig.trusted) {
			case 0: 
				printHex(statr[i].node,' ',response); 
				break;
			case 1: 
				if (SerialName(statr[i].node, response) < 0) {
					printHex(statr[i].node,' ',response);
				};
				break;
			case 2: 
				if (SerialName(statr[i].node, response) < 0) {
					continue;
				};
				break;
			case 3: // Value and we do not print unless also defined for LOCAL_SERVER
			default:
#				if _MONITOR>=1
					mPrint("Unknow value for gwayConfig.trusted");
#				endif //_MONITOR		
				break;
		}
		
#else //_TRUSTED_NODES
		printHex(statr[i].node,' ',response);
#endif //_TRUSTED_NODES

		response += "</td>";
		
#if _LOCALSERVER>=1
	if (gwayConfig.showdata) {
		response += String() + "<td class=\"cell\">";						// Data
		if (statr[i].datal>24) statr[i].datal=24;							
		for (int j=0; j<statr[i].datal; j++) {
			if (statr[i].data[j] <0x10) response+= "0";
			response += String(statr[i].data[j],HEX) + " ";
		}
		response += "</td>";
	}
#endif //_LOCALSERVER


		response += String() + "<td class=\"cell\">" + statr[i].ch + "</td>";
		response += String() + "<td class=\"cell\">" + freqs[statr[i].ch].upFreq + "</td>";
		response += String() + "<td class=\"cell\">" + statr[i].sf + "</td>";

		response += String() + "<td class=\"cell\">" + statr[i].prssi + "</td>";
#if RSSI==1
		if (debug >= 2) {
			response += String() + "<td class=\"cell\">" + statr[i].rssi + "</td>";
		}
#endif
		response += "</tr>";
		server.sendContent(response);
	}
	
	server.sendContent("</table>");
	
#endif
}

// --------------------------------------------------------------------------------
// H2 NODE SEEN HISTORY
// If enabled, display the sensor last Seen history.
// This setting ,pves togetjer with the "expert" mode.
//  If that mode is enabled than the node seen intory is displayed
//
// Parameters:
//	- <none>
// Returns:
//	- <none>
// --------------------------------------------------------------------------------
static void nodeHistory() 
{
#	if _MAXSEEN>=1
	if (gwayConfig.seen) {
		// First draw the headers
		String response= "";
	
		response += "<h2>Node Last Seen History</h2>";
		response += "<table class=\"config_table\">";
		response += "<tr>";
		response += "<th class=\"thead\" style=\"width: 215px;\">Time</th>";
		response += "<th class=\"thead\" style=\"width: 20px;\">Up/Dwn</th>";
		response += "<th class=\"thead\">Node</th>";
		response += "<th class=\"thead\">Pkgs</th>";
		response += "<th class=\"thead\" style=\"width: 20px;\">C</th>";
		response += "<th class=\"thead\" style=\"width: 40px;\">SF</th>";

		response += "</tr>";
		server.sendContent(response);
		
		// Now start the contents
		
		for (int i=0; i<gwayConfig.maxSeen; i++) {
		
			if (listSeen[i].idSeen == 0)
			{
#				if _MONITOR>=1
				if ((debug>=2) && (pdebug & P_MAIN)) {
					mPrint("nodeHistory:: idSeen==0 for i="+String(i));
				}
#				endif //_MONITOR
				break;															// Continue loop
			}
			response = "";
		
			response += String("<tr><td class=\"cell\">");						// Tmst
			stringTime((listSeen[i].timSeen), response);
			response += "</td>";
			
			response += String("<td class=\"cell\">");							// upDown
			switch (listSeen[i].upDown) 
			{
				case 0: response += String("^"); break;
				case 1: response += String("v"); break;
				default: mPrint("wwwServer.ino:: ERROR upDown");
			}
			response += "</td>";
		
			response += String() + "<td class=\"cell\">"; 						// Node
#			ifdef  _TRUSTED_NODES												// Do nothing with TRUSTED NODES
				switch (gwayConfig.trusted) {
					case 0: 	
						printHex(listSeen[i].idSeen,' ',response);
							// Only print the HEX address, no names
						break;
					case 1: 
						if (SerialName(listSeen[i].idSeen, response) < 0) {
							// if no name found print HEX, else print name
							printHex(listSeen[i].idSeen,' ',response);
						};
						break;
					case 2: 
						if (SerialName(listSeen[i].idSeen, response) < 0) {
							// If name not found print nothing, else print name
							continue;
							//break;
						};
						break;
					case 3: 
						// Value 3 and we do not print unless also defined for LOCAL_SERVER
					default:
#						if _MONITOR>=1
							mPrint("Unknow value for gwayConfig.trusted");
#						endif //_MONITOR
					break;
				}	
#			else
				printHex(listSeen[i].idSeen,' ',response);
#			endif //_TRUSTED_NODES
			
			response += "</td>";
			
			response += String() + "<td class=\"cell\">" + listSeen[i].cntSeen + "</td>";			// Counter		
			response += String() + "<td class=\"cell\">" + listSeen[i].chnSeen + "</td>";								// Channel
			response += String() + "<td class=\"cell\">" + listSeen[i].sfSeen + "</td>";			// SF
			
			server.sendContent(response);
			//yield();
		}
		server.sendContent("</table>");
	}
#	endif //_MAXSEEN

	return;
} // nodeHistory()



// --------------------------------------------------------------------------------
// MONITOR DATA
// This is a cicular buffer that starts at the latest mPrint line and then goes
// down until there are no lines left (or we reach the limit).
// This function will print monitor data for the gateway based on the settings in
// _MONITOR in file configGway.h
//
// XXX We have to make the function such that when printed, the webpage refreshes.
// --------------------------------------------------------------------------------
void monitorData() 
{
#	if _MONITOR>=1
	if (gwayConfig.monitor) {
		String response="";
		response +="<h2>Monitoring Console</h2>";
	
		response +="<table class=\"config_table\">";
		response +="<tr>";
		response +="<th class=\"thead\">Monitor Console</th>";
		response +="</tr>";
		
		for (int i= iMoni-1+gwayConfig.maxMoni; i>=iMoni; i--) {
			if (monitor[i % gwayConfig.maxMoni].txt == "-") {	// If equal to init value '-'
				break;
			}
			response +="<tr><td class=\"cell\">" ;
			response += String(monitor[i % gwayConfig.maxMoni].txt);
			response += "</td></tr>";
		}
		
		response +="</table>";
		server.sendContent(response);
	}
#	endif //_MONITOR
}


// --------------------------------------------------------------------------------
// WIFI CONFIG
// wifiConfig() displays the most important Wifi parameters gathered
//
// --------------------------------------------------------------------------------
static void wifiConfig()
{
	if (gwayConfig.expert) {
		String response="";
		response +="<h2>WiFi Config</h2>";

		response +="<table class=\"config_table\">";

		response +="<tr><th class=\"thead\">Parameter</th><th class=\"thead\">Value</th></tr>";
	
		response +="<tr><td class=\"cell\">WiFi host</td><td class=\"cell\">"; 
#if defined(ESP32_ARCH)
		response +=WiFi.getHostname(); response+="</tr>";
#else
		response +=wifi_station_get_hostname(); response+="</tr>";
#endif

		response +="<tr><td class=\"cell\">WiFi SSID</td><td class=\"cell\">"; 
		response +=WiFi.SSID(); response+="</tr>";
	
		response +="<tr><td class=\"cell\">IP Address</td><td class=\"cell\">"; 
		printIP((IPAddress)WiFi.localIP(),'.',response); 
		response +="</tr>";
		response +="<tr><td class=\"cell\">IP Gateway</td><td class=\"cell\">"; 
		printIP((IPAddress)WiFi.gatewayIP(),'.',response); 
		response +="</tr>";
#ifdef _TTNSERVER
		response +="<tr><td class=\"cell\">NTP Server</td><td class=\"cell\">"; response+=NTP_TIMESERVER; response+="</tr>";
		response +="<tr><td class=\"cell\">LoRa Router</td><td class=\"cell\">"; response+=_TTNSERVER; response+="</tr>";
		response +="<tr><td class=\"cell\">LoRa Router IP</td><td class=\"cell\">"; 
		printIP((IPAddress)ttnServer,'.',response); 
		response +="</tr>";
#endif //_TTNSERVER
#ifdef _THINGSERVER
		response +="<tr><td class=\"cell\">LoRa Router 2</td><td class=\"cell\">"; response+=_THINGSERVER; 
		response += String() + ":" + _THINGPORT + "</tr>";
		response +="<tr><td class=\"cell\">LoRa Router 2 IP</td><td class=\"cell\">"; 
		printIP((IPAddress)thingServer,'.',response);
		response +="</tr>";
#endif //_THINGSERVER

		response +="</table>";

		server.sendContent(response);
	} // gwayConfig.expert
} // wifiConfig


// --------------------------------------------------------------------------------
// H2 systemStatus
// systemStatus is additional and only available in the expert mode.
// It provides a number of system specific data such as heap size etc.
// --------------------------------------------------------------------------------
static void systemStatus()
{
	if (gwayConfig.expert) {
		String response="";
		response +="<h2>System Status</h2>";
	
		response +="<table class=\"config_table\">";
		response +="<tr>";
		response +="<th class=\"thead\">Parameter</th>";
		response +="<th class=\"thead\" style=\"width:120px;\">Value</th>";
		response +="<th colspan=\"2\" class=\"thead width:120px;\">Set</th>";
		response +="</tr>";
	
		response +="<tr><td style=\"border: 1px solid black;\">Gateway ID</td>";
		response +="<td class=\"cell\">";	
		if (MAC_array[0]< 0x10) response +='0'; response +=String(MAC_array[0],HEX);	// The MAC array is always returned in lowercase
		if (MAC_array[1]< 0x10) response +='0'; response +=String(MAC_array[1],HEX);
		if (MAC_array[2]< 0x10) response +='0'; response +=String(MAC_array[2],HEX);
		response +="FFFF"; 
		if (MAC_array[3]< 0x10) response +='0'; response +=String(MAC_array[3],HEX);
		if (MAC_array[4]< 0x10) response +='0'; response +=String(MAC_array[4],HEX);
		if (MAC_array[5]< 0x10) response +='0'; response +=String(MAC_array[5],HEX);
		response+="</tr>";
	

		response +="<tr><td class=\"cell\">Free heap</td><td class=\"cell\">"; response+=ESP.getFreeHeap(); response+="</tr>";
// XXX We Should find an ESP32 alternative
#		if !defined ESP32_ARCH
			response +="<tr><td class=\"cell\">ESP speed</td><td class=\"cell\">"; response+=ESP.getCpuFreqMHz(); 
			response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"SPEED=80\"><button>80</button></a></td>";
			response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"SPEED=160\"><button>160</button></a></td>";
			response+="</tr>";
			response +="<tr><td class=\"cell\">ESP Chip ID</td><td class=\"cell\">"; response+=ESP.getChipId(); response+="</tr>";
#		endif
		response +="<tr><td class=\"cell\">OLED</td><td class=\"cell\">"; response+=_OLED; response+="</tr>";
		
#		if _STATISTICS >= 1
			response +="<tr><td class=\"cell\">WiFi Setups</td><td class=\"cell\">"; response+=gwayConfig.wifis; response+="</tr>";
			response +="<tr><td class=\"cell\">WWW Views</td><td class=\"cell\">"; response+=gwayConfig.views; response+="</tr>";
#		endif

		// Time Correction DELAY
		response +="<tr><td class=\"cell\">Time Correction (uSec)</td><td class=\"cell\">"; 
		response += gwayConfig.txDelay; 
		response +="</td>";
		response +="<td class=\"cell\"><a href=\"DELAY=-1\"><button>-</button></a></td>";
		response +="<td class=\"cell\"><a href=\"DELAY=1\"><button>+</button></a></td>";
		response +="</tr>";

		// Package Statistics List
		response +="<tr><td class=\"cell\">Package Statistics List</td><td class=\"cell\">"; 
		response += gwayConfig.maxStat; 
		response +="</td>";
		response +="<td class=\"cell\"><a href=\"MAXSTAT=-5\"><button>-</button></a></td>";
		response +="<td class=\"cell\"><a href=\"MAXSTAT=5\"><button>+</button></a></td>";
		response +="</tr>";

		// Last Seen Statistics
		response +="<tr><td class=\"cell\">Last Seen List</td><td class=\"cell\">"; 
		response += gwayConfig.maxSeen; 
		response +="</td>";
		response +="<td class=\"cell\"><a href=\"MAXSEEN-5\"><button>-</button></a></td>";
		response +="<td class=\"cell\"><a href=\"MAXSEEN+5\"><button>+</button></a></td>";
		response +="</tr>";

		// Reset Accesspoint
#		if _WIFIMANAGER==1
			response +="<tr><td><tr><td>";
			response +="Click <a href=\"/NEWSSID\">here</a> to reset accesspoint<br>";
			response +="</td><td></td></tr>";
#		endif //_WIFIMANAGER

#		if _UPDFIRMWARE==1
			// Update Firmware
			response +="<tr><td class=\"cell\">Update Firmware</td><td colspan=\"2\"></td>";
			response +="<td class=\"cell\" colspan=\"2\" class=\"cell\"><a href=\"/UPDATE=1\"><button>UPDATE</button></a></td></tr>";
#		endif //_UPDFIRMWARE

		response +="</table>";
		server.sendContent(response);
	} // gwayConfig.expert
} // systemStatus


// --------------------------------------------------------------------------------
// H2 System State and Interrupt
// Display interrupt data, but only for debug >= 2
//
// --------------------------------------------------------------------------------
static void interruptData()
{
	if (gwayConfig.expert) {
		uint8_t flags = readRegister(REG_IRQ_FLAGS);
		uint8_t mask = readRegister(REG_IRQ_FLAGS_MASK);
		String response="";
		
		response +="<h2>System State and Interrupt</h2>";
		
		response +="<table class=\"config_table\">";
		response +="<tr>";
		response +="<th class=\"thead\">Parameter</th>";
		response +="<th class=\"thead\">Value</th>";
		response +="<th colspan=\"2\"  class=\"thead\">Set</th>";
		response +="</tr>";
		
		response +="<tr><td class=\"cell\">_state</td>";
		response +="<td class=\"cell\">";
		switch (_state) {							// See loraModem.h
			case S_INIT: response +="INIT"; break;
			case S_SCAN: response +="SCAN"; break;
			case S_CAD: response +="CAD"; break;
			case S_RX: response +="RX"; break;
			case S_TX: response +="TX"; break;
			case S_TXDONE: response +="TXDONE"; break;
			default: response +="unknown"; break;
		}
		response +="</td></tr>";
		
		response +="<tr><td class=\"cell\">_STRICT_1CH</td>";
		response +="<td class=\"cell\">" ;
		response += String(_STRICT_1CH);
		response +="</td></tr>";		

		response +="<tr><td class=\"cell\">flags (8 bits)</td>";
		response +="<td class=\"cell\">0x";
		if (flags <16) response += "0";
		response +=String(flags,HEX); 
		response+="</td></tr>";

		
		response +="<tr><td class=\"cell\">mask (8 bits)</td>";
		response +="<td class=\"cell\">0x"; 
		if (mask <16) response += "0";
		response +=String(mask,HEX); 
		response+="</td></tr>";
		
		response +="<tr><td class=\"cell\">Re-entrant cntr</td>";
		response +="<td class=\"cell\">"; 
		response += String(gwayConfig.reents);
		response +="</td></tr>";

		response +="<tr><td class=\"cell\">ntp call cntr</td>";
		response +="<td class=\"cell\">"; 
		response += String(gwayConfig.ntps);
		response+="</td></tr>";
		
		response +="<tr><td class=\"cell\">ntpErr cntr</td>";
		response +="<td class=\"cell\">"; 
		response += String(gwayConfig.ntpErr);
		response +="</td>";
		response +="<td colspan=\"2\" style=\"border: 1px solid black;\">";
		stringTime(gwayConfig.ntpErrTime, response);
		response +="</td></tr>";

		response +="<tr><td class=\"cell\">loraWait errors/success</td>";
		response +="<td class=\"cell\">"; 
		response += String(gwayConfig.waitErr);
		response +="</td>";
		response +="<td colspan=\"2\" style=\"border: 1px solid black;\">";
		response += String(gwayConfig.waitOk);
		response +="</td></tr>";
		
		response +="</tr>";
		
		response +="</table>";
		
		server.sendContent(response);
	}// if gwayConfig.expert
} // interruptData



// --------------------------------------------------------------------------------
// setupWWW is the main function for webserver functions/
// SetupWWW function called by main setup() program to setup webserver
// It does actually not much more than installing all the callback handlers
// for messages sent to the webserver
//
// Implemented is an interface like:
// http://<server>/<Variable>=<value>
//
// --------------------------------------------------------------------------------
void setupWWW() 
{
	server.begin();									// Start the webserver

	server.on("/", []() {
		sendWebPage("","");							// Send the webPage string
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	
	// --------------------------------
	//     BUTTONS PART
	// --------------------------------
	// BUTTONS, define what should happen with the buttons we press on the homepage
	
	// Display Expert mode or Simple mode
	server.on("/EXPERT", []() {
		server.sendHeader("Location", String("/"), true);
		gwayConfig.expert = bool(1 - (int) gwayConfig.expert) ;
		server.send( 302, "text/plain", "");
	});

#	if _MONITOR>=1
	// Display Monitor Console or not
	server.on("/MONITOR", []() {
		server.sendHeader("Location", String("/"), true);
		gwayConfig.monitor = bool(1 - (int) gwayConfig.monitor) ;
		server.send( 302, "text/plain", "");
	});
#	endif //_MONITOR
	
	// Display the SEEN statistics
	server.on("/SEEN", []() {
		server.sendHeader("Location", String("/"), true);
		gwayConfig.seen = bool(1 - (int) gwayConfig.seen) ;
		server.send( 302, "text/plain", "");
	});

	// --------------------------------
	//     GATEWAY SETTINGS PART
	// --------------------------------
	
	server.on("/HELP", []() {
		sendWebPage("HELP","");					// Send the webPage string
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	// Set CAD function off/on
	server.on("/CAD=1", []() {
		gwayConfig.cad=(bool)1;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/CAD=0", []() {
		gwayConfig.cad=(bool)0;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});


	// Set debug parameter
	server.on("/DEBUG=-1", []() {				// Set debug level 0-2. Note: +3 is same as -1					
		debug = (debug+3)%4;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#		if _DUSB>=1 || _MONITOR>=1
		if ((debug>=1) && (pdebug & P_GUI)) {
			mPrint("DEBUG -1: config written");
		}
#		endif //_DUSB _MONITOR
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	
	server.on("/DEBUG=1", []() {
		debug = (debug+1)%4;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#		if _MONITOR>=1
		if (pdebug & P_GUI) {
			mPrint("DEBUG +1: config written");
		}
#		endif //_MONITOR
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});


	// Format the filesystem
	server.on("/FORMAT", []() {
		Serial.print(F("FORMAT ..."));
		msg_oLED("FORMAT");		
		SPIFFS.format();							// Normally not used. Use only when SPIFFS corrupt
		initConfig(&gwayConfig);					// Well known values
		gwayConfig.formatCntr++;
		writeConfig(_CONFIGFILE, &gwayConfig);
		printSeen( _SEENFILE, listSeen);			// Write the last time record  is Seen
#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_MAIN )) {
			mPrint("www:: manual Format DONE");
		}
#		endif //_MONITOR
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	
	
	// Reset the statistics (the gateway function) but not the system specific things
	server.on("/RESET", []() {
		mPrint("RESET");
		startTime= now() - 1;					// Reset all timers too (-1 to avoid division by 0)
		
		statc.msg_ttl = 0;						// Reset ALL package statistics
		statc.msg_ok = 0;
		statc.msg_down = 0;
		statc.msg_sens = 0;
		
#if _STATISTICS >= 3
		statc.msg_ttl_0 = 0;
		statc.msg_ttl_1 = 0;
		statc.msg_ttl_2 = 0;
		
		statc.msg_ok_0 = 0;
		statc.msg_ok_1 = 0;
		statc.msg_ok_2 = 0;
		
		statc.msg_down_0 = 0;
		statc.msg_down_1 = 0;
		statc.msg_down_2 = 0;

		statc.msg_sens_0 = 0;
		statc.msg_sens_1 = 0;
		statc.msg_sens_2 = 0;
#endif

#	if _STATISTICS >= 1
		for (int i=0; i< gwayConfig.maxStat; i++) { statr[i].sf = 0; }
#		if _STATISTICS >= 2
			statc.sf7 = 0;
			statc.sf8 = 0;
			statc.sf9 = 0;
			statc.sf10= 0;
			statc.sf11= 0;
			statc.sf12= 0;
		
			statc.resets= 0;
			
#			if _STATISTICS >= 3
				statc.sf7_0 = 0; statc.sf7_1 = 0; statc.sf7_2 = 0;
				statc.sf8_0 = 0; statc.sf8_1 = 0; statc.sf8_2 = 0;
				statc.sf9_0 = 0; statc.sf9_1 = 0; statc.sf9_2 = 0;
				statc.sf10_0= 0; statc.sf10_1= 0; statc.sf10_2= 0;
				statc.sf11_0= 0; statc.sf11_1= 0; statc.sf11_2= 0;
				statc.sf12_0= 0; statc.sf12_1= 0; statc.sf12_2= 0;
#			endif //_STATISTICS==3
			
#		endif //_STATISTICS==2
#	endif //_STATISTICS==1

		writeGwayCfg(_CONFIGFILE, &gwayConfig );			
		initSeen(listSeen);						// Clear all Seen records as well.
		
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	// Reset the boot counter, and other system specific counters
	server.on("/BOOT", []() {
		mPrint("BOOT");
#if _STATISTICS >= 2		
		gwayConfig.wifis = 0;
		gwayConfig.views = 0;
		gwayConfig.ntpErr = 0;					// NTP errors
		gwayConfig.ntpErrTime = 0;				// NTP last error time
		gwayConfig.ntps = 0;					// Number of NTP calls
#endif
		gwayConfig.boots = 0;					//
		gwayConfig.reents = 0;					// Re-entrance
		
		writeGwayCfg(_CONFIGFILE, &gwayConfig );
#if		_MONITOR>=1
		if ((debug>=2) && (pdebug & P_GUI)) {
			mPrint("wwwServer:: BOOT: config written");
		}
#		endif //_MONITOR
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	// Reboot the Gateway
	// NOTE: There is no button for this code yet
	server.on("/REBOOT", []() {
		sendWebPage("",""); 					// Send the webPage string
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
		ESP.restart();
	});

	// New SSID for the machine
	server.on("/NEWSSID", []() {
		sendWebPage("NEWSSID","");				// Send the webPage string
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});



	// Set PDEBUG parameter
	//
	server.on("/PDEBUG=SCAN", []() {			// Set debug level 0x01						
		pdebug ^= P_SCAN;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/PDEBUG=CAD", []() {				// Set debug level 0x02						
		pdebug ^= P_CAD;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/PDEBUG=RX", []() {				// Set debug level 0x04						
		pdebug ^= P_RX;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/PDEBUG=TX", []() {				// Set debug level 0x08						
		pdebug ^= P_TX;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/PDEBUG=PRE", []() {				// Set debug level 0-2						
		pdebug ^= P_PRE;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/PDEBUG=MAIN", []() {				// Set debug level 0-2						
		pdebug ^= P_MAIN;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/PDEBUG=GUI", []() {				// Set debug level 0-2						
		pdebug ^= P_GUI;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/PDEBUG=RADIO", []() {			// Set debug level 0-2						
		pdebug ^= P_RADIO;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	
	// Set delay in microseconds
	server.on("/DELAY=1", []() {
		gwayConfig.txDelay+=5000;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_GUI)) {
			mPrint("DELAY +, config written");
		}
#		endif //_MONITOR
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/DELAY=-1", []() {
		gwayConfig.txDelay-=5000;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_GUI)) {
			mPrint("DELAY -, config written");
		}
#		endif //_MONITOR
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	// Set Trusted Node Parameter
	server.on("/TRUSTED=1", []() {
	gwayConfig.trusted = (gwayConfig.trusted +1)%4;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#		if _MONITOR>=1
			mPrint("TRUSTED +, config written");
#		endif //_MONITOR
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/TRUSTED=-1", []() {
		gwayConfig.trusted = (gwayConfig.trusted -1)%4;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#		if _MONITOR>=1
			mPrint("TRUSTED -, config written");
#		endif //_MONITOR
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	// Spreading Factor setting
	server.on("/SF=1", []() {
		if (sf>=SF12) sf=SF7; else sf= (sf_t)((int)sf+1);
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/SF=-1", []() {
		if (sf<=SF7) sf=SF12; else sf= (sf_t)((int)sf-1);
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	// Set Frequency of the GateWay node
	server.on("/FREQ=1", []() {
		uint8_t nf = sizeof(freqs)/sizeof(freqs[0]);	// Number of elements in array
#if _DUSB>=2
		Serial.print("FREQ==1:: For freq[0] sizeof vector=");
		Serial.print(sizeof(freqs[0]));
		Serial.println();
#endif
		if (gwayConfig.ch==(nf-1)) gwayConfig.ch=0; else gwayConfig.ch++;
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/FREQ=-1", []() {
		uint8_t nf = sizeof(freqs)/sizeof(freqs[0]);	// Number of elements in array
		if (gwayConfig.ch==0) gwayConfig.ch=(nf-1); else gwayConfig.ch--;
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});


	// GatewayNode
	server.on("/NODE=1", []() {
#if _GATEWAYNODE==1
		gwayConfig.isNode =(bool)1;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#endif //_GATEWAYNODE
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/NODE=0", []() {
#if _GATEWAYNODE==1
		gwayConfig.isNode =(bool)0;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#endif //_GATEWAYNODE
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

#if _GATEWAYNODE==1	
	// Framecounter of the Gateway node
	server.on("/FCNT", []() {

		frameCount=0; 
		rxLoraModem();							// Reset the radio with the new frequency
		writeGwayCfg(_CONFIGFILE, &gwayConfig );

		//sendWebPage("","");						// Send the webPage string
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
#endif

	// WWW Page refresh function
	server.on("/REFR=1", []() {					// WWW page auto refresh ON
#if _REFRESH==1
		gwayConfig.refresh =1;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#endif		
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/REFR=0", []() {					// WWW page auto refresh OFF
#if _REFRESH==1
		gwayConfig.refresh =0;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
#endif
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	// WWW Page serial print function
	server.on("/DUSB=1", []() {					// WWW page Serial Print ON
		gwayConfig.dusbStat =1;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/DUSB=0", []() {					// WWW page Serial Print OFF
		gwayConfig.dusbStat =0;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	// SHOWDATA, write node trusted value 
	server.on("/SHOWDATA=1", []() {					// WWW page Serial Print ON
		gwayConfig.showdata =1;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/SHOWDATA=0", []() {					// WWW page Serial Print OFF
		gwayConfig.showdata =0;
		writeGwayCfg(_CONFIGFILE, &gwayConfig );	// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	
	// Switch off/on the HOP functions
	server.on("/HOP=1", []() {
		gwayConfig.hop=true;
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/HOP=0", []() {
		gwayConfig.hop=false;
		setFreq(freqs[gwayConfig.ch].upFreq);
		rxLoraModem();
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

#if !defined ESP32_ARCH
	// Change speed to 160 MHz
	server.on("/SPEED=80", []() {
		system_update_cpu_freq(80);
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/SPEED=160", []() {
		system_update_cpu_freq(160);
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
#endif
	// Display Documentation pages
	server.on("/DOCU", []() {

		server.sendHeader("Location", String("/"), true);
		buttonDocu();
		server.send( 302, "text/plain", "");
	});

	// Display LOGging information
	server.on("/LOG", []() {
		server.sendHeader("Location", String("/"), true);
#		if _MONITOR>=1
			mPrint("LOG button");
#		endif //_MONITOR
		buttonLog();
		server.send( 302, "text/plain", "");
	});


	// Update the sketch. Not yet implemented
	server.on("/UPDATE=1", []() {
		// Future work
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});


	// ----------------------------
	//    SYSTEM STATUS PART    
	// ---------------------------- 
	server.on("/MAXSTAT=-5", []() {
		if (gwayConfig.maxStat>5) {
			struct stat_t * oldStat = statr;
			gwayConfig.maxStat-=5;
			statr = (struct stat_t *) malloc(gwayConfig.maxStat * sizeof(struct stat_t));
			for (int i=0; i<gwayConfig.maxStat; i+=1) {
				statr[i]=oldStat[i];
			}
			free(oldStat);
		}
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/MAXSTAT=5", []() {
		struct stat_t * oldStat = statr;
		gwayConfig.maxStat+=5;
		statr = (struct stat_t *) malloc(gwayConfig.maxStat * sizeof(struct stat_t));
		for (int i=0; i<gwayConfig.maxStat; i+=1) {
			if (i<(gwayConfig.maxStat-5)) {
				statr[i]=oldStat[i];
			}
			else {
				statr[i].sf=0;
			}
		}
		free(oldStat);
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	server.on("/MAXSEEN-5", []() {
		if (gwayConfig.maxSeen>5) {
			struct nodeSeen * oldSeen = listSeen;
			gwayConfig.maxSeen-=5;
			listSeen = (struct nodeSeen *) malloc(gwayConfig.maxSeen * sizeof(struct nodeSeen));
			for (int i=0; i<gwayConfig.maxSeen; i+=1) {
				listSeen[i]=oldSeen[i];
			}
			if (gwayConfig.maxSeen<iSeen) iSeen = gwayConfig.maxSeen;			
			free(oldSeen);
		}
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});
	server.on("/MAXSEEN+5", []() {
		struct nodeSeen * oldSeen = listSeen;
		gwayConfig.maxSeen+=5;
		listSeen = (struct nodeSeen *) malloc(gwayConfig.maxSeen * sizeof(struct nodeSeen));
		for (int i=0; i<gwayConfig.maxSeen; i+=1) {
			if (i<(gwayConfig.maxSeen-5)) listSeen[i]=oldSeen[i];
			else listSeen[i].idSeen=0;
		}
		free(oldSeen);
		server.sendHeader("Location", String("/"), true);
		server.send( 302, "text/plain", "");
	});

	// -----------
	// This section from version 4.0.7 defines what PART of the
	// webpage is shown based on the buttons pressed by the user
	// Maybe not all information should be put on the screen since it
	// may take too much time to serve all information before a next
	// package interrupt arrives at the gateway
#	if _MONITOR>=1
		mPrint("WWW Server started on port " + String(_SERVERPORT) );
#	endif //_MONITOR

	return;
} // setupWWW

// --------------------------------------------------------------------------------
// SEND WEB PAGE() 
// Call the webserver and send the standard content and the content that is 
// passed by the parameter. Each time a variable is changed, this function is 
// called to display the webpage again/
//
// NOTE: This is the only place where yield() or delay() calls are used.
//
// --------------------------------------------------------------------------------
void sendWebPage(const char *cmd, const char *arg)
{
	openWebPage(); yield();						// Do the initial website setup
	
	wwwButtons();								// Display buttons such as Documentation, Mode, Logfiles
	
	setVariables(cmd,arg); yield();				// Read Webserver commands from line

	statisticsData(); yield();		 			// Node statistics
	messageHistory(); yield();					// Display the sensor history, message statistics
	nodeHistory(); yield();						// Display the lastSeen array
	monitorData(); yield();						// Console
	
	gatewaySettings(); yield();					// Display web configuration
	wifiConfig(); yield();						// WiFi specific parameters
	systemStatus(); yield();					// System statistics such as heap etc.
	interruptData(); yield();					// Display interrupts only when debug >= 2

	websiteFooter(); yield();
	
	server.client().stop();
}


// --------------------------------------------------------------------------------
// websiteFooter
// 
// Thi function displays the last messages without header on the webpage and then
// closes the webpage.
// --------------------------------------------------------------------------------
static void websiteFooter()
{
	// Close the client connection to server
	server.sendContent(String() + "<br><br /><p style='font-size:10px'>Click <a href=\"/HELP\">here</a> to explain Help and REST options</p><br>");
	server.sendContent(String() + "</BODY></HTML>");
	
	server.sendContent(""); yield();
}


#endif //_SERVER==1

