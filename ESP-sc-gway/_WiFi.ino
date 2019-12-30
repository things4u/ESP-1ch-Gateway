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

// ----------------------------------------------------------------------------
// WLANSTATUS prints the status of the Wlan.
// The status of the Wlan "connection" can change if we have no relation
// with the well known router anymore. Normally this relation is preserved
// but sometimes we have to reconfirm to the router again and we get the same
// address too.
// So, if the router is still in range we can "survive" with the same address
// and we may have to renew the "connection" from time to time.
// But when we loose the SSID connection, we may have to look for another router.
//
// Parameters: <none>
// Return value: Returns 1 when still WL_CONNETED, otherwise returns 0
// ----------------------------------------------------------------------------
int WlanStatus() {

	switch (WiFi.status()) {
		case WL_CONNECTED:
#			if _MONITOR>=1
			if ( debug>=1 ) {
				mPrint("A WlanStatus:: CONNECTED to " + String(WiFi.SSID()));	// 3
			}
#			endif //_MONITOR
			WiFi.setAutoReconnect(true);				// Reconect to this AP if DISCONNECTED
			return(1);
			break;

		// In case we get disconnected from the AP we loose the IP address.
		// The ESP is configured to reconnect to the last router in memory.
		case WL_DISCONNECTED:
#			if _MONITOR>=1
			if ( debug>=0 ) {
				mPrint("A WlanStatus:: DISCONNECTED, IP=" + String(WiFi.localIP().toString())); // 6
			}
#			endif
			//while (! WiFi.isConnected() ) {
				// Wait
				delay(1);
			//}
			return(0);
			break;

		// When still pocessing
		case WL_IDLE_STATUS:
#			if _MONITOR>=1
			if ( debug>=0 ) {
				mPrint("A WlanStatus:: IDLE");								// 0
			}
#			endif //_MONITOR
			break;
		
		// This code is generated as soonas the AP is out of range
		// Whene detected, the program will search for a better AP in range
		case WL_NO_SSID_AVAIL:
#			if _MONITOR>=1
			if ( debug>=0 )
				mPrint("WlanStatus:: NO SSID");								// 1
#			endif //_MONITOR
			break;
			
		case WL_CONNECT_FAILED:
#			if _MONITOR>=1
			if ( debug>=0 )
				mPrint("A WlanStatus:: FAILED");							// 4
#			endif //_MONITOR
			break;
			
		// Never seen this code
		case WL_SCAN_COMPLETED:
#			if _MONITOR>=1
			if ( debug>=0 )
				mPrint("A WlanStatus:: SCAN COMPLETE");						// 2
#			endif //_MONITOR
			break;
			
		// Never seen this code
		case WL_CONNECTION_LOST:
#			if _MONITOR>=1
			if ( debug>=0 )
				mPrint("A WlanStatus:: Connection LOST");					// 5
#			endif //_MONITOR
			break;
			
		// This code is generated for example when WiFi.begin() has not been called
		// before accessing WiFi functions
		case WL_NO_SHIELD:
#			if _MONITOR>=1
			if ( debug>=0 )
				Serial.println(F("A WlanStatus:: WL_NO_SHIELD"));			// 
#			endif //_MONITOR
			break;
			
		default:
#			if _MONITOR>=1
			if ( debug>=0 ) {
				mPrint("A WlanStatus Error:: code=" + String(WiFi.status()));	// 255 means ERROR
			}
#			endif //_MONITOR
			break;
	}
	return(-1);
	
} // WlanStatus

// ----------------------------------------------------------------------------
// config.txt is a text file that contains lines(!) with WPA configuration items
// Each line contains an KEY vaue pair describing the gateway configuration
//
// ----------------------------------------------------------------------------
int WlanReadWpa() {
	
	readConfig(CONFIGFILE, &gwayConfig);

	if (gwayConfig.sf != (uint8_t) 0) sf = (sf_t) gwayConfig.sf;
	ifreq = gwayConfig.ch;
	debug = gwayConfig.debug;
	pdebug = gwayConfig.pdebug;
	_cad = gwayConfig.cad;
	_hop = gwayConfig.hop;
	gwayConfig.boots++;							// Every boot of the system we increase the reset
	
#if GATEWAYNODE==1
	if (gwayConfig.fcnt != (uint8_t) 0) frameCount = gwayConfig.fcnt+10;
#endif
	
#if _WIFIMANAGER==1
	String ssid=gwayConfig.ssid;
	String pass=gwayConfig.pass;

	char ssidBuf[ssid.length()+1];
	ssid.toCharArray(ssidBuf,ssid.length()+1);
	char passBuf[pass.length()+1];
	pass.toCharArray(passBuf,pass.length()+1);
	Serial.print(F("WlanReadWpa: ")); Serial.print(ssidBuf); Serial.print(F(", ")); Serial.println(passBuf);
	
	strcpy(wpa[0].login, ssidBuf);				// XXX changed from wpa[0][0] = ssidBuf
	strcpy(wpa[0].passw, passBuf);
	
	Serial.print(F("WlanReadWpa: <")); 
	Serial.print(wpa[0].login); 				// XXX
	Serial.print(F(">, <")); 
	Serial.print(wpa[0].passw);
	Serial.println(F(">"));
#endif

}


// ----------------------------------------------------------------------------
// Print the WPA data of last WiFiManager to the config file
// ----------------------------------------------------------------------------
#if _WIFIMANAGER==1
int WlanWriteWpa( char* ssid, char *pass) {

#	if _MONITOR>=1
	if (( debug >=0 ) && ( pdebug & P_MAIN )) {
		mPrint("WlanWriteWpa:: ssid="+String(ssid)+", pass="+String(pass)); 
	}
#	endif //_MONITOR
	// Version 3.3 use of config file
	String s((char *) ssid);
	gwayConfig.ssid = s;
	
	String p((char *) pass);
	gwayConfig.pass = p;

#	if GATEWAYNODE==1	
		gwayConfig.fcnt = frameCount;
#	endif
	gwayConfig.ch = ifreq;
	gwayConfig.sf = sf;
	gwayConfig.cad = _cad;
	gwayConfig.hop = _hop;
	
	writeConfig( CONFIGFILE, &gwayConfig);
	return 1;
}
#endif



// ----------------------------------------------------------------------------
// Function to join the Wifi Network
//	It is a matter of returning to the main loop() asap and make sure in next loop
//	the reconnect is done first thing. By default the system will reconnect to the
// samen SSID as it was connected to before.
// Parameters:
//		int maxTry: Number of retries we do:
//		0: Used during Setup first CONNECT
//		1: Try once and if unsuccessful return(1);
//		x: Try x times
//
//  Returns:
//		On failure: Return -1
//		On connect: return 1
//		On Disconnect state: return 0
//
//  XXX After a few retries, the ESP8266 should be reset. Note: Switching between 
//	two SSID's does the trick. Rettrying the same SSID does not.
//	Workaround is found below: Let the ESP8266 forget the SSID
//
//  NOTE: The Serial works only on debug setting and not on pdebug. This is 
//	because WiFi problems would make webserver (which works on WiFi) useless.
// ----------------------------------------------------------------------------
int WlanConnect(int maxTry) {
  
#if _WIFIMANAGER==1
	WiFiManager wifiManager;
#endif

	unsigned char agains = 0;
	unsigned char wpa_index = (_WIFIMANAGER >0 ? 0 : 1);		// Skip over first record for WiFiManager

	// The initial setup() call is done with parameter 0
	// We clear the WiFi memory and start with previous AP.
	//
	if (maxTry==0) {
#		if _MONITOR>=1
		mPrint("WlanConnect:: Init para 0");
#		endif //_MONITOR
		WiFi.persistent(false);
		WiFi.mode(WIFI_OFF);   // this is a temporary line, to be removed after SDK update to 1.5.4
		if (gwayConfig.ssid.length() >0) {
			WiFi.begin(gwayConfig.ssid.c_str(), gwayConfig.pass.c_str());
			delay(100);
		}
	}
	
	// So try to connect to WLAN as long as we are not connected.
	// The try parameters tells us how many times we try before giving up
	// Value 0 is reserved for setup() first time connect
	int i=0;

	while ( (WiFi.status() != WL_CONNECTED) && ( i<= maxTry ) )
	{

		// We try every SSID in wpa array until success
		for (int j=wpa_index; (j< (sizeof(wpa)/sizeof(wpa[0]))) && (WiFi.status() != WL_CONNECTED ); j++)
		{
			// Start with well-known access points in the list
			char *ssid		= wpa[j].login;
			char *password	= wpa[j].passw;
#if _DUSB>=1
			if (debug>=0)  {
				Serial.print(i);
				Serial.print(':');
				Serial.print(j); 
				Serial.print(':');
				Serial.print(sizeof(wpa)/sizeof(wpa[0]));
				Serial.print(F(". WiFi connect SSID=")); 
				Serial.print(ssid);
				if ( debug>=1 ) {
					Serial.print(F(", pass="));
					Serial.print(password);
				}
				Serial.println();
			}
#endif		
			// Count the number of times we call WiFi.begin
			gwayConfig.wifis++;



			WiFi.mode(WIFI_STA);
			delay(1000);
			WiFi.begin(ssid, password);
			delay(8000);
			
			// Check the connection status again, return values
			// 1 = CONNECTED
			// 0 = DISCONNECTED (will reconnect)
			// -1 = No SSID or other cause			
			int stat = WlanStatus();
			if ( stat == 1) {
				writeGwayCfg(CONFIGFILE);					// Write configuration to SPIFFS
				return(1);
			}
		
			// We increase the time for connect but try the same SSID
			// We try for 10 times
			agains=1;
			while (((WiFi.status()) != WL_CONNECTED) && (agains < 10)) {
				agains++;
				delay(agains*500);
#				if _DUSB>=1
				if ( debug>=0 ) {
					Serial.print(".");
				}
#				endif //_DUSB
			}
#			if _DUSB>=1
				Serial.println();
#			endif //_DUSB		
			//if ( WiFi.status() == WL_DISCONNECTED) return(0);				// 180811 removed


			// Make sure that we can connect to different AP's than 1
			// this is a patch. Normally we connect to previous one.
			WiFi.persistent(false);
			WiFi.mode(WIFI_OFF);   // this is a temporary line, to be removed after SDK update to 1.5.4

		} //for next WPA defined AP
	  
		i++;			// Number of times we try to connect
	} //while

	
	// If we are not connected to a well known AP
	// we can invoike _WIFIMANAGER or else return unsuccessful.
	if (WiFi.status() != WL_CONNECTED) {
#if _WIFIMANAGER==1
#		if _MONITOR>=1
		if (debug>=1) {
			mPrint("Starting Access Point Mode");
			mPrint("Connect Wifi to accesspoint: "+String(AP_NAME)+" and connect to IP: 192.168.4.1");
		}
#		endif //_MONITOR
		wifiManager.autoConnect(AP_NAME, AP_PASSWD );
		//wifiManager.startConfigPortal(AP_NAME, AP_PASSWD );
		// At this point, there IS a Wifi Access Point found and connected
		// We must connect to the local SPIFFS storage to store the access point
		//String s = WiFi.SSID();
		//char ssidBuf[s.length()+1];
		//s.toCharArray(ssidBuf,s.length()+1);
		
		// Now look for the password
		struct station_config sta_conf;
		wifi_station_get_config(&sta_conf);

		//WlanWriteWpa(ssidBuf, (char *)sta_conf.password);
		WlanWriteWpa((char *)sta_conf.ssid, (char *)sta_conf.password);
#else
#		if _MONITOR>=1
		if (debug>=0) {
			mPrint("WlanConnect:: Not connected, WLAN retry="+String(i)+", stat="+String(WiFi.status()) );
		}
#		endif // _MONITOR
		return(-1);
#endif
	}

	yield();
	return(1);
}

// ----------------------------------------------------------------------------
// resolveHost
// This function will use MDNS or DNS to resolve a hostname. 
// So it may be .local or a normal hostname.
// Return:
//	svrIP: 4 byte IP address of machine resolved
// ----------------------------------------------------------------------------
IPAddress resolveHost(String svrName) 
{
	IPAddress svrIP;
	
#	if _MONITOR>=1
		mPrint("Server " + String(svrName));
#	endif

	if (svrName.endsWith(".local")) {
#		if ESP32_ARCH==1
			svrName=svrName.substring(0,svrName.length()-6);
			svrIP = MDNS.queryHost(svrName);
			for (byte i=0; i<5; i++) {						// Try 5 times MDNS
				svrIP = MDNS.queryHost(svrName);
				if (svrIP.toString() != "0.0.0.0") break;
#				if (_MONITOR>=1)
					mPrint("ReTrying to resolve with mDNS");
#				endif
				delay(1000);
			}
#		else
			char cc[svrName.length() +1 ];
			strcpy(cc, svrName.c_str());
			if (!WiFi.hostByName(cc, svrIP)) 				// Use DNS to get server IP once
			{
				die("resolveHost:: ERROR hostByName="+ String(cc));
			};
#		endif
	}
	else 
	{
		char cc[svrName.length() +1 ];
		strcpy(cc, svrName.c_str());
		if (!WiFi.hostByName(cc, svrIP)) // Use DNS to get server IP once
		{
			die("resolveHost:: ERROR hostByName="+ String(cc));
		};
	}
	return svrIP;
}