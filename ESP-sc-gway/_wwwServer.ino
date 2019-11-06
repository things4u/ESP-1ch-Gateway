// 1-channel LoRa Gateway for ESP8266
// Copyright (c) 2016, 2017, 2018, 2019 Maarten Westenberg version for ESP8266
// Version 6.1.1
// Date: 2019-11-06
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


// --------------------------------------------------------------------------------
// PRINT IP
// Output the 4-byte IP address for easy printing.
// As this function is also used by _otaServer.ino do not put in #define
// --------------------------------------------------------------------------------
static void printIP(IPAddress ipa, const char sep, String& response)
{
	response+=(IPAddress)ipa[0]; response+=sep;
	response+=(IPAddress)ipa[1]; response+=sep;
	response+=(IPAddress)ipa[2]; response+=sep;
	response+=(IPAddress)ipa[3];
}

//
// The remainder of the file ONLY works is A_SERVER=1 is set.
//
#if A_SERVER==1

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
// Paramters of the JavaScript function:
//	s: Th strig contining the question to be answered
//	o: The OK tab for the webpage where to go to
//	c: The Cancel string (optional)
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
	
// Put something like this in the ESP program
//	response += "<input type=\"button\" value=\"YesNo\" onclick=\"ynDialog()\" />";
	
	return(ret);
}


// --------------------------------------------------------------------------------
// WWWFILE
// This function will open a pop-up in the browser and then 
//	display the contents of a file in that window
// Output is sent to server.sendContent()
//	Note: The output is not in a variable, its size would be too large
// Parameters:
//		fn; String with filename
// Returns:
//		<none>
// --------------------------------------------------------------------------------
void wwwFile(String fn) {

	if (!SPIFFS.exists(fn)) {
#if _DUSB>=1
		Serial.print(F("wwwFile:: ERROR: file not found="));
		Serial.println(fn);
#endif
		return;
	}
#if _DUSB>=2
	else {
		Serial.print(F("wwwFile:: File existist= "));
		Serial.println(fn);
	}
#endif

#if _DUSB>=1
	File f = SPIFFS.open(fn, "r");					// Open the file for reading
		
	int j;
	for (j=0; j<LOGFILEREC; j++) {
			
		String s=f.readStringUntil('\n');
		if (s.length() == 0) {
			Serial.print(F("wwwFile:: String length 0"));
			break;
		}
		server.sendContent(s.substring(12));		// Skip the first 12 Gateway specific binary characters
		server.sendContent("\n");
		yield();
	}
	
	f.close();

#endif
	
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
	response += "      document.location.href = \"https://things4u.github.io/Projects/SingleChannelGateway/UserGuide/Introduction%206.html\"; ";
	response += "    }";
	response += "  }";
	response += "}";
	
	response += "</script>";
	server.sendContent(response);
}




// --------------------------------------------------------------------------------
// Button function Log displays  logfiles.
// This is a button on the top of the GUI screen
// --------------------------------------------------------------------------------
void buttonLog() 
{
	
	String response = "";
	String fn = "";
	int i = 0;
	
	while (i< LOGFILEMAX ) {
		fn = "/log-" + String(gwayConfig.logFileNo - i);
		wwwFile(fn);									// Display the file contents in the browser
		i++;
	}
	
	server.sendContent(response);
}

// --------------------------------------------------------------------------------
// BUTTONNODE
// Read the logfiles and display info about nodes (last seend, SF used etc).
// This is a button on the top of the GUI screen
// --------------------------------------------------------------------------------
void buttonNode() 
{
	
//	String response = "";
	String fn = "";
	int i = 0;
	
	while (i< LOGFILEMAX ) {
		fn = "/log-" + String(gwayConfig.logFileNo - i);
		wwwFile(fn);									// Display the file contents in the browser
		i++;
	}
	
//	server.sendContent(response);
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

	YesNo();												// Init the Yes/No function
	buttonDocu();

	response += "<input type=\"button\" value=\"Documentation\" onclick=\"showDocu()\" >";
	
	response += "<a href=\"EXPERT\" download><button type=\"button\">" + mode + "</button></a>";
	response += "<a href=\"NODE\" download><button type=\"button\">Nodes Seen</button></a>";
	response += "<a href=\"LOG\" download><button type=\"button\">Log Files</button></a>";

	server.sendContent(response);							// Send to the screen
}


// --------------------------------------------------------------------------------
// SET ESP8266 WEB SERVER VARIABLES
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
static void setVariables(const char *cmd, const char *arg) {

	// DEBUG settings; These can be used as a single argument
	if (strcmp(cmd, "DEBUG")==0) {									// Set debug level 0-2
		if (atoi(arg) == 1) {
			debug = (debug+1)%4;
		}	
		else if (atoi(arg) == -1) {
			debug = (debug+3)%4;
		}
		writeGwayCfg(CONFIGFILE);									// Save configuration to file
	}
	
	if (strcmp(cmd, "CAD")==0) {									// Set -cad on=1 or off=0
		_cad=(bool)atoi(arg);
		writeGwayCfg(CONFIGFILE);									// Save configuration to file
	}
	
	if (strcmp(cmd, "HOP")==0) {									// Set -hop on=1 or off=0
		_hop=(bool)atoi(arg);
		if (! _hop) { 
			ifreq=0; 
			setFreq(freqs[ifreq].upFreq);			
			rxLoraModem();
			sf = SF7;
			cadScanner();
		}
		writeGwayCfg(CONFIGFILE);									// Save configuration to file
	}
	
	if (strcmp(cmd, "DELAY")==0) {									// Set delay usecs
		txDelay+=atoi(arg)*1000;
	}
	
	// SF; Handle Spreading Factor Settings
	if (strcmp(cmd, "SF")==0) {
		uint8_t sfi = sf;
		if (atoi(arg) == 1) {
			if (sf>=SF12) sf=SF7; else sf= (sf_t)((int)sf+1);
		}	
		else if (atoi(arg) == -1) {
			if (sf<=SF7) sf=SF12; else sf= (sf_t)((int)sf-1);
		}
		rxLoraModem();												// Reset the radio with the new spreading factor
		writeGwayCfg(CONFIGFILE);									// Save configuration to file
	}
	
	// FREQ; Handle Frequency  Settings
	if (strcmp(cmd, "FREQ")==0) {
		uint8_t nf = sizeof(freqs)/ sizeof(freqs[0]);				// Number of frequency elements in array
		
		// Compute frequency index
		if (atoi(arg) == 1) {
			if (ifreq==(nf-1)) ifreq=0; else ifreq++;
		}
		else if (atoi(arg) == -1) {
			Serial.println("down");
			if (ifreq==0) ifreq=(nf-1); else ifreq--;
		}
		setFreq(freqs[ifreq].upFreq);
		rxLoraModem();												// Reset the radio with the new frequency
		writeGwayCfg(CONFIGFILE);									// Save configuration to file
	}

	//if (strcmp(cmd, "GETTIME")==0) { Serial.println(F("gettime tbd")); }	// Get the local time
	
	//if (strcmp(cmd, "SETTIME")==0) { Serial.println(F("settime tbd")); }	// Set the local time
	
	if (strcmp(cmd, "HELP")==0)    { Serial.println(F("Display Help Topics")); }
	
#if GATEWAYNODE==1
	if (strcmp(cmd, "NODE")==0) {									// Set node on=1 or off=0
		gwayConfig.isNode =(bool)atoi(arg);
		writeGwayCfg(CONFIGFILE);									// Save configuration to file
	}
	
	if (strcmp(cmd, "FCNT")==0)   { 
		frameCount=0; 
		rxLoraModem();												// Reset the radio with the new frequency
		writeGwayCfg(CONFIGFILE);
	}
#endif
	
#if _WIFIMANAGER==1
	if (strcmp(cmd, "NEWSSID")==0) { 
		WiFiManager wifiManager;
		strcpy(wpa[0].login,""); 
		strcpy(wpa[0].passw,"");
		WiFi.disconnect();
		wifiManager.autoConnect(AP_NAME, AP_PASSWD );
	}
#endif

#if A_OTA==1
	if (strcmp(cmd, "UPDATE")==0) {
		if (atoi(arg) == 1) {
			updateOtaa();
		}
	}
#endif

#if A_REFRESH==1
	if (strcmp(cmd, "REFR")==0) {									// Set refresh on=1 or off=0
		gwayConfig.refresh =(bool)atoi(arg);
		writeGwayCfg(CONFIGFILE);									// Save configuration to file
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
	++gwayConfig.views;									// increment number of views
#if A_REFRESH==1
	//server.client().stop();							// Experimental, stop webserver in case something is still running!
#endif
	String response="";	

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	
	// init webserver, fill the webpage
	// NOTE: The page is renewed every _WWW_INTERVAL seconds, please adjust in configGway.h
	//
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text/html", "");
#if A_REFRESH==1
	if (gwayConfig.refresh) {
		response += String() + "<!DOCTYPE HTML><HTML><HEAD><meta http-equiv='refresh' content='"+_WWW_INTERVAL+";http://";
		printIP((IPAddress)WiFi.localIP(),'.',response);
		response += "'><TITLE>ESP8266 1ch Gateway</TITLE>";
	}
	else {
		response += String() + "<!DOCTYPE HTML><HTML><HEAD><TITLE>ESP8266 1ch Gateway</TITLE>";
	}
#else
	response += String() + "<!DOCTYPE HTML><HTML><HEAD><TITLE>ESP8266 1ch Gateway</TITLE>";
#endif
	response += "<META HTTP-EQUIV='CONTENT-TYPE' CONTENT='text/html; charset=UTF-8'>";
	response += "<META NAME='AUTHOR' CONTENT='M. Westenberg (mw1554@hotmail.com)'>";

	response += "<style>.thead {background-color:green; color:white;} ";
	response += ".cell {border: 1px solid black;}";
	response += ".config_table {max_width:100%; min-width:400px; width:98%; border:1px solid black; border-collapse:collapse;}";
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
	response += String() + days + "-";
	if (_hour < 10) response += "0";
	response += String() + _hour + ":";
	if (_minute < 10) response += "0";
	response += String() + _minute + ":";
	if (_second < 10) response += "0";
	response += String() + _second;
	
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
	response +="<th colspan=\"4\" style=\"background-color: green; color: white; width:100px;\">Set</th>";
	response +="</tr>";
	
	bg = " background-color: ";
	bg += ( _cad ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">CAD</td>";
	response +="<td colspan=\"2\" style=\"border: 1px solid black;"; response += bg; response += "\">";
	response += ( _cad ? "ON" : "OFF" );
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"CAD=1\"><button>ON</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"CAD=0\"><button>OFF</button></a></td>";
	response +="</tr>";
	
	bg = " background-color: ";
	bg += ( _hop ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">HOP</td>";
	response +="<td colspan=\"2\" style=\"border: 1px solid black;"; response += bg; response += "\">";
	response += ( _hop ? "ON" : "OFF" );
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"HOP=1\"><button>ON</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"HOP=0\"><button>OFF</button></a></td>";
	response +="</tr>";
	
	response +="<tr><td class=\"cell\">SF Setting</td><td class=\"cell\" colspan=\"2\">";
	if (_cad) {
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
	if (_hop) {
		response += "AUTO</td>";
	}
	else {
		response += String() + ifreq; 
		response +="</td>";
		response +="<td class=\"cell\"><a href=\"FREQ=-1\"><button>-</button></a></td>";
		response +="<td class=\"cell\"><a href=\"FREQ=1\"><button>+</button></a></td>";
	}
	response +="</tr>";

	// Debugging options, only when _DUSB is set, otherwise no
	// serial activity
#if _DUSB>=1	
	response +="<tr><td class=\"cell\">Debug level</td><td class=\"cell\" colspan=\"2\">"; 
	response +=debug; 
	response +="</td>";
	response +="<td class=\"cell\"><a href=\"DEBUG=-1\"><button>-</button></a></td>";
	response +="<td class=\"cell\"><a href=\"DEBUG=1\"><button>+</button></a></td>";
	response +="</tr>";
	
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
	// Serial Debugging
	response +="<tr><td class=\"cell\">Usb Debug</td><td class=\"cell\" colspan=\"2\">"; 
	response += _DUSB; 
	response +="</td>";
	//response +="<td class=\"cell\"> </td>";
	//response +="<td class=\"cell\"> </td>";
	response +="</tr>";
	
#if GATEWAYNODE==1
	response +="<tr><td class=\"cell\">Framecounter Internal Sensor</td>";
	response +="<td class=\"cell\" colspan=\"2\">";
	response +=frameCount;
	response +="</td><td colspan=\"2\" style=\"border: 1px solid black;\">";
	response +="<button><a href=\"/FCNT\">RESET</a></button></td>";
	response +="</tr>";
	
	bg = " background-color: ";
	bg += ( (gwayConfig.isNode == 1) ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">Gateway Node</td>";
	response +="<td class=\"cell\" style=\"border: 1px solid black;" + bg + "\">";
	response += ( (gwayConfig.isNode == true) ? "ON" : "OFF" );
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"NODE=1\"><button>ON</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"NODE=0\"><button>OFF</button></a></td>";
	response +="</tr>";
#endif

#if A_REFRESH==1
	bg = " background-color: ";
	bg += ( (gwayConfig.refresh == 1) ? "LightGreen" : "orange" );
	response +="<tr><td class=\"cell\">WWW Refresh</td>";
	response +="<td class=\"cell\" colspan=\"2\" style=\"border: 1px solid black; " + bg + "\">";
	response += ( (gwayConfig.refresh == 1) ? "ON" : "OFF" );
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"REFR=1\"><button>ON</button></a></td>";
	response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"REFR=0\"><button>OFF</button></a></td>";
	response +="</tr>";
#endif

	// Reset Accesspoint
#if _WIFIMANAGER==1
	response +="<tr><td><tr><td>";
	response +="Click <a href=\"/NEWSSID\">here</a> to reset accesspoint<br>";
	response +="</td><td></td></tr>";
#endif

	// Update Firmware all statistics
	response +="<tr><td class=\"cell\">Update Firmware</td><td colspan=\"2\"></td>";
	response +="<td class=\"cell\" colspan=\"2\" class=\"cell\"><a href=\"/UPDATE=1\"><button>UPDATE</button></a></td></tr>";

	// Format the Filesystem
	response +="<tr><td class=\"cell\">Format SPIFFS</td>";
	response +=String() + "<td class=\"cell\" colspan=\"2\" >"+""+"</td>";
	response +="<td colspan=\"2\" class=\"cell\"><input type=\"button\" value=\"FORMAT\" onclick=\"ynDialog(\'Do you really want to format?\',\'FORMAT\')\" /></td></tr>";

	// Reset all statistics
#if _STATISTICS >= 1
	response +="<tr><td class=\"cell\">Statistics</td>";
	response +=String() + "<td class=\"cell\" colspan=\"2\" >"+statc.resets+"</td>";
	response +="<td colspan=\"2\" class=\"cell\"><input type=\"button\" value=\"RESET\" onclick=\"ynDialog(\'Do you really want to reset statistics?\',\'RESET\')\" /></td></tr>";

	// Reset
	response +="<tr><td class=\"cell\">Boots and Resets</td>";
	response +=String() + "<td class=\"cell\" colspan=\"2\" >"+gwayConfig.boots+"</td>";
	response +="<td colspan=\"2\" class=\"cell\"><input type=\"button\" value=\"RESET\" onclick=\"ynDialog(\'Do you want to reset boots?\',\'BOOT\')\" /></td></tr>";
#endif
	
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

	// Header
	response +="<h2>Package Statistics</h2>";
	response +="<table class=\"config_table\">";
	response +="<tr><th class=\"thead\">Counter</th>";
#if _STATISTICS == 3
	response +="<th class=\"thead\">C 0</th>";
	response +="<th class=\"thead\">C 1</th>";
	response +="<th class=\"thead\">C 2</th>";
#endif
	response +="<th class=\"thead\">Pkgs</th>";
	response +="<th class=\"thead\">Pkgs/hr</th>";
	response +="</tr>";

	//
	// Table rows
	//
	response +="<tr><td class=\"cell\">Packages Downlink</td>";
#if _STATISTICS == 3
		response +="<td class=\"cell\">" + String(statc.msg_down_0) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_down_1) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_down_2) + "</td>"; 
#endif
	response += "<td class=\"cell\">" + String(statc.msg_down) + "</td>";
	response +="<td class=\"cell\"></td></tr>";
		
	response +="<tr><td class=\"cell\">Packages Uplink Total</td>";
#if _STATISTICS == 3
		response +="<td class=\"cell\">" + String(statc.msg_ttl_0) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_ttl_1) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_ttl_2) + "</td>";
#endif
		response +="<td class=\"cell\">" + String(statc.msg_ttl) + "</td>";
		response +="<td class=\"cell\">" + String((statc.msg_ttl*3600)/(now() - startTime)) + "</td></tr>";
		
	response +="<tr><td class=\"cell\">Packages Uplink OK </td>";
#if _STATISTICS == 3
		response +="<td class=\"cell\">" + String(statc.msg_ok_0) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_ok_1) + "</td>";
		response +="<td class=\"cell\">" + String(statc.msg_ok_2) + "</td>";
#endif
	response +="<td class=\"cell\">" + String(statc.msg_ok) + "</td>";
	response +="<td class=\"cell\"></td></tr>";
		

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
#endif
#if _STATISTICS == 3
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
#endif

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
// --------------------------------------------------------------------------------
static void messageHistory() 
{
#if _STATISTICS >= 1
	String response="";
	
	response += "<h2>Message History</h2>";
	response += "<table class=\"config_table\">";
	response += "<tr>";
	response += "<th class=\"thead\">Time</th>";
	response += "<th class=\"thead\">Node</th>";
#if _LOCALSERVER==1
	response += "<th class=\"thead\">Data</th>";
#endif
	response += "<th class=\"thead\" style=\"width: 20px;\">C</th>";
	response += "<th class=\"thead\">Freq</th>";
	response += "<th class=\"thead\" style=\"width: 40px;\">SF</th>";
	response += "<th class=\"thead\" style=\"width: 50px;\">pRSSI</th>";
#if RSSI==1
	if (debug > 1) {
		response += "<th class=\"thead\" style=\"width: 50px;\">RSSI</th>";
	}
#endif
	response += "</tr>";
	server.sendContent(response);

	for (int i=0; i<MAX_STAT; i++) {
		if (statr[i].sf == 0) break;
		
		response = "";
		
		response += String() + "<tr><td class=\"cell\">";					// Tmst
		stringTime((statr[i].tmst), response);			// XXX Change tmst not to be millis() dependent
		response += "</td>";
		
		response += String() + "<td class=\"cell\">"; 						// Node
		if (SerialName((char *)(& (statr[i].node)), response) < 0) {		// works with TRUSTED_NODES >= 1
			printHEX((char *)(& (statr[i].node)),' ',response);				// else
		}
		response += "</td>";
		
#if _LOCALSERVER==1
		response += String() + "<td class=\"cell\">";						// Data
		for (int j=0; j<statr[i].datal; j++) {
			if (statr[i].data[j] <0x10) response+= "0";
			response += String(statr[i].data[j],HEX) + " ";
		}
		response += "</td>";
#endif


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

	gatewaySettings(); yield();					// Display web configuration
	wifiConfig(); yield();						// WiFi specific parameters
	
	systemStatus(); yield();					// System statistics such as heap etc.
	interruptData(); yield();					// Display interrupts only when debug >= 2
	
	websiteFooter(); yield();
	

	
	server.client().stop();
}


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
	
	// -----------------
	// BUTTONS, define what should happen with the buttons we press on the homepage
	
	server.on("/", []() {
		sendWebPage("","");							// Send the webPage string
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	
	server.on("/HELP", []() {
		sendWebPage("HELP","");					// Send the webPage string
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	// Format the filesystem
	server.on("/FORMAT", []() {
		Serial.print(F("FORMAT ..."));
		
		SPIFFS.format();								// Normally disabled. Enable only when SPIFFS corrupt
		initConfig(&gwayConfig);
		writeConfig( CONFIGFILE, &gwayConfig);
#if _DUSB>=1
		Serial.println(F("DONE"));
#endif
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	
	
	// Reset the statistics
	server.on("/RESET", []() {
		Serial.println(F("RESET"));
		startTime= now() - 1;					// Reset all timers too	
		
		statc.msg_ttl = 0;						// Reset package statistics
		statc.msg_ok = 0;
		statc.msg_down = 0;
		
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
#endif

#if _STATISTICS >= 1
		for (int i=0; i<MAX_STAT; i++) { statr[i].sf = 0; }
#if _STATISTICS >= 2
		statc.sf7 = 0;
		statc.sf8 = 0;
		statc.sf9 = 0;
		statc.sf10= 0;
		statc.sf11= 0;
		statc.sf12= 0;
		
		statc.resets= 0;
		writeGwayCfg(CONFIGFILE);
#if _STATISTICS >= 3
		statc.sf7_0 = 0; statc.sf7_1 = 0; statc.sf7_2 = 0;
		statc.sf8_0 = 0; statc.sf8_1 = 0; statc.sf8_2 = 0;
		statc.sf9_0 = 0; statc.sf9_1 = 0; statc.sf9_2 = 0;
		statc.sf10_0= 0; statc.sf10_1= 0; statc.sf10_2= 0;
		statc.sf11_0= 0; statc.sf11_1= 0; statc.sf11_2= 0;
		statc.sf12_0= 0; statc.sf12_1= 0; statc.sf12_2= 0;
#endif
#endif
#endif
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	// Reset the boot counter
	server.on("/BOOT", []() {
		Serial.println(F("BOOT"));
#if _STATISTICS >= 2
		gwayConfig.boots = 0;
		gwayConfig.wifis = 0;
		gwayConfig.views = 0;
		gwayConfig.ntpErr = 0;					// NTP errors
		gwayConfig.ntpErrTime = 0;				// NTP last error time
		gwayConfig.ntps = 0;					// Number of NTP calls
#endif
		gwayConfig.reents = 0;					// Re-entrance

		writeGwayCfg(CONFIGFILE);
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	server.on("/NEWSSID", []() {
		sendWebPage("NEWSSID","");				// Send the webPage string
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	// Set debug parameter
	server.on("/DEBUG=-1", []() {				// Set debug level 0-2						
		debug = (debug+3)%4;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/DEBUG=1", []() {
		debug = (debug+1)%4;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	// Set PDEBUG parameter
	//
		server.on("/PDEBUG=SCAN", []() {		// Set debug level 0-2						
		pdebug ^= P_SCAN;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/PDEBUG=CAD", []() {				// Set debug level 0-2						
		pdebug ^= P_CAD;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/PDEBUG=RX", []() {				// Set debug level 0-2						
		pdebug ^= P_RX;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/PDEBUG=TX", []() {				// Set debug level 0-2						
		pdebug ^= P_TX;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/PDEBUG=PRE", []() {				// Set debug level 0-2						
		pdebug ^= P_PRE;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/PDEBUG=MAIN", []() {				// Set debug level 0-2						
		pdebug ^= P_MAIN;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/PDEBUG=GUI", []() {				// Set debug level 0-2						
		pdebug ^= P_GUI;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/PDEBUG=RADIO", []() {				// Set debug level 0-2						
		pdebug ^= P_RADIO;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	
	// Set delay in microseconds
	server.on("/DELAY=1", []() {
		txDelay+=5000;
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/DELAY=-1", []() {
		txDelay-=5000;
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});


	// Spreading Factor setting
	server.on("/SF=1", []() {
		if (sf>=SF12) sf=SF7; else sf= (sf_t)((int)sf+1);
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/SF=-1", []() {
		if (sf<=SF7) sf=SF12; else sf= (sf_t)((int)sf-1);
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	// Set Frequency of the GateWay node
	server.on("/FREQ=1", []() {
		uint8_t nf = sizeof(freqs)/sizeof(freqs[0]);	// Number of elements in array
#if _DUSB==2
		Serial.print("FREQ==1:: For freq[0] sizeof vector=");
		Serial.print(sizeof(freqs[0]));
		Serial.println();
#endif
		if (ifreq==(nf-1)) ifreq=0; else ifreq++;
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/FREQ=-1", []() {
		uint8_t nf = sizeof(freqs)/sizeof(freqs[0]);	// Number of elements in array
		if (ifreq==0) ifreq=(nf-1); else ifreq--;
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	// Set CAD function off/on
	server.on("/CAD=1", []() {
		_cad=(bool)1;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/CAD=0", []() {
		_cad=(bool)0;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	// GatewayNode
	server.on("/NODE=1", []() {
#if GATEWAYNODE==1
		gwayConfig.isNode =(bool)1;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
#endif
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/NODE=0", []() {
#if GATEWAYNODE==1
		gwayConfig.isNode =(bool)0;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
#endif
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

#if GATEWAYNODE==1	
	// Framecounter of the Gateway node
	server.on("/FCNT", []() {

		frameCount=0; 
		rxLoraModem();							// Reset the radio with the new frequency
		writeGwayCfg(CONFIGFILE);

		//sendWebPage("","");						// Send the webPage string
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
#endif
	// WWW Page refresh function
	server.on("/REFR=1", []() {					// WWW page auto refresh ON
#if A_REFRESH==1
		gwayConfig.refresh =1;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
#endif		
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/REFR=0", []() {					// WWW page auto refresh OFF
#if A_REFRESH==1
		gwayConfig.refresh =0;
		writeGwayCfg(CONFIGFILE);				// Save configuration to file
#endif
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	
	// Switch off/on the HOP functions
	server.on("/HOP=1", []() {
		_hop=true;
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/HOP=0", []() {
		_hop=false;
		ifreq=0; 
		setFreq(freqs[ifreq].upFreq);
		rxLoraModem();
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

#if !defined ESP32_ARCH
	// Change speed to 160 MHz
	server.on("/SPEED=80", []() {
		system_update_cpu_freq(80);
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
	server.on("/SPEED=160", []() {
		system_update_cpu_freq(160);
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});
#endif
	// Display Documentation pages
	server.on("/DOCU", []() {

		server.sendHeader("Location", String("/"), true);
		buttonDocu();
		server.send ( 302, "text/plain", "");
	});

	// Display LOGging information
	server.on("/LOG", []() {
		server.sendHeader("Location", String("/"), true);
#if _DUSB>=1
		Serial.println(F("LOG button"));
#endif
		buttonLog();
		server.send ( 302, "text/plain", "");
	});
	
	// Display Expert mode or Simple mode
	server.on("/EXPERT", []() {
		server.sendHeader("Location", String("/"), true);
		gwayConfig.expert = bool(1 - (int) gwayConfig.expert) ;
		server.send ( 302, "text/plain", "");
	});
	
	// Display the NODE statistics
	server.on("/NODE", []() {
		server.sendHeader("Location", String("/"), true);
#if _DUSB>=1
		Serial.println(F("NODE button"));
#endif
		buttonNode();
		server.send ( 302, "text/plain", "");
	});

	
	// Update the sketch. Not yet implemented
	server.on("/UPDATE=1", []() {
#if A_OTA==1
		updateOtaa();
#endif
		server.sendHeader("Location", String("/"), true);
		server.send ( 302, "text/plain", "");
	});

	// -----------
	// This section from version 4.0.7 defines what PART of the
	// webpage is shown based on the buttons pressed by the user
	// Maybe not all information should be put on the screen since it
	// may take too much time to serve all information before a next
	// package interrupt arrives at the gateway
	
	Serial.print(F("WWW Server started on port "));
	Serial.println(A_SERVERPORT);
	return;
} // setupWWW



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
#if ESP32_ARCH==1
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
	response +="<tr><td class=\"cell\">NTP Server</td><td class=\"cell\">"; response+=NTP_TIMESERVER; response+="</tr>";
	response +="<tr><td class=\"cell\">LoRa Router</td><td class=\"cell\">"; response+=_TTNSERVER; response+="</tr>";
	response +="<tr><td class=\"cell\">LoRa Router IP</td><td class=\"cell\">"; 
	printIP((IPAddress)ttnServer,'.',response); 
	response +="</tr>";
#ifdef _THINGSERVER
	response +="<tr><td class=\"cell\">LoRa Router 2</td><td class=\"cell\">"; response+=_THINGSERVER; 
	response += String() + ":" + _THINGPORT + "</tr>";
	response +="<tr><td class=\"cell\">LoRa Router 2 IP</td><td class=\"cell\">"; 
	printIP((IPAddress)thingServer,'.',response);
	response +="</tr>";
#endif
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
		response +="<th class=\"thead\">Value</th>";
		response +="<th colspan=\"2\" class=\"thead\">Set</th>";
		response +="</tr>";
	
		response +="<tr><td style=\"border: 1px solid black; width:120px;\">Gateway ID</td>";
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
// XXX We Shoudl find an ESP32 alternative
#if !defined ESP32_ARCH
		response +="<tr><td class=\"cell\">ESP speed</td><td class=\"cell\">"; response+=ESP.getCpuFreqMHz(); 
		response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"SPEED=80\"><button>80</button></a></td>";
		response +="<td style=\"border: 1px solid black; width:40px;\"><a href=\"SPEED=160\"><button>160</button></a></td>";
		response+="</tr>";
		response +="<tr><td class=\"cell\">ESP Chip ID</td><td class=\"cell\">"; response+=ESP.getChipId(); response+="</tr>";
#endif
		response +="<tr><td class=\"cell\">OLED</td><td class=\"cell\">"; response+=OLED; response+="</tr>";
		
#if _STATISTICS >= 1
		response +="<tr><td class=\"cell\">WiFi Setups</td><td class=\"cell\">"; response+=gwayConfig.wifis; response+="</tr>";
		response +="<tr><td class=\"cell\">WWW Views</td><td class=\"cell\">"; response+=gwayConfig.views; response+="</tr>";
#endif

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
			default: response +="unknown"; break;
		}
		response +="</td></tr>";
		
		response +="<tr><td class=\"cell\">_STRICT_1CH</td>";
		response +="<td class=\"cell\">" ;
		response += String() + _STRICT_1CH;
		response +="</td></tr>";		

		response +="<tr><td class=\"cell\">flags (8 bits)</td>";
		response +="<td class=\"cell\">0x";
		if (flags <16) response += "0";
		response +=String(flags,HEX); response+="</td></tr>";

		
		response +="<tr><td class=\"cell\">mask (8 bits)</td>";
		response +="<td class=\"cell\">0x"; 
		if (mask <16) response += "0";
		response +=String(mask,HEX); response+="</td></tr>";
		
		response +="<tr><td class=\"cell\">Re-entrant cntr</td>";
		response +="<td class=\"cell\">"; 
		response += String() + gwayConfig.reents;
		response +="</td></tr>";

		response +="<tr><td class=\"cell\">ntp call cntr</td>";
		response +="<td class=\"cell\">"; 
		response += String() + gwayConfig.ntps;
		response+="</td></tr>";
		
		response +="<tr><td class=\"cell\">ntpErr cntr</td>";
		response +="<td class=\"cell\">"; 
		response += String() + gwayConfig.ntpErr;
		response +="</td>";
		response +="<td colspan=\"2\" style=\"border: 1px solid black;\">";
		stringTime(gwayConfig.ntpErrTime, response);
		response +="</td>";
		response +="</tr>";
		
		response +="<tr><td class=\"cell\">Time Correction (uSec)</td><td class=\"cell\">"; 
		response += txDelay; 
		response +="</td>";
		response +="<td class=\"cell\"><a href=\"DELAY=-1\"><button>-</button></a></td>";
		response +="<td class=\"cell\"><a href=\"DELAY=1\"><button>+</button></a></td>";
		response +="</tr>";
		
		response +="</table>";
		
		server.sendContent(response);
	}// if gwayConfig.expert
} // interruptData


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


#endif // A_SERVER==1

