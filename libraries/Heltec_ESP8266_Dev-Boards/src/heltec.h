

#ifndef _HELTEC_H_
#define _HELTEC_H_

#if defined(ESP8266)

#include <Arduino.h>
#include <Wire.h>

#if defined( WIFI_Kit_8 )
#include "oled/SSD1306.h"
#include "oled/OLEDDisplayUi.h"

#endif


class Heltec_ESP8266 {

 public:
    Heltec_ESP8266();
	~Heltec_ESP8266();

    void begin(bool DisplayEnable=true, bool SerialEnable=true);
//#if defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ) || defined( Wireless_Stick )
//   LoRaClass LoRa;
//#endif

    SSD1306Wire *display;

/*wifi kit 32 and WiFi LoRa 32(V1) do not have vext*/
//    void VextON(void);
//    void VextOFF(void);
};

extern Heltec_ESP8266 Heltec;

#else
#error ��This library only supports boards with ESP8266 processor.��
#endif


#endif
