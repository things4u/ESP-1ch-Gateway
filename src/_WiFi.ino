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
				mPrint("WlanStatus:: CONNECTED ssid=" + String(WiFi.SSID()));	// 3
			}
#			endif //_MONITOR
			WiFi.setAutoReconnect(true);				// Reconect to this AP if DISCONNECTED
			return(1);
			break;

		// In case we get disconnected from the AP we lose the IP address.
		// The ESP is configured to reconnect to the last router in memory.
		case WL_DISCONNECTED:
#			if _MONITOR>=1
			if ( debug>=0 ) {
				mPrint("WlanStatus:: DISCONNECTED, IP=" + String(WiFi.localIP().toString())); // 6
			}
#			endif
			//while (! WiFi.isConnected() ) {
				delay(100);
			//}
			return(0);
			break;

		// When still pocessing
		case WL_IDLE_STATUS:
#			if _MONITOR>=1
			if ( debug>=0 ) {
				mPrint("WlanStatus:: IDLE");								// 0
			}
#			endif //_MONITOR
			break;
		
		// This code is generated as soon as the AP is out of range
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
				mPrint("WlanStatus:: Connect FAILED");								// 4
#			endif //_MONITOR
			break;
			
		// Never seen this code
		case WL_SCAN_COMPLETED:
#			if _MONITOR>=1
			if ( debug>=0 )
				mPrint("WlanStatus:: SCAN COMPLETE");						// 2
#			endif //_MONITOR
			break;
			
		// Never seen this code
		case WL_CONNECTION_LOST:
#			if _MONITOR>=1
			if ( debug>=0 )
				mPrint("WlanStatus:: Connection LOST");						// 5
#			endif //_MONITOR
			break;
			
		// This code is generated for example when WiFi.begin() has not been called
		// before accessing WiFi functions
		case WL_NO_SHIELD:
#			if _MONITOR>=1
			if ( debug>=0 )
				mPrint("WlanStatus:: WL_NO_SHIELD");							// 
#			endif //_MONITOR
			break;
			
		default:
#			if _MONITOR>=1
			if ( debug>=0 ) {
				mPrint("WlanStatus Error:: code=" + String(WiFi.status()));	// 255 means ERROR
			}
#			endif //_MONITOR
			break;
	}
	return(-1);
	
} // WlanStatus


// ----------------------------------------------------------------------------
// When ESP WiFi Manager, do this
// ----------------------------------------------------------------------------
int wifiMgr() 
{
#if _WIFIMANAGER==1

	WiFiManager wifiManager;
	
#	if _MONITOR>=1
	if (debug>=1) {
		mPrint("Starting Access Point Mode");
		mPrint("Connect Wifi to accesspoint: "+String(AP_NAME)+" and connect to IP: 192.168.4.1");
	}
#	endif //_MONITOR

	// Add the ID to the SSID of the WiFiManager
	String ssid = String(AP_NAME) + "-" + String(ESP_getChipId(), HEX);
	char s [ssid.length() + 1];
	strncpy(s, ssid.c_str(), ssid.length());
	s[ssid.length()]= 0;

	wifiManager.setDebugOutput(false);
	wifiManager.setConfigPortalTimeout(120);
	wifiManager.startConfigPortal(s, AP_PASSWD );
	wifiManager.setAPCallback(configModeCallback);

	if (!wifiManager.autoConnect()) {
#		if _MONITOR>=1
			Serial.println("wifiMgr:: failed to connect and hit timeout");
#		endif
		ESP.restart();
		delay(1000);
	}

#	if _MONITOR>=1
	if ((debug>=1) && (pdebug & P_MAIN)) {
		mPrint("WlanConnect:: Now starting WlanStatus");
		delay(1);
		int i = WlanStatus();
		switch (i) {
			case 1: mPrint("WlanConnect:: WlanStatus Connected"); break;
			case 0: mPrint("WlanConnect:: WlanStatus Disconnected"); break;
			default: mPrint("WlanConnect:: WlanStatus other");
		}
	}
#	endif //_MONITOR 

	// At this point, there IS a Wifi Access Point found and connected
	// We must connect to the local SPIFFS storage to store the access point
	//String s = WiFi.SSID();

#	if defined(ARDUINO_ARCH_ESP8266)
		// Now look for the password
		struct station_config sta_conf;
		wifi_station_get_config(&sta_conf);
#	else
		mPrint("wifiMgr:: define arch specific");
#	endif

#endif //_WIFIMANAGER
	return 1;
}

// ----------------------------------------------------------------------------
// Callback Function for MiFiManager
// ----------------------------------------------------------------------------
//gets called when WiFiManager enters configuration mode

#if _WIFIMANAGER==1
void configModeCallback (WiFiManager *myWiFiManager) 
{

  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());

}
#endif //_WIFIMANAGER

// ----------------------------------------------------------------------------
// Function to join the Wifi Network (as defined in sta array
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
//  XXX After a few retries, the ESP should be reset. Note: Switching between 
//	two SSID's does the trick. Retrying the same SSID does not.
//	Workaround is found below: Let the ESP forget the SSID
//
//  NOTE: The Serial works only on debug setting and not on pdebug. This is 
//	because WiFi problems would make webserver (which works on WiFi) useless.
// ----------------------------------------------------------------------------
int WlanConnect(int maxTry) {
  
	unsigned char agains = 0;
	unsigned char wpa_index = 0;

//		WiFi.persistent(false);
//		WiFi.mode(WIFI_OFF);   		// this is a temporary line, to be removed after SDK update to 1.5.4
	
	// Else: try to connect to WLAN as long as we are not connected.
	// The try parameters tells us how many times we try before giving up
	// Value 0 is reserved for setup() first time connect
	int i=0;

	while ((WiFi.status() != WL_CONNECTED) && (( i<= maxTry ) || (maxTry==0)) )
	{
		// We try every SSID in wpa array until success
		for (unsigned int j=wpa_index; (j<(sizeof(wpa)/sizeof(wpa[0]))) && (WiFi.status() != WL_CONNECTED ); j++)
		{
			// Start with well-known access points in the list
			char *ssid		= wpa[j].login;
			char *password	= wpa[j].passw;
			
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_MAIN)) {
				Serial.print(i);
				Serial.print(':');
				Serial.print(j); 
				Serial.print(':');
				Serial.print(sizeof(wpa)/sizeof(wpa[0]));
				Serial.print(F(". WlanConnect SSID=")); 
				Serial.print(ssid);
				if ( debug>=2 ) {
					Serial.print(F(", pass="));
					Serial.print(password);
				}
				Serial.println();
			}
#			endif //_MONITOR
	
			// Count the number of times we call WiFi.begin
			gwayConfig.wifis++;

			WiFi.mode(WIFI_STA);
			delay(1000);
			WiFi.begin(ssid, password);
			delay(8000);
			
			// Check the connection status again, return values
			// 1	= CONNECTED
			// 0	= DISCONNECTED (will reconnect)
			// -1	= No SSID or other cause			
			int stat = WlanStatus();
			if ( stat == 1) {
				writeGwayCfg(_CONFIGFILE, &gwayConfig );					// Write configuration to SPIFFS
				return(1);
			}
		
			// We increase the time for connect but try the same SSID
			// We try for several times
			agains=1;
			while ((WiFi.status() != WL_CONNECTED) && (agains < 8)) {
				agains++;		
				delay(8000);											// delay(agains*500);
#				if _MONITOR>=1
				if ( debug>=0 ) {
					Serial.print(".");									// Serial only
				}
#				endif //_MONITOR
			}	
	
			// Make sure that we can connect to different AP's than 1
			// this is a patch. Normally we connect to previous one.
			WiFi.persistent(false);
			WiFi.mode(WIFI_OFF);   // this is a temporary line, to be removed after SDK update to 1.5.4

		} //for next WPA defined AP
	  
		i++;			// Number of times we try to connect
	} //while

	yield();
	return(1);
	
} //WlanConnect


// ----------------------------------------------------------------------------
// resolveHost
// This function will use MDNS or DNS to resolve a hostname. 
// So it may be .local or a normal hostname.
// Parameters:
//	svrName: Name of the server we want the IP address from
//	maxTry: The number we triue to get the value. 0 means: wait forever.
// Return:
//	svrIP: 4 byte IP address of machine resolved
// ----------------------------------------------------------------------------
IPAddress resolveHost(String svrName, int maxTry) 
{
	IPAddress svrIP;
	
	if (svrName.endsWith(".local")) {
#		if defined(ESP32_ARCH)
			svrName=svrName.substring(0,svrName.length()-6);
			svrIP = MDNS.queryHost(svrName);
			for (byte i=0; i<maxTry; i++) {						// Try 5 times MDNS
				svrIP = MDNS.queryHost(svrName);
				if (svrIP.toString() != "0.0.0.0") break;
#				if (_MONITOR>=1)
					mPrint("ReTrying to resolve with mDNS");
#				endif //_MONITOR
				delay(12000);
			}
#		else
			char cc[svrName.length() +1 ];
			strncpy(cc, svrName.c_str(),svrName.length());
			cc[svrName.length()]=0;
		
			for (byte i=0; i<maxTry; i++) {
				if (!WiFi.hostByName(cc, svrIP)) 					// Use DNS to get server IP once
				{
					mPrint("resolveHost:: ERROR hostByName="+ String(cc)+", len=" + String(sizeof(cc)));
				};
				delay(1000);
			}
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_MAIN)) {
				mPrint("resolveHost:: "+  String(cc) +" IP=" + String(svrIP.toString()) );
			}
#			endif //_MONITOR
#		endif
	}
	else // Non LOCAL
	{
		char cc[svrName.length() +1 ];							// Assume whole array initially 0
		strncpy(cc, svrName.c_str(),svrName.length());
		cc[svrName.length()]=0;
		
		for (byte i=0; i<maxTry; i++) {
			if (WiFi.hostByName(cc, svrIP)) 					// Use DNS to get server IP once
			{
#				if _MONITOR>=1
					mPrint("resolveHost:: OK="+  String(cc) +" IP=" + String(svrIP.toString()) );
#				endif //_MONITOR
					return svrIP;									// If connected
			}
			else 													// Else not connected
			{
				mPrint("resolveHost:: ERROR hostByName="+ String(cc)+", len=" + String(sizeof(cc)));
			};
			delay(1000);
		}
	}
	return svrIP;
}