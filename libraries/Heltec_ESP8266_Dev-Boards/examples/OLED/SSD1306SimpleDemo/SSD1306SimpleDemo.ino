// This example just provide basic function test;
// For more informations, please vist www.heltec.cn or mail to support@heltec.cn

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "heltec.h" // alias for `#include "SSD1306Wire.h"`
#include "images.h"


#define DEMO_DURATION 3000
typedef void (*Demo)(void);

int demoMode = 0;
int counter = 1;

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Serial Enable*/);

  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);

}

void drawFontFaceDemo() {
    // Font Demo1
    // create more fonts at http://oledHeltec.display->squix.ch/
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 0, "Hello world");
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->drawString(0, 10, "Hello world");
   
}

void drawTextFlowDemo() {
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->drawStringMaxWidth(0, 0, 128,
      "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr" );
}

void drawTextAlignmentDemo() {
    // Text alignment demo
  Heltec.display->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(0, 0, "Left aligned (0,10)");

  // The coordinates define the center of the text
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawString(64, 10, "Center aligned (64,10)");

  // The coordinates define the right end of the text
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(128, 20, "Right aligned (128,20)");
}

void drawRectDemo() {
      // Draw a pixel at given position
    for (int i = 0; i < 10; i++) {
      Heltec.display->setPixel(i, i);
      Heltec.display->setPixel(10 - i, i);
    }
    Heltec.display->drawRect(12, 12, 20, 20);

    // Fill the rectangle
    Heltec.display->fillRect(14, 14, 17, 17);

    // Draw a line horizontally
    Heltec.display->drawHorizontalLine(0, 40, 20);

    // Draw a line horizontally
    Heltec.display->drawVerticalLine(40, 0, 20);
}

void drawCircleDemo() {
  for (int i=1; i < 8; i++) {
    Heltec.display->setColor(WHITE);
    Heltec.display->drawCircle(32, 16, i*2);
    if (i % 2 == 0) {
      Heltec.display->setColor(BLACK);
    }
    Heltec.display->fillCircle(96, 16, 32 - i* 2);
  }
}

void drawProgressBarDemo() {
  int progress = (counter / 5) % 100;
  // draw the progress bar
  Heltec.display->drawProgressBar(0, 10, 120, 10, progress);

  // draw the percentage as String
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawString(64, 0, String(progress) + "%");
}

void drawImageDemo() {
    // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
    // on how to create xbm files
    Heltec.display->drawXbm(34, 0, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

Demo demos[] = {drawFontFaceDemo, drawTextFlowDemo, drawTextAlignmentDemo, drawRectDemo, drawCircleDemo, drawProgressBarDemo, drawImageDemo};
int demoLength = (sizeof(demos) / sizeof(Demo));
long timeSinceLastModeSwitch = 0;

void loop() {
  // clear the display
  Heltec.display->clear();
  // draw the current demo method
  demos[demoMode]();

  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(10, 128, String(millis()));
  // write the buffer to the display
  Heltec.display->display();

  if (millis() - timeSinceLastModeSwitch > DEMO_DURATION) {
    demoMode = (demoMode + 1)  % demoLength;
    timeSinceLastModeSwitch = millis();
  }
  counter++;
  delay(10);
}