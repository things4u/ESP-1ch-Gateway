// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2020 Maarten Westenberg version for ESP8266
//
// based on work done by Thomas Telkamp for Raspberry PI 1ch gateway
// and many others.
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
// This file contains a number of compile-time settings and definitions for Oled support.
//
// ----------------------------------------------------------------------------------------

// OLEDs supported by this program must be I2C.
// This is because we do not want any disturbance in the SPI area
// which is also interfacing the LORA tranceiver.
//
// The following OLEDs are supported:
// 0. No OLED connected
// 1. 0.9" OLED (cheap)
// 2. 1.3" OLED with much better and larger display
// 4. TTGO board

#if _OLED>=1									// If OLED is used

// ----------------------------------------------------------------------------------------
// Define the different PIN's used for SCL/SDA for each arch.
//
#if _PIN_OUT==1									// HALLARD
#define OLED_SCL 5								// GPIO5 / D1
#define OLED_SDA 4								// GPIO4 / D2

#elif _PIN_OUT==2								// COMRESULT				
#define OLED_SCL 0								// GPIO0 / D3
#define OLED_SDA 2								// GPIO2 / D4

#elif _PIN_OUT==4								// TTGO (onboard version used, also for DIY)
#define OLED_SCL 15								// GPIO15 / 
#define OLED_SDA 4								// GPIO4 / 
#define OLED_RST 16								// Reset pin (Some OLED displays do not have it)

#elif _PIN_OUT==5								// TTGO with external OLED
#define OLED_SCL 22								// GPIO22 / SCL
#define OLED_SDA 21								// GPIO21 / SDA
#define OLED_RST 16								// Reset pin (Some OLED displays do not have it)

#endif


// ----------------------------------------------------------------------------------------
// Define the different OLED versions
//
#if _OLED==1
#include "SSD1306.h"
#define OLED_ADDR 0x3C							// Default 0x3C for 0.9", for 1.3" it is 0x78
SSD1306  display(OLED_ADDR, OLED_SDA, OLED_SCL);// i2c ADDR & SDA, SCL on wemos
#endif //_OLED

// This is an 1.3" OLED display which is running on I2C
#if _OLED==2
#include "SH1106.h"
#define OLED_ADDR 0x3C							// Default 0x3C for 1.3" SH1106
SH1106  display(OLED_ADDR, OLED_SDA, OLED_SCL);	// i2c ADDR & SDA, SCL on wemos
#endif

#endif //_OLED>=1
