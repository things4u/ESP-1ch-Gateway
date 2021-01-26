// Slow down the serial port speed
#define _BAUDRATE 9600						// Works for debug messages to serial momitor

// change the gateway to another band

// Define the frequency band the gateway will listen on. Valid options are
// EU863_870	Europe
// US902_928	North America
// AU925_928	Australia
// CN470_510	China
// IN865_867	India
// CN779-787	(Not Used!)
// EU433		Europe
// AS923		(Not Used)
// You can find the definitions in "loraModem.h" and frequencies in
// See https://www.thethingsnetwork.org/docs/lorawan/frequency-plans.html
#undef EU863_870
#define AU925_928 1