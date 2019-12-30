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
// This file contains the LoRa modem specific code enabling to receive
// and transmit packages/messages.
// The functions implemented work in user-space so not with interrupt.
// ========================================================================================
//
//
// ----------------------------------------------------------------------------------------
// Variable definitions
//
//
// ----------------------------------------------------------------------------------------



//
// ========================================================================================
// SPI AND INTERRUPTS
// The RFM96/SX1276 communicates with the ESP8266 by means of interrupts 
// and SPI interface. The SPI interface is bidirectional and allows both
// parties to simultaneous write and read to registers.
// Major drawback is that access is not protected for interrupt and non-
// interrupt access. This means that when a program in loop() and a program
// in interrupt do access the readRegister and writeRegister() function
// at the same time that probably an error will occur.
// Therefore it is best to either not use interrupts AT all (like LMIC)
// or only use these functions in interrupts and to further processing
// in the main loop() program.
//
// ========================================================================================


// ----------------------------------------------------------------------------------------
// Mutex definitions
//
// ----------------------------------------------------------------------------------------
#if MUTEX==1
	void CreateMutux(int *mutex) {
		*mutex=1;
	}

#define LIB_MUTEX 1
#if LIB_MUTEX==1
	bool GetMutex(int *mutex) {
		//noInterrupts();
		if (*mutex==1) { 
			*mutex=0; 
			//interrupts(); 
			return(true); 
		}
		//interrupts();
		return(false);
	}
#else	
	bool GetMutex(int *mutex) {

	int iOld = 1, iNew = 0;

	asm volatile (
		"rsil a15, 1\n"    // read and set interrupt level to 1
		"l32i %0, %1, 0\n" // load value of mutex
		"bne %0, %2, 1f\n" // compare with iOld, branch if not equal
		"s32i %3, %1, 0\n" // store iNew in mutex
		"1:\n"             // branch target
		"wsr.ps a15\n"     // restore program state
		"rsync\n"
		: "=&r" (iOld)
		: "r" (mutex), "r" (iOld), "r" (iNew)
		: "a15", "memory"
	);
	return (bool)iOld;
}
#endif

	void ReleaseMutex(int *mutex) {
		*mutex=1;
	}
	
#endif //MUTEX==1

// ----------------------------------------------------------------------------------------
// Read one byte value, par addr is address
// Returns the value of register(addr)
// 
// The SS (Chip select) pin is used to make sure the RFM95 is selected
// The variable is for obvious reasons valid for read and write traffic at the
// same time. Since both read and write mean that we write to the SPI interface.
// Parameters:
//	Address: SPI address to read from. Type uint8_t
// Return:
//	Value read from address
// ----------------------------------------------------------------------------------------

// define the SPI settings for reading messages
SPISettings readSettings(SPISPEED, MSBFIRST, SPI_MODE0);

uint8_t readRegister(uint8_t addr)
{

	SPI.beginTransaction(readSettings);				
    digitalWrite(pins.ss, LOW);					// Select Receiver
	SPI.transfer(addr & 0x7F);
	uint8_t res = (uint8_t) SPI.transfer(0x00);
    digitalWrite(pins.ss, HIGH);				// Unselect Receiver
	SPI.endTransaction();
    return((uint8_t) res);
}


// ----------------------------------------------------------------------------------------
// Write value to a register with address addr. 
// Function writes one byte at a time.
// Parameters:
//	addr: SPI address to write to
//	value: The value to write to address
// Returns:
//	<void>
// ----------------------------------------------------------------------------------------

// define the settings for SPI writing
SPISettings writeSettings(SPISPEED, MSBFIRST, SPI_MODE0);

void writeRegister(uint8_t addr, uint8_t value)
{
	SPI.beginTransaction(writeSettings);
	digitalWrite(pins.ss, LOW);					// Select Receiver
	
	SPI.transfer((addr | 0x80) & 0xFF);
	SPI.transfer(value & 0xFF);
	
    digitalWrite(pins.ss, HIGH);				// Unselect Receiver
	
	SPI.endTransaction();
}


// ----------------------------------------------------------------------------------------
// Write a buffer to a register with address addr. 
// Function writes one byte at a time.
// Parameters:
//	addr: SPI address to write to
//	value: The value to write to address
// Returns:
//	<void>
// ----------------------------------------------------------------------------------------

void writeBuffer(uint8_t addr, uint8_t *buf, uint8_t len)
{
	//noInterrupts();							// XXX
	
	SPI.beginTransaction(writeSettings);
	digitalWrite(pins.ss, LOW);					// Select Receiver
	
	SPI.transfer((addr | 0x80) & 0xFF);			// write buffer address
	for (uint8_t i=0; i<len; i++) {
		SPI.transfer(buf[i] & 0xFF);
	}
    digitalWrite(pins.ss, HIGH);				// Unselect Receiver
	
	SPI.endTransaction();
}

// ----------------------------------------------------------------------------------------
//  setRate is setting rate and spreading factor and CRC etc. for transmission
//  for example
//		Modem Config 1 (MC1) == 0x72 for sx1276
//		Modem Config 2 (MC2) == (CRC_ON) | (sf<<4)
//		Modem Config 3 (MC3) == 0x04 | (optional SF11/12 LOW DATA OPTIMIZE 0x08)
//		sf == SF7 default 0x07, (SF7<<4) == SX72_MC2_SF7
//		bw == 125 == 0x70
//		cr == CR4/5 == 0x02
//		CRC_ON == 0x04
//
//	sf is SF7 to SF12
//	CRC is 0x00 (off) or 
// ----------------------------------------------------------------------------------------

void setRate(uint8_t sf, uint8_t crc) 
{
	uint8_t mc1=0, mc2=0, mc3=0;
#	if _MONITOR>=2
	if ((sf<SF7) || (sf>SF12)) {
		if (( debug>=1 ) && ( pdebug & P_RADIO )) {
			mPrint("setRate:: SF=" + String(sf));
		}
		return;
	}
#	endif //_MONITOR

	// Set rate based on Spreading Factor etc
    if (sx1272) {
		mc1= 0x0A;				// SX1276_MC1_BW_250 0x80 | SX1276_MC1_CR_4_5 0x02
		mc2= ((sf<<4) | crc) % 0xFF;
		// SX1276_MC1_BW_250 0x80 | SX1276_MC1_CR_4_5 0x02 | SX1276_MC1_IMPLICIT_HEADER_MODE_ON 0x01
        if (sf == SF11 || sf == SF12) { 
			mc1= 0x0B; 
		}			        
    }
	
	// For sx1276 chips is the CRC ON is 
	else {
		uint8_t bw = 0;				// bw setting is in freqs[ifreq].dwnBw
		uint8_t cr = 0;				// cr settings dependent on SF setting
		//switch (
		
		if (sf==SF8) {
			mc1= 0x78;				// SX1276_MC1_BW_125==0x70 | SX1276_MC1_CR_4_8==0x08
		}
		else {
			mc1= 0x72;				// SX1276_MC1_BW_125==0x70 | SX1276_MC1_CR_4_5==0x02
		}
		mc2= ((sf<<4) | crc) & 0xFF; // crc is 0x00 or 0x04==SX1276_MC2_RX_PAYLOAD_CRCON
		mc3= 0x04;					// 0x04; SX1276_MC3_AGCAUTO
        if (sf == SF11 || sf == SF12) { mc3|= 0x08; }		// 0x08 | 0x04
    }
	
	// Implicit Header (IH), for CLASS B beacons (&& SF6)
	//if (getIh(LMIC.rps)) {
    //   mc1 |= SX1276_MC1_IMPLICIT_HEADER_MODE_ON;
    //    writeRegister(REG_PAYLOAD_LENGTH, getIh(LMIC.rps)); // required length
    //}
	
	writeRegister(REG_MODEM_CONFIG1, (uint8_t) mc1);
	writeRegister(REG_MODEM_CONFIG2, (uint8_t) mc2);
	writeRegister(REG_MODEM_CONFIG3, (uint8_t) mc3);
	
	// Symbol timeout settings
    if (sf == SF10 || sf == SF11 || sf == SF12) {
        writeRegister(REG_SYMB_TIMEOUT_LSB, (uint8_t) 0x05);
    } else {
        writeRegister(REG_SYMB_TIMEOUT_LSB, (uint8_t) 0x08);
    }
	return;
}


// ----------------------------------------------------------------------------------------
// Set the frequency for our gateway
// The function has no parameter other than the freq setting used in init.
// Since we are using a 1ch gateway this value is set fixed.
// ----------------------------------------------------------------------------------------

void  setFreq(uint32_t freq)
{
    // set frequency
    uint64_t frf = ((uint64_t)freq << 19) / 32000000;
    writeRegister(REG_FRF_MSB, (uint8_t)(frf>>16) );
    writeRegister(REG_FRF_MID, (uint8_t)(frf>> 8) );
    writeRegister(REG_FRF_LSB, (uint8_t)(frf>> 0) );
	
	return;
}


// ----------------------------------------------------------------------------------------
//	Set Power for our gateway
// ----------------------------------------------------------------------------------------
void setPow(uint8_t powe)
{
	if (powe >= 16) powe = 15;
	//if (powe >= 15) powe = 14;
	else if (powe < 2) powe =2;
	
	ASSERT((powe>=2)&&(powe<=15));
	
	uint8_t pac = (0x80 | (powe & 0xF)) & 0xFF;
	writeRegister(REG_PAC, (uint8_t)pac);								// set 0x09 to pac
	
	// Note: Power settings for CFG_sx1272 are different
	
	return;
}



// ----------------------------------------------------------------------------------------
// Set the opmode to a value as defined on top
// Values are 0x00 to 0x07
// The value is set for the lowest 3 bits, the other bits are as before.
// ----------------------------------------------------------------------------------------
void  opmode(uint8_t mode)
{
	if (mode == OPMODE_LORA) 
		writeRegister(REG_OPMODE, (uint8_t) mode);
	else
		writeRegister(REG_OPMODE, (uint8_t)((readRegister(REG_OPMODE) & ~OPMODE_MASK) | mode));
}

// ----------------------------------------------------------------------------------------
// Hop to next frequency as defined by NUM_HOPS
// This function should only be used for receiver operation. The current
// receiver frequency is determined by ifreq index like so: freqs[ifreq] 
// ----------------------------------------------------------------------------------------
void hop() {

	// 1. Set radio to standby
	opmode(OPMODE_STANDBY);
		
	// 3. Set frequency based on value in freq		
	ifreq = (ifreq + 1) % NUM_HOPS ;							// Increment the freq round robin
	setFreq(freqs[ifreq].upFreq);
	
	// 4. Set spreading Factor
	sf = SF7;													// Starting the new frequency 
	setRate(sf, 0x40);											// set the sf to SF7 
		
	// Low Noise Amplifier used in receiver
	writeRegister(REG_LNA, (uint8_t) LNA_MAX_GAIN);  			// 0x0C, 0x23
	
	// 7. set sync word
	writeRegister(REG_SYNC_WORD, (uint8_t) 0x34);				// set 0x39 to 0x34 LORA_MAC_PREAMBLE
	
	// prevent node to node communication
	writeRegister(REG_INVERTIQ,0x27);							// 0x33, 0x27; to reset from TX
	
	// Max Payload length is dependent on 256 byte buffer. At startup TX starts at
	// 0x80 and RX at 0x00. RX therefore maximized at 128 Bytes
	writeRegister(REG_MAX_PAYLOAD_LENGTH,MAX_PAYLOAD_LENGTH);	// set 0x23 to 0x80==128 bytes
	writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);			// 0x22, 0x40==64Byte long
	
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_RX_BASE_AD));	// set reg 0x0D to 0x0F
	writeRegister(REG_HOP_PERIOD,0x00);							// reg 0x24, set to 0x00

	// 5. Config PA Ramp up time								// set reg 0x0A  
	writeRegister(REG_PARAMP, (readRegister(REG_PARAMP) & 0xF0) | 0x08); // set PA ramp-up time 50 uSec
	
	// Set 0x4D PADAC for SX1276 ; XXX register is 0x5a for sx1272
	writeRegister(REG_PADAC_SX1276,  0x84); 					// set 0x4D (PADAC) to 0x84
	
	// 8. Reset interrupt Mask, enable all interrupts
	writeRegister(REG_IRQ_FLAGS_MASK, 0x00);
		
	// 9. clear all radio IRQ flags
    writeRegister(REG_IRQ_FLAGS, 0xFF);
	
	// Be aware that micros() has increased significantly from calling 
	// the hop function until printed below
	//
#	if _MONITOR>=1
	if (( debug>=2 ) && ( pdebug & P_RADIO )){
			String response = "hop:: hopTime:: " + String(micros() - hopTime);
			mStat(0, response);
			mPrint(response);
	}
#	endif //_MONITOR
	// Remember the last time we hop
	hopTime = micros();									// At what time did we hop
}
	

// ----------------------------------------------------------------------------------------
// UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP
// This LoRa function reads a message from the LoRa transceiver
// on Success: returns message length read when message correctly received.
// on Failure: it returns a negative value on error (CRC error for example).
// This is the "lowlevel" receive function called by stateMachine()
// dealing with the radio specific LoRa functions.
//
// |         | CR = 4/8  | CR= Coding Rate      |             |
// |Preamble |Header| CRC| Payload              | Payload CRC |
// |---------|------|----|----------------------|-------------|
//
// The function deals with Explicit Header Mode. Implicit header mode is only
// valid for SF6 and only for Uplink so it does not work for LoraWAN.
//
// Parameters:
//		Payload: uint8_t[] message. when message is read it is returned in payload.
// Returns:
//		Length of payload received
//
// 9 bytes header (Explicit Header mode only)
//	Code Rate is 4/8 for Header, might be different for Payload content
//
//	1: Payload Length (in bytes)
//	2: Forward Error Correction Rate
//	3: Optional 16 Bit (2 Byte) PHDR_CRC for the PHDR (Physical Header)
//
// followed by data N bytes Payload (Max 255)
//
// 4 bytes MIC end 
//
// ----------------------------------------------------------------------------------------
uint8_t receivePkt(uint8_t *payload)
{

    statc.msg_ttl++;													// Receive statistics counter

    uint8_t irqflags = readRegister(REG_IRQ_FLAGS);						// 0x12; read back flags											
	uint8_t crcUsed = readRegister(REG_HOP_CHANNEL);					// Is CRC used? (Register 0x1C)
	if (crcUsed & 0x40) {
#	if _DUSB>=1
		if (( debug>=2) && (pdebug & P_RX )) {
			Serial.println(F("R rxPkt:: CRC used"));
		}
#	endif //_DUSB
	}
	
    //  Check for payload IRQ_LORA_CRCERR_MASK=0x20 set
    if (irqflags & IRQ_LORA_CRCERR_MASK)								// Is CRC error?
    {
#		if _DUSB>=1
        if (( debug>=0) && ( pdebug & P_RADIO )) {
			Serial.print(F("rxPkt:: Err CRC, t="));
			SerialTime();
			Serial.println();
		}
#		endif //_DUSB
		return 0;
    }
	
	// Is header OK?
	// Please note that if we reset the HEADER interrupt in RX,
	// that we would here conclude that there is no HEADER
	else if ((irqflags & IRQ_LORA_HEADER_MASK) == false)				// Header not ok?
    {
#		if _DUSB>=1
			if (( debug>=0) && ( pdebug & P_RADIO )) {
				Serial.println(F("rxPkt:: Err HEADER"));
			}
#		endif //_DUSB
		// Reset VALID-HEADER flag 0x10
        writeRegister(REG_IRQ_FLAGS, (uint8_t)(IRQ_LORA_HEADER_MASK  | IRQ_LORA_RXDONE_MASK));	// 0x12; clear HEADER (== 0x10) flag
        return 0;
    }
	
	// If there are no error messages, read the buffer from the FIFO
	// This means "Set FifoAddrPtr to FifoRxBaseAddr"
	else {
        statc.msg_ok++;													// Receive OK statistics counter
		switch(statr[0].ch) {
			case 0: statc.msg_ok_0++; break;
			case 1: statc.msg_ok_1++; break;
			case 2: statc.msg_ok_2++; break;
		}

		if (readRegister(REG_FIFO_RX_CURRENT_ADDR) != readRegister(REG_FIFO_RX_BASE_AD)) {
#if			_DUSB>=1
			if (( debug>=1 ) && ( pdebug & P_RADIO )) {
				Serial.print(F("RX BASE <"));
				Serial.print(readRegister(REG_FIFO_RX_BASE_AD));
				Serial.print(F("> != RX CURRENT <"));
				Serial.print(readRegister(REG_FIFO_RX_CURRENT_ADDR));
				Serial.print(F(">"));
				Serial.println();
			}
#			endif //_DUSB
		}
		
        //uint8_t currentAddr = readRegister(REG_FIFO_RX_CURRENT_ADDR);	// 0x10
		uint8_t currentAddr = readRegister(REG_FIFO_RX_BASE_AD);		// 0x0F
        uint8_t receivedCount = readRegister(REG_RX_NB_BYTES);			// 0x13; How many bytes were read
#		if _DUSB>=1
			if ((debug>=1) && (currentAddr > 64)) {						// More than 64 read?
				Serial.print(F("rxPkt:: Rx addr>64"));
				Serial.println(currentAddr);
			}
#		endif //_DUSB
        writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) currentAddr);		// 0x0D 

		if (receivedCount > PAYLOAD_LENGTH) {
#			if _DUSB>=1
				if (( debug>=1 ) & ( pdebug & P_RADIO )) {
					Serial.print(F("rxPkt:: receivedCount="));
					Serial.println(receivedCount);
				}
#			endif //_DUSB
			receivedCount=PAYLOAD_LENGTH;
		}

        for(int i=0; i < receivedCount; i++)
        {
            payload[i] = readRegister(REG_FIFO);						// 0x00, FIFO will auto shift register
        }

		writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);					// Reset ALL interrupts
		
		// A long as _DUSB is enabled, and P_RX debug messages are selected,
		// the received packet is displayed on the output.
#		if _DUSB>=1
		if (( debug>=1 ) && ( pdebug & P_RX )){
		
			Serial.print(F("rxPkt:: t="));
			SerialTime();
			
			Serial.print(F(", f="));
			Serial.print(ifreq);
			Serial.print(F(", sf="));
			Serial.print(sf);
			Serial.print(F(", a="));
			if (payload[4]<0x10) Serial.print('0'); Serial.print(payload[4], HEX);
			if (payload[3]<0x10) Serial.print('0'); Serial.print(payload[3], HEX);
			if (payload[2]<0x10) Serial.print('0'); Serial.print(payload[2], HEX);
			if (payload[1]<0x10) Serial.print('0'); Serial.print(payload[1], HEX);
			Serial.print(F(", flags="));
			Serial.print(irqflags,HEX);
			Serial.print(F(", addr="));
			Serial.print(currentAddr);
			Serial.print(F(", len="));
			Serial.print(receivedCount);

			// If debug level 1 is specified, we display the content of the message as well
			// We need to decode the message as well will it make any sense

#			if _TRUSTED_DECODE>=2
			if (debug>=1)  {							// Must be 1 for operational use

				int index;								// The index of the codex struct to decode
				String response="";

				uint8_t data[receivedCount];
				
				uint8_t DevAddr [4];
					DevAddr[0] = payload[4];
					DevAddr[1] = payload[3];
					DevAddr[2] = payload[2];
					DevAddr[3] = payload[1];
			
				if ((index = inDecodes((char *)(payload+1))) >=0 ) {	
					Serial.print(F(", Ind="));
					Serial.print(index);
					//Serial.println();
				}
				else if (debug>=1) {	
					Serial.print(F(", No Index"));
					Serial.println();
					return(receivedCount);
				}	

				// ------------------------------
				
				Serial.print(F(", data="));

				for (int i=0; i<receivedCount; i++) { 			// Copy array
					data[i] = payload[i]; 
				}

				uint16_t frameCount=payload[7]*256 + payload[6];
				
				// The message received has a length, but data starts at byte 9, and stops 4 bytes
				// before the end since those are MIC bytes
				uint8_t CodeLength = encodePacket((uint8_t *)(data + 9), receivedCount-9-4, (uint16_t)frameCount, DevAddr, decodes[index].appKey, 0);

				Serial.print(F("- NEW fc="));
				Serial.print(frameCount);
				Serial.print(F(", addr="));
				
				for (int i=0; i<4; i++) {
					if (DevAddr[i]<=0xF) {
						Serial.print('0');
					}
					Serial.print(DevAddr[i], HEX);
					Serial.print(' ');
				}
				
				Serial.print(F(", len="));
				Serial.print(CodeLength);
				Serial.print(F(", data="));

				for (int i=0; i<receivedCount; i++) {
					if (data[i]<=0xF) Serial.print('0');
					Serial.print(data[i], HEX);
					Serial.print(' ');
				}
			|
#			endif // _TRUSTED_DECODE
			
			Serial.println();
			
			if (debug>=2) Serial.flush();
		}
#		endif //DUSB
		return(receivedCount);
    }

	writeRegister(REG_IRQ_FLAGS, (uint8_t) (
		IRQ_LORA_RXDONE_MASK | 
		IRQ_LORA_RXTOUT_MASK |
		IRQ_LORA_HEADER_MASK | 
		IRQ_LORA_CRCERR_MASK));							// 0x12; Clear RxDone IRQ_LORA_RXDONE_MASK
    return 0;
} //receivePkt UP
	
	
	
// ----------------------------------------------------------------------------------------
// DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN 
// This DOWN function sends a payload to the LoRa node over the air
// Radio must go back in standby mode as soon as the transmission is finished
// 
// NOTE:: writeRegister functions should not be used outside interrupts
// ----------------------------------------------------------------------------------------
bool sendPkt(uint8_t *payLoad, uint8_t payLength)
{
#	if _DUSB>=2
	if (payLength>=128) {
		if (debug>=1) {
			Serial.print("sendPkt:: len=");
			Serial.println(payLength);
		}
		return false;
	}
#	endif
	
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_TX_BASE_AD));	// 0x0D, 0x0E
	
	writeRegister(REG_PAYLOAD_LENGTH, (uint8_t) payLength);				// 0x22
	payLoad[payLength] = 0x00;											// terminate buffer
	writeBuffer(REG_FIFO, (uint8_t *) payLoad, payLength);
	
	return true;
}

// ----------------------------------------------------------------------------------------
// loraWait()
// This function implements the wait protocol needed for downstream transmissions.
// Note: Timing of downstream and JoinAccept messages is VERY critical.
//
// As the ESP8266 watchdog will not like us to wait more than a few hundred
// milliseconds (or it will kick in) we have to implement a simple way to wait
// time in case we have to wait seconds before sending messages (e.g. for OTAA 5 or 6 seconds)
// Without it, the system is known to crash in half of the cases it has to wait for 
// JOIN-ACCEPT messages to send.
//
// This function uses a combination of delay() statements and delayMicroseconds().
// As we use delay() only when there is still enough time to wait and we use micros()
// to make sure that delay() did not take too much time this works.
// 
// Parameter: uint32-t tmst gives the micros() value when transmission should start. (!!!)
// Note: We assume LoraDown.sfTx contains the SF we will use for downstream message.
// ----------------------------------------------------------------------------------------

void loraWait(const uint32_t timestamp)
{
	uint32_t startMics = micros();						// Start of the loraWait function
	uint32_t tmst = timestamp;

	int32_t adjust=0;
	switch (LoraDown.sfTx) {
		case 7: adjust= 60000; break;					// Make time for SF7 longer 
		case 8: break;									// Around 60ms
		case 9: break;
		case 10: break;
		case 11: break;
		case 12: break;
		default:
#		if _DUSB>=1
		if (( debug>=1 ) && ( pdebug & P_TX )) {
			Serial.print(F("T loraWait:: unknown SF="));
			Serial.print(LoraDown.sfTx);
		}
#		endif
		break;
	}
	tmst = tmst + gwayConfig.txDelay + adjust;			// tmst based on txDelay and spreading factor
	uint32_t waitTime = tmst - micros();				// Or make waitTime an unsigned and change next statement
	if (micros()>tmst) {								// to (waitTime<0)
		Serial.println(F("loraWait:: Error wait time < 0"));
		return;
	}
	
	// For larger delay times we use delay() since that is for > 15ms
	// This is the most efficient way
	while (waitTime > 16000) {
		delay(15);										// ms delay including yield, slightly shorter
		waitTime= tmst - micros();
	}
	// The remaining wait time is less tan 15000 uSecs
	// And we use delayMicroseconds() to wait
	if (waitTime>0) delayMicroseconds(waitTime);

#	if _DUSB>=1
	else if ((waitTime+20) < 0) {
		Serial.println(F("loraWait:: TOO LATE"));		// Never happens
	}

	if (( debug>=2 ) && ( pdebug & P_TX )) { 
		Serial.print(F("T start: ")); 
		Serial.print(startMics);
		Serial.print(F(", tmst: "));					// tmst
		Serial.print(tmst);
		Serial.print(F(", end: "));						// This must be micros(), and equal to tmst
		Serial.print(micros());
		Serial.print(F(", waited: "));
		Serial.print(tmst - startMics);
		Serial.print(F(", delay="));
		Serial.print(gwayConfig.txDelay);
		Serial.println();
		if (debug>=2) Serial.flush();
	}
#	endif
}


// ----------------------------------------------------------------------------------------
// txLoraModem
// Init the transmitter and transmit the buffer
// After successful transmission (dio0==1) TxDone re-init the receiver
//
//	crc is set to 0x00 for TX
//	iiq is set to 0x27 (or 0x40 based on ipol value in txpkt)
//
//	1. opmode Lora (only in Sleep mode)
//	2. opmode StandBY
//	3. Configure Modem
//	4. Configure Channel
//	5. write PA Ramp
//	6. config Power
//	7. RegLoRaSyncWord LORA_MAC_PREAMBLE
//	8. write REG dio mapping (dio0)
//	9. write REG IRQ flags
// 10. write REG IRQ mask
// 11. write REG LoRa Fifo Base Address
// 12. write REG LoRa Fifo Addr Ptr
// 13. write REG LoRa Payload Length
// 14. Write buffer (byte by byte)
// 15. Wait until the right time to transmit has arrived
// 16. opmode TX
// ----------------------------------------------------------------------------------------

void txLoraModem(
				uint8_t *payLoad, 		// Payload contents
				uint8_t payLength, 		// Length of payload
				uint32_t tmst, 			// Timestamp
				uint8_t sfTx,			// Sending spreading factor
				uint8_t powe, 			// power
				uint32_t freq, 			// frequency
				uint8_t crc, 			// Do we use CRC error checking
				uint8_t iiq				// Interrupt
				)
{
#	if _DUSB>=2
	if (debug>=1) {
		// Make sure that all serial stuff is done before continuing
		Serial.print(F("txLoraModem::"));
		Serial.print(F("  powe: ")); Serial.print(powe);
		Serial.print(F(", freq: ")); Serial.print(freq);
		Serial.print(F(", crc: ")); Serial.print(crc);
		Serial.print(F(", iiq: 0X")); Serial.print(iiq,HEX);
		Serial.println();
		if (debug>=2) Serial.flush();
	}
#	endif

	_state = S_TX;
		
	// 1. Select LoRa modem from sleep mode
	//opmode(OPMODE_LORA);									// set register 0x01 to 0x80
	
	// Assert the value of the current mode
	ASSERT((readRegister(REG_OPMODE) & OPMODE_LORA) != 0);
	
	// 2. enter standby mode (required for FIFO loading))
	opmode(OPMODE_STANDBY);									// set 0x01 to 0x01
	
	// 3. Init spreading factor and other Modem setting
	setRate(sfTx, crc);
	
	// Frequency hopping
	//writeRegister(REG_HOP_PERIOD, (uint8_t) 0x00);		// set 0x24 to 0x00 only for receivers
	
	// 4. Init Frequency, config channel
	setFreq(freq);

	// 6. Set power level, REG_PAC
	setPow(powe);
	
	// 7. prevent node to node communication
	writeRegister(REG_INVERTIQ, (uint8_t) iiq);					// 0x33, (0x27 or 0x40)
	
	// 8. set the IRQ mapping DIO0=TxDone DIO1=NOP DIO2=NOP (or less for 1ch gateway)
    writeRegister(REG_DIO_MAPPING_1, (uint8_t)(
		MAP_DIO0_LORA_TXDONE | 
		MAP_DIO1_LORA_NOP | 
		MAP_DIO2_LORA_NOP |
		MAP_DIO3_LORA_CRC));
	
	// 9. clear all radio IRQ flags
    writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);
	
	// 10. mask all IRQs but TxDone
    writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) ~IRQ_LORA_TXDONE_MASK);
	
	// txLora
	opmode(OPMODE_FSTX);									// set 0x01 to 0x02 (actual value becomes 0x82)
	
	// 11, 12, 13, 14. write the buffer to the FiFo
	sendPkt(payLoad, payLength);

	// 15. wait extra delay out. The delayMicroseconds timer is accurate until 16383 uSec.
	loraWait(tmst);
	
	//Set the base addres of the transmit buffer in FIFO
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_TX_BASE_AD));	// set 0x0D to 0x0F (contains 0x80);	
	
	//For TX we have to set the PAYLOAD_LENGTH
	writeRegister(REG_PAYLOAD_LENGTH, (uint8_t) payLength);		// set 0x22, max 0x40==64Byte long
	
	//For TX we have to set the MAX_PAYLOAD_LENGTH
	writeRegister(REG_MAX_PAYLOAD_LENGTH, (uint8_t) MAX_PAYLOAD_LENGTH);	// set 0x22, max 0x40==64Byte long
	
	// Reset the IRQ register
	writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);			// Clear the mask
	writeRegister(REG_IRQ_FLAGS, (uint8_t) IRQ_LORA_TXDONE_MASK);// set 0x12 to 0x08, clear TXDONE
	
	// 16. Initiate actual transmission of FiFo
	opmode(OPMODE_TX);											// set 0x01 to 0x03 (actual value becomes 0x83)
	
}// txLoraModem


// ----------------------------------------------------------------------------------------
// Setup the LoRa receiver on the connected transceiver.
// - Determine the correct transceiver type (sx1272/RFM92 or sx1276/RFM95)
// - Set the frequency to listen to (1-channel remember)
// - Set Spreading Factor (standard SF7)
// The reset RST pin might not be necessary for at least the RFM95 transceiver
//
// 1. Put the radio in LoRa mode
// 2. Put modem in sleep or in standby
// 3. Set Frequency
// 4. Spreading Factor
// 5. Set interrupt mask
// 6. 
// 7. Set opmode to OPMODE_RX
// 8. Set _state to S_RX
// 9. Reset all interrupts
// ----------------------------------------------------------------------------------------

void rxLoraModem()
{
	// 1. Put system in LoRa mode
	//opmode(OPMODE_LORA);										// Is already so
	
	// 2. Put the radio in sleep mode
	opmode(OPMODE_STANDBY);										// CAD set 0x01 to 0x00
	
	// 3. Set frequency based on value in freq
	setFreq(freqs[ifreq].upFreq);										// set to the right frequency

	// 4. Set spreading Factor and CRC
    setRate(sf, 0x04);
	
	// prevent node to node communication
	writeRegister(REG_INVERTIQ, (uint8_t) 0x27);				// 0x33, 0x27; to reset from TX
	
	// Max Payload length is dependent on 256 byte buffer. 
	// At startup TX starts at 0x80 and RX at 0x00. RX therefore maximized at 128 Bytes
	//For TX we have to set the PAYLOAD_LENGTH
    //writeRegister(REG_PAYLOAD_LENGTH, (uint8_t) PAYLOAD_LENGTH);	// set 0x22, 0x40==64Byte long

	// Set CRC Protection for MAX payload protection
	//writeRegister(REG_MAX_PAYLOAD_LENGTH, (uint8_t) MAX_PAYLOAD_LENGTH);	// set 0x23 to 0x80==128
	
	//Set the start address for the FiFO (Which should be 0)
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_RX_BASE_AD));	// set 0x0D to 0x0F (contains 0x00);
	
	// Low Noise Amplifier used in receiver
	writeRegister(REG_LNA, (uint8_t) LNA_MAX_GAIN);  						// 0x0C, 0x23
	
	// 5. Accept no interrupts except RXDONE, RXTOUT en RXCRC
	writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) ~(
		IRQ_LORA_RXDONE_MASK | 
		IRQ_LORA_RXTOUT_MASK | 
		IRQ_LORA_HEADER_MASK | 
		IRQ_LORA_CRCERR_MASK));

	// set frequency hopping
	if (_hop) {
		//writeRegister(REG_HOP_PERIOD, 0x01);					// 0x24, 0x01 was 0xFF
		writeRegister(REG_HOP_PERIOD,0x00);						// 0x24, 0x00 was 0xFF
	}
	else {
		writeRegister(REG_HOP_PERIOD,0x00);						// 0x24, 0x00 was 0xFF
	}
	// Set RXDONE interrupt to dio0
	writeRegister(REG_DIO_MAPPING_1, (uint8_t)(
			MAP_DIO0_LORA_RXDONE | 
			MAP_DIO1_LORA_RXTOUT |
			MAP_DIO2_LORA_NOP |			
			MAP_DIO3_LORA_CRC));

	// 7+8.  Set the opmode to either single or continuous receive. The first is used when
	// every message can come on a different SF, the second when we have fixed SF
	if (_cad) {
		// cad Scanner setup, set _state to S_RX
		// Set Single Receive Mode, and go in STANDBY mode after receipt
		_state= S_RX;											// 8.
		opmode(OPMODE_RX_SINGLE);								// 7. 0x80 | 0x06 (listen one message)
	}
	else {
		// Set Continous Receive Mode, useful if we stay on one SF
		_state= S_RX;											// 8.
#		if _DUSB>=1
		if (_hop) {
			Serial.println(F("rxLoraModem:: ERROR continuous receive in hop mode"));
		}
#		endif
		opmode(OPMODE_RX);										// 7. 0x80 | 0x05 (listen)
	}
	
	// 9. clear all radio IRQ flags
    writeRegister(REG_IRQ_FLAGS, 0xFF);
	
	return;
}// rxLoraModem


// ----------------------------------------------------------------------------------------
// function cadScanner()
//
// CAD Scanner will scan on the given channel for a valid Symbol/Preamble signal.
// So instead of receiving continuous on a given channel/sf combination
// we will wait on the given channel and scan for a preamble. Once received
// we will set the radio to the SF with best rssi (indicating reception on that sf).
// The function sets the _state to S_SCAN
// NOTE: DO not set the frequency here but use the frequency hopper
// ----------------------------------------------------------------------------------------
void cadScanner()
{
	// 1. Put system in LoRa mode (which destroys all other modes)
	//opmode(OPMODE_LORA);
	
	// 2. Put the radio in sleep mode
	opmode(OPMODE_STANDBY);										// Was old value
	
	// 3. Set frequency based on value in ifreq					// XXX New, might be needed when receiving down 
	setFreq(freqs[ifreq].upFreq);								// set to the right frequency

	// For every time we start the scanner, we set the SF to the begin value
	//sf = SF7;													// XXX 180501 Not by default
	
	// 4. Set spreading Factor and CRC
	setRate(sf, 0x04);
	
	// listen to LORA_MAC_PREAMBLE
	writeRegister(REG_SYNC_WORD, (uint8_t) 0x34);				// set reg 0x39 to 0x34
	
	// Set the interrupts we want to listen to
	writeRegister(REG_DIO_MAPPING_1, (uint8_t)(
		MAP_DIO0_LORA_CADDONE | 
		MAP_DIO1_LORA_CADDETECT | 
		MAP_DIO2_LORA_NOP | 
		MAP_DIO3_LORA_CRC ));
	
	// Set the mask for interrupts (we do not want to listen to) except for
	writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) ~(
		IRQ_LORA_CDDONE_MASK | 
		IRQ_LORA_CDDETD_MASK | 
		IRQ_LORA_CRCERR_MASK | 
		IRQ_LORA_HEADER_MASK));
	
	// Set the opMode to CAD
	opmode(OPMODE_CAD);

	// Clear all relevant interrupts
	//writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF );						// May work better, clear ALL interrupts
	
	// If we are here. we either might have set the SF or we have a timeout in which
	// case the receive is started just as normal.
	return;
	
}// cadScanner


// ----------------------------------------------------------------------------------------
// First time initialisation of the LoRa modem
// Subsequent changes to the modem state etc. done by txLoraModem or rxLoraModem
// After initialisation the modem is put in rx mode (listen)
//	1.	Set Radio to sleep
//	2.	Set opmode to LoRa
//	3.	Set Frequency
//	4.	Set rate and Spreading Factor
//	5.	Set chip version
//	6.	Set SYNC word
//	7.	Set ranp-up time
//	8.	Set interrupt masks
//	9.	Clear INT flags
// ----------------------------------------------------------------------------------------
void initLoraModem(
				)
{
	_state = S_INIT;
#if ESP32_ARCH==1
	digitalWrite(pins.rst, LOW);
	delayMicroseconds(10000);
    digitalWrite(pins.rst, HIGH);
	delayMicroseconds(10000);
	digitalWrite(pins.ss, HIGH);

#else
	// Reset the transceiver chip with a pulse of 10 mSec
	digitalWrite(pins.rst, HIGH);
	delayMicroseconds(10000);
    digitalWrite(pins.rst, LOW);
	delayMicroseconds(10000);
#endif
	// 1. Set radio to sleep
	opmode(OPMODE_SLEEP);										// set register 0x01 to 0x00

	// 2 Set LoRa Mode
	opmode(OPMODE_LORA);										// set register 0x01 to 0x80
	
	// 3. Set frequency based on value in freq
	setFreq(freqs[ifreq].upFreq);												// set to 868.1MHz or the last saved frequency
	
	// 4. Set spreading Factor
    setRate(sf, 0x04);
	
	// Low Noise Amplifier used in receiver
    writeRegister(REG_LNA, (uint8_t) LNA_MAX_GAIN);  			// 0x0C, 0x23
#	if _PIN_OUT==4
		delay(1);
#	endif

	// 5. Set chip type/version
    uint8_t version = readRegister(REG_VERSION);				// Read the LoRa chip version id
    if (version == 0x22) {
        // sx1272
#		if _DUSB>=2
			Serial.println(F("WARNING:: SX1272 detected"));
#		endif
        sx1272 = true;
    } 
	
	else if (version == 0x12) {
        // sx1276?
#			if _DUSB>=2
            if (debug >=1) 
				Serial.println(F("SX1276 starting"));
#			endif
            sx1272 = false;
	}
	else {
		// Normally this means that we connected the wrong type of board and
		// therefore specified the wrong type of wiring/pins to the software
		// Maybe this issue can be resolved of we try one of the other defined 
		// boards. (Comresult or Hallard or ...)
#		if _DUSB>=1
			Serial.print(F("Unknown transceiver="));
			Serial.print(version,HEX);
			Serial.print(F(", pins.rst ="));	Serial.print(pins.rst);
			Serial.print(F(", pins.ss  ="));	Serial.print(pins.ss);
			Serial.print(F(", pins.dio0 ="));	Serial.print(pins.dio0);
			Serial.print(F(", pins.dio1 ="));	Serial.print(pins.dio1);
			Serial.print(F(", pins.dio2 ="));	Serial.print(pins.dio2);
			Serial.println();
			Serial.flush();
#		endif
		die("");												// Maybe first try another kind of receiver
    }
	// If we are here, the chip is recognized successfully
	
	// 6. set sync word
	writeRegister(REG_SYNC_WORD, (uint8_t) 0x34);				// set 0x39 to 0x34 LORA_MAC_PREAMBLE
	
	// prevent node to node communication
	writeRegister(REG_INVERTIQ,0x27);							// 0x33, 0x27; to reset from TX
	
	// Max Payload length is dependent on 256 byte buffer. At startup TX starts at
	// 0x80 and RX at 0x00. RX therefore maximized at 128 Bytes
	writeRegister(REG_MAX_PAYLOAD_LENGTH,MAX_PAYLOAD_LENGTH);	// set 0x23 to 0x80==128 bytes
	writeRegister(REG_PAYLOAD_LENGTH,PAYLOAD_LENGTH);			// 0x22, 0x40==64Byte long
	
	writeRegister(REG_FIFO_ADDR_PTR, (uint8_t) readRegister(REG_FIFO_RX_BASE_AD));	// set reg 0x0D to 0x0F
	writeRegister(REG_HOP_PERIOD,0x00);							// reg 0x24, set to 0x00

	// 7. Config PA Ramp up time								// set reg 0x0A  
	writeRegister(REG_PARAMP, (readRegister(REG_PARAMP) & 0xF0) | 0x08); // set PA ramp-up time 50 uSec
	
	// Set 0x4D PADAC for SX1276 ; XXX register is 0x5a for sx1272
	writeRegister(REG_PADAC_SX1276,  0x84); 					// set 0x4D (PADAC) to 0x84
	//writeRegister(REG_PADAC, readRegister(REG_PADAC) | 0x4);
	
	// 8. Reset interrupt Mask, enable all interrupts
	writeRegister(REG_IRQ_FLAGS_MASK, 0x00);
	
	// 9. clear all radio IRQ flags
    writeRegister(REG_IRQ_FLAGS, 0xFF);
}// initLoraModem


// ----------------------------------------------------------------------------------------
// Void function startReceiver.
// This function starts the receiver loop of the LoRa service.
// It starts the LoRa modem with initLoraModem(), and then starts
// the receiver either in single message (CAD) of in continuous
// reception (STD).
// ----------------------------------------------------------------------------------------
void startReceiver() {
	initLoraModem();								// XXX 180326, after adapting this function 
	if (_cad) {
#		if _DUSB>=1
		if (( debug>=1 ) && ( pdebug & P_SCAN )) {
			Serial.println(F("S PULL:: _state set to S_SCAN"));
			if (debug>=2) Serial.flush();
		}
#		endif
		_state = S_SCAN;
		sf = SF7;
		cadScanner();
	}
	else {
		_state = S_RX;
		rxLoraModem();
	}
	writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);
	writeRegister(REG_IRQ_FLAGS, 0xFF);				// Reset all interrupt flags
}


// ----------------------------------------------------------------------------------------
// Interrupt_0 Handler.
// Both interrupts DIO0 and DIO1 are mapped on GPIO15. Se we have to look at 
// the interrupt flags to see which interrupt(s) are called.
//
// NOTE:: This method may work not as good as just using more GPIO pins on 
//  the ESP8266 mcu. But in practice it works good enough
// ----------------------------------------------------------------------------------------
void ICACHE_RAM_ATTR Interrupt_0()
{
	_event=1;
}


// ----------------------------------------------------------------------------------------
// Interrupt handler for DIO1 having High Value
// As DIO0 and DIO1 may be multiplexed on one GPIO interrupt handler
// (as we do) we have to be careful only to call the right Interrupt_x
// handler and clear the corresponding interrupts for that dio.
// NOTE: Make sure all Serial communication is only for debug level 3 and up.
// Handler for:
//		- CDDETD
//		- RXTIMEOUT
//		- (RXDONE error only)
// ----------------------------------------------------------------------------------------
void ICACHE_RAM_ATTR Interrupt_1()
{
	_event=1;
}

// ----------------------------------------------------------------------------------------
// Frequency Hopping Channel (FHSS) dio2
// ----------------------------------------------------------------------------------------
void ICACHE_RAM_ATTR Interrupt_2() 
{
	_event=1;
}


