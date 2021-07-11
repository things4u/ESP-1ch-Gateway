# Single Channel LoRaWAN Gateway

Last Updated: September 08, 2020	  
Author: M. Westenberg (mw12554@hotmail.com)  
Copyright: M. Westenberg (mw12554@hotmail.com)  

All rights reserved. This program and the accompanying materials are made available under the terms 
of the MIT License which accompanies this distribution, and is available at
https://opensource.org/licenses/mit-license.php  
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Maintained by Maarten Westenberg (mw12554@hotmail.com)



# ToDo Functions

Features not in release 6.2.6

- Make better version for _ENCODE
- Repair _REPEATER to work for devices that are far away from the gateway
- Change Downstream timing to be more accurate (interrupt driven?).
- Frequency: Support for eu433 frequencies (Standard)
- Testing and timing of downlink functions (need quiet area)
- Get HOP frequency functions to work on three frequencies (Naah)
- Security: Enable passwords for GUI access and WiFi upload (may not be necessary for normal home router use)
- Enable remote updating through GUI
- Support FSK (This may not be necessary)
- Support for other Class A and B, C of LoRa devices
- Support for 3G/4G/5G devices (Probably overkill for ESP devices, better buy a real gateway)



# License

The source files of the gateway sketch in this repository is made available under the MIT
license. The libraries included in this repository are included for convenience only and 
all have their own license, and are not part of the ESP 1ch gateway code.
