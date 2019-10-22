// This example just provide basic function test;
// For more informations, please vist www.heltec.cn or mail to support@heltec.cn

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "heltec.h" // alias for `#include "SSD1306Wire.h"`



void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Serial Enable*/);
  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);

  Heltec.display->drawString(0,0,"Hello World!");
  Heltec.display->display();
}

void loop() { }