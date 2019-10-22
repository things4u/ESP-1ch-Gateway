/*
 * HelTec Automation(TM) wifi kit 8 factory test code, witch include
 * follow functions:
 *
 * - Basic OLED function test;
 *
 * - Basic serial port test(in baud rate 115200);
 *
 * - LED blink test;
 *
 * - WIFI join and scan test;
 *
 * - Timer test and some other Arduino basic functions.
 *
 * by Aaron.Lee from HelTec AutoMation, ChengDu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.cn
 *
 * this project also realess in GitHub:
 * https://github.com/HelTecAutomation/Heltec_ESP8266
*/
#include "heltec.h"
#include "ESP8266WiFi.h"


//unsigned int counter = 0;


void WIFISetUp(void)
{
	// Set WiFi to station mode and disconnect from an AP if it was previously connected
	WiFi.disconnect(true);
	delay(100);
	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.begin("Your WiFi SSID","Your Password");//fill in "Your WiFi SSID","Your Password"
	delay(100);
	Heltec.display->clear();

	byte count = 0;
	while(WiFi.status() != WL_CONNECTED && count < 10)
	{
		count ++;
		delay(500);
		Heltec.display->drawString(0, 0, "Connecting...");
		Heltec.display->display();
	}
  //Heltec.display->clear();
	if(WiFi.status() == WL_CONNECTED)
	{
		//Heltec.display->drawString(35, 38, "WIFI SETUP");
		Heltec.display->drawString(0, 9, "OK");
		Heltec.display->display();
		delay(1000);
		Heltec.display->clear();
	}
	else
	{
		//Heltec.display->clear();
		Heltec.display->drawString(0, 9, "Failed");
		Heltec.display->display();
		delay(1000);
		Heltec.display->clear();
	}
}

void WIFIScan(unsigned int value)
{
	unsigned int i;
	WiFi.mode(WIFI_STA);
	for(i=0;i<value;i++)
	{
		Heltec.display->drawString(0, 0, "Scan start...");
		Heltec.display->display();

		int n = WiFi.scanNetworks();
		Heltec.display->drawString(0, 9, "Scan done");
		Heltec.display->display();
		delay(500);
		Heltec.display->clear();

		if (n == 0)
		{
			Heltec.display->clear();
			Heltec.display->drawString(0, 18, "no network found");
			Heltec.display->display();
			while(1);
		}
		else
		{
			Heltec.display->drawString(0, 18, (String)n + " nets found");
			Heltec.display->display();
			delay(2000);
			Heltec.display->clear();
		}
	}
}

void setup()
{
	//pinMode(LED,OUTPUT);
	Heltec.begin(true /*DisplayEnable Enable*/, true /*Serial Enable*/);

	WIFISetUp();
	WIFIScan(1);
}

void loop()
{

}
