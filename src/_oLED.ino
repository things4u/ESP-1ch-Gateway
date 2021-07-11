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
// This file contains the state machine code enabling to receive
// and transmit packages/messages.
// ========================================================================================
//




// --------------------------------------------------------------------	
// Initilize the Oled functions.
// This function will init the Oled screen. Depending on the 
// availability of the reset button it will reset the display first.
// --------------------------------------------------------------------
void init_oLED() 
{
#if _OLED>=1
#if defined OLED_RST
	pinMode(OLED_RST,OUTPUT);
	digitalWrite(OLED_RST, LOW); 	// low to reset Oled
	delay(100); 
	digitalWrite(OLED_RST, HIGH); 	// must be high to turn on Oled
	delay(50);
#else
	//
#endif
	// Initialising the UI will init the display too.
	display.init();
	delay(100);

	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_24);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 24, "STARTING");
	display.display();
#endif
}

// --------------------------------------------------------------------
// Activate the Oled. Always print the same info.
// These are 4 fields:
// SSID, IP, ID, 
//
// --------------------------------------------------------------------
void acti_oLED() 
{
#if _OLED>=1
	// Initialising the UI will init the display too.
	display.clear();

# if _OLED==1
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 0, "READY,  SSID=");
	display.drawString(0, 16, WiFi.SSID());
	display.drawString(0, 32, "IP=");
	display.drawString(0, 48, WiFi.localIP().toString().c_str() );
# elif _OLED==2
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 0, "READY,  SSID=");
	display.drawString(0, 16, WiFi.SSID());
	display.drawString(0, 32, "IP=");
	display.drawString(0, 48, WiFi.localIP().toString().c_str() );
# endif //_OLED

	display.display();

#endif //_OLED
	delay(4000);
}

// --------------------------------------------------------------------
// Print a message on the Oled.
// Note: The whole message must fit in the buffer
//
// --------------------------------------------------------------------
void msg_oLED(String mesg) 
{
#if _OLED>=1
    display.clear();

	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_24);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 24, String(mesg));

    display.display();
	yield();
#endif //_OLED
}

// Print a larger Oled message consisting of two strings

void msg_lLED(String mesg, String mesg2) 
{
#if _OLED>=1
    display.clear();

	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 8, String(mesg));
	display.drawString(0, 36, String(mesg2));
	
    display.display();
	yield();
#endif //_OLED
}

// --------------------------------------------------------------------
// Print the Oled address in use
//
// --------------------------------------------------------------------
void addr_oLED() 
{
#if _OLED>=1
	#if _DUSB>=1
		Serial.print(F("OLED_ADDR=0x"));
		Serial.println(OLED_ADDR, HEX);
	#endif //_DUSB
#endif
}
