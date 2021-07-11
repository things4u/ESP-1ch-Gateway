# ESP32 HTTP Firmware Update (OTA)

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/03b36fac07824cd08884e1f19bb34fcb)](https://www.codacy.com/app/suculent/esp32-http-update?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=suculent/esp32-http-update&amp;utm_campaign=Badge_Grade)

ESP Clone of https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266httpUpdate (most work done by Markus Sattler).

# TL;DR

This is just a quick port of ESP8266httpUpdate for ESP32.

Buildable with Arduino framework for ESP32.

There are things remaining to be solved:

* Some headers are deprecated (will change for ESP32 anyway)
* Download to SPIFFS with AES-256 decryption
* Does not support ESP-IDF.
