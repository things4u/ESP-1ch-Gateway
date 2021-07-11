// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2020 Maarten Westenberg version for ESP8266
//
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
// This file contains the ota code for the ESP Single Channel Gateway.

// Provide OTA server funcionality so the 1ch gateway can be updated 
// over the air.
// This code uses the ESPhttpServer functions to update the gateway.

#if _OTA==1

//extern ArduinoOTAClass ArduinoOTA;

// Make sure that webserver is running before continuing

// ----------------------------------------------------------------------------
// setupOta
// Function to run in the setup() function to initialise the update function
// ----------------------------------------------------------------------------
void setupOta(char *hostname) {

	ArduinoOTA.begin();
	
#	if _MONITOR>=1
		mPrint("setupOta:: Started");
#	endif	 //_MONITOR

	// Hostname defaults to esp8266-[ChipID] for ESP8266 nodes
	ArduinoOTA.setHostname(hostname);
	
	ArduinoOTA.onStart([]() {
		String type;
		// XXX version mismatch of platform.io and ArduinoOtaa
		// see https://github.com/esp8266/Arduino/issues/3020
		//if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		//else // U_SPIFFS
		//	type = "filesystem";

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.println("Start updating " + type);
	});
	
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	
#	if _MONITOR>=1
		if (debug>=1) {
			mPrint("Ready IP address: " + String(WiFi.localIP().toString()));
		}
#	endif //_MONITOR
	
	// Only if the Webserver is active also
#if _SERVER==2										// Displayed for the moment
	ESPhttpUpdate.rebootOnUpdate(false);
   
	server.on("/esp", HTTP_POST, [&](){
   
      HTTPUpdateResult ret = ESPhttpUpdate.update(server.arg("firmware"), "1.0.0");
	  
      switch(ret) {
        case HTTP_UPDATE_FAILED:
            //PREi::sendJSON(500, "Update failed.");
			Serial.println(F("Update failed"));
            break;
        case HTTP_UPDATE_NO_UPDATES:
            //PREi::sendJSON(304, "Update not necessary.");
			Serial.println(F("Update not necessary"));
            break;
        case HTTP_UPDATE_OK:
            //PREi::sendJSON(200, "Update started.");
			Serial.println(F("Update started"));
            ESP.restart();
            break;
		default:
			Serial.println(F("setupOta:: Unknown ret="));
      }
	});
#endif
}


// ----------------------------------------------------------------------------
// updateOtaa()
//
// ----------------------------------------------------------------------------
void updateOtaa() {

	String response="";
	Serial.print(F("updateOtaa:: <unimplemented> IP="));
	printIP((IPAddress)WiFi.localIP(),'.',response);
	Serial.println(response);
	
//	ESPhttpUpdate.update(response, 80, "/arduino.bin");
}


#endif