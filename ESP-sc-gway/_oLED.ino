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
// This file contains the state machine code enabling to receive
// and transmit packages/messages.
// ========================================================================================
//

#if OLED>=1


// --------------------------------------------------------------------	
// Initilize the OLED functions.
// This function will init the OLED screenb. Depending on the 
// availability of the reset button it will reset the display first.
// --------------------------------------------------------------------
void init_oLED() 
{
#if defined OLED_RST
	pinMode(OLED_RST,OUTPUT);
	digitalWrite(OLED_RST, LOW); 	// low to reset OLED
	delay(100); 
	digitalWrite(OLED_RST, HIGH); 	// must be high to turn on OLED
	delay(50);
#else
#endif
	// Initialising the UI will init the display too.
	display.init();
	delay(100);
	//display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, FALSE)
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_24);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 24, "STARTING");
	display.display();
}

// --------------------------------------------------------------------
// Activate the OLED
//
// --------------------------------------------------------------------
void acti_oLED() 
{
	// Initialising the UI will init the display too.
	display.clear();
	
#if OLED==1
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 0, "READY,  SSID=");
	display.drawString(0, 16, WiFi.SSID());
	display.drawString(0, 32, "IP=");
	display.drawString(0, 48, WiFi.localIP().toString().c_str() );
#elif OLED==2
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 0, "READY,  SSID=");
	display.drawString(0, 16, WiFi.SSID());
	display.drawString(0, 32, "IP=");
	display.drawString(0, 48, WiFi.localIP().toString().c_str() );
#endif

	display.display();
	
	delay(4000);
}

// --------------------------------------------------------------------
// Print a message on the OLED.
// Note: The whole message must fit in the buffer
//
// --------------------------------------------------------------------
void msg_oLED(String tim, String sf) {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
	
	display.drawString(0, 48, "LEN: " );
//    display.drawString(40, 48, String((int)messageLength) );
    display.display();
	yield();
}

// --------------------------------------------------------------------
// Print the OLED address in use
//
// --------------------------------------------------------------------
void addr_oLED() 
{
#if _DUSB>=1
	Serial.print(F("OLED_ADDR=0x"));
	Serial.println(OLED_ADDR, HEX);
#endif //_DUSB
}

#endif