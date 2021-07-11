// 1-channel LoRa Gateway for ESP8266 and ESP32
// Copyright (c) 2016-2020 Maarten Westenberg version for ESP8266
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
// ============================================================================================


// --------------------------------------------------------------------------------------------
// stateMachine handler of the state machine.
// We use ONE state machine for all kind of interrupts. This assures that we take
// the correct action upon receiving an interrupt.
//
// _event is the software interrupt: If set this function is executed from loop(),
// the function should itself take care of setting or resetting the variable.
//
// STATE MACHINE
// The program uses the following state machine (in _state), all states
// are done in interrupt routine, only the follow-up of S-RXDONE is done
// in the main loop() program. This is because otherwise the interrupt processing
// would take too long to finish
//
// So _state has one of the following state values:
//
// S-INIT=0, 	The commands in this state are executed only once
//	- Goto S_SCAN
//
// S-SCAN, 		CadScanner() part
//	- Upon CDDECT (int1) goto S_RX, 
//	- upon CDDONE (int0) goto S_CAD, walk through all SF until CDDETD
//	- Else stay in SCAN state
//
// S-CAD, 		
//	- Upon CDDECT (int1) goto S_RX, 
//	- Upon CDDONE (int0) goto S_SCAN, start with SF7 recognition again
//
// S-RX, 		Received CDDECT so message detected, RX cycle started. 
//	- Upon RXDONE (int0) package read. If read finished continue to read message, then goto S_SCAN
//	- upon RXTOUT (int1) error, goto S_SCAN
//
// S-TX			Transmitting a message
//	- When transmission done, goto TXDONE
//
// S-TXDONE		Transmission complete by loop() now again in interrupt
//	- Set the Mask
//	- reset the Flags
//	- Goto either SCAN or RX
//
// This interrupt routine has been kept as simple and short as possible.
// If we receive an interrupt that does not belong to a _state then print error.
// _event is a special variable which indicate that an interrupt event has happened
// and we need to take action OR that we generate a soft interrupt for state machine.
// 
// NOTE: We may clear the interrupt but leave the flag for the moment. 
//	The eventHandler should take care of repairing flags between interrupts.
// --------------------------------------------------------------------------------------------

void stateMachine()
{
	// Determine what interrupt flags are set
	//
	uint8_t flags = readRegister(REG_IRQ_FLAGS);
	uint8_t mask  = readRegister(REG_IRQ_FLAGS_MASK);
	uint8_t intr  = flags & ( ~ mask );							// Only react on non masked interrupts
	uint8_t rssi;
	_event=0;													// Reset the interrupt detector	

#	if _MONITOR>=1
	if (intr != flags) {
		String response = "stateMachine:: Error: flags="+String(flags,HEX)+", ";
		mStat(intr, response);
		mPrint(response);
	}
#	endif //_MONITOR

	// If Hopping is selected AND if there is NO event interrupt detected 
	// and the state machine is called anyway
	// then we know its a soft interrupt and we do nothing and return to main loop.
	//
	if ((gwayConfig.hop) && (intr == 0x00))
	{
		// eventWait is the time since we have had a CDDETD event (preamble detected).
		// If we are not in scanning state, and there will be an interrupt coming,
		// In state S_RX it could be RXDONE in which case allow kernel time
		//
		if ((_state == S_SCAN) || (_state == S_CAD)) {

			_event=0;

			uint32_t eventWait = EVENT_WAIT;
			switch (_state) {
				case S_INIT:	eventWait = 0; break;
				// Next two are most important
				case S_SCAN:	eventWait = EVENT_WAIT * 1; break;
				case S_CAD:		eventWait = EVENT_WAIT * 1; break;
				case S_RX:		eventWait = EVENT_WAIT * 8; break;
				case S_TX:		eventWait = EVENT_WAIT * 1; break;
				case S_TXDONE:	eventWait = EVENT_WAIT * 4; break;
				default:
					eventWait=0;
#					if _MONITOR>=1
						String response = "StateMachine:: Default: ";
						mStat(intr, response);
						mPrint(response);
#					endif //_MONITOR
			}

			// doneWait is the time that we received CDDONE interrupt
			// So we init the wait time for RXDONE based on the current SF.
			// As for highter CF it takes longer to receive symbols
			// Assume symbols in SF8 take twice the time of SF7
			//
			uint32_t doneWait = DONE_WAIT;						// Initial value
			switch (sf) {
				case SF7: 	break;
				case SF8: 	doneWait *= 2;	break;
				case SF9: 	doneWait *= 4;	break;
				case SF10:	doneWait *= 8;	break;
				case SF11:	doneWait *= 16;	break;
				case SF12:	doneWait *= 32;	break;
				default:
					doneWait *= 1;
#					if _MONITOR>=1
					if ((debug>=0) && (pdebug & P_PRE)) {
						mPrint("StateMachine:: PRE: DEF set");
					}
#					endif //_MONITOR
					break;
			}

			// If micros is starting over again after 51 minutes 
			// it's value is smaller than an earlier value of eventTime/doneTime
			//
			if (eventTime > micros())	eventTime=micros();
			if (doneTime > micros())	doneTime=micros();

			if (((micros() - doneTime) > doneWait) &&
				((_state == S_SCAN) || (_state == S_CAD)))
			{
				_state = S_SCAN;
				hop();											// increment gwayConfig.ch = (gwayConfig.ch + 1) % NUM_HOPS ;
				cadScanner();									// Reset to initial SF, leave frequency "freqs[gwayConfig.ch]"
#				if _MONITOR>=1
				if ((debug >= 1) && (pdebug & P_PRE)) {
					String response = "DONE  :: ";
					mStat(intr, response);
					mPrint(response);							// Can move down for timing reasons
				}
#				endif //_MONITOR
				eventTime=micros();								// reset the timer on timeout
				doneTime=micros();								// reset the timer on timeout
				return;
			}
			// If timeout occurs and still no _event, then hop
			// and start scanning again
			//
			if ((micros() - eventTime) > eventWait ) 
			{
				_state = S_SCAN;
				hop();											// gwayConfig.ch= (gwayConfig.ch+1)%NUM_HOPS ;
				cadScanner();									// Reset to initial SF, leave "freqs[gwayConfig.ch]"
#				if _MONITOR>=1
				if ((debug >= 2) && (pdebug & P_PRE)) {
					String response = "HOP ::  ";
					mStat(intr, response);
					mPrint(response);
				}
#				endif //_MONITOR
				eventTime=micros();								// reset the eventtimer on timeout
				doneTime=micros();								// reset the timer on timeout
				return;
			}


			// If we are here, NO timeout has occurred 
			// So we need to return to the main State Machine
			// as there was NO interrupt
#			if _MONITOR>=1
			if ((debug>=3) && (pdebug & P_PRE)) {
				String response  = "PRE:: eventTime=";
				response += String(eventTime);
				response += ", micros=";
				response += String(micros());
				response += ": ";
				mStat(intr, response);
				mPrint(response);
			}
#			endif //_MONITOR
		} // if SCAN or CAD

		// else, S_RX of S_TX for example
		else {
			//
		} // else S_RX or S_TX, TXDONE

		yield();										// if hopping is enabled

	}// intr==0 && gwayConfig.hop



	// ==========================================================================================
	// This is the actual state machine of the gateway
	// and its next actions are depending on the state we are in.
	// For hop situations we do not get interrupts, so we have to
	// simulate and generate events ourselves.
	//
	switch (_state) 
	{
	  // ----------------------------------------------------------------------------------------
	  // If the state is init, we are starting up.
	  // The initLoraModem() function is already called in setup();
	  //
	  case S_INIT:
#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_PRE)) { 
			mPrint("S_INIT"); 
		}
#		endif //_MONITOR
		// new state, needed to startup the radio (to S_SCAN)
		writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF );				// Clear ALL interrupts
		writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00 );			// Accept ALL interrupts
		_event=0;
	  break;


	  // ----------------------------------------------------------------------------------------
	  // In S_SCAN we measure a high RSSI this means that there (probably) is a message
	  // coming in at that freq. But not necessarily on the current SF.
	  // If so find the right SF with CDDETD. 
	  //
	  case S_SCAN:
	    //
		// Intr==IRQ_LORA_CDDETD_MASK
		// We detected a message on this frequency and SF when scanning
		// We clear both CDDETD and swich to reading state to read the message
		//
		if (intr & IRQ_LORA_CDDETD_MASK) {

			_state = S_RX;											// Set state to receiving

			// Set RXDONE interrupt to dio0, RXTOUT to dio1
			writeRegister(REG_DIO_MAPPING_1, (
				MAP_DIO0_LORA_RXDONE | 
				MAP_DIO1_LORA_RXTOUT | 
				MAP_DIO2_LORA_NOP | 
				MAP_DIO3_LORA_CRC));

			// Since new state is S_RX, accept no interrupts except RXDONE or RXTOUT
			// HEADER and CRCERR
			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) ~(
				IRQ_LORA_RXDONE_MASK | 
				IRQ_LORA_RXTOUT_MASK | 
				IRQ_LORA_HEADER_MASK | 
				IRQ_LORA_CRCERR_MASK));

			// Starting with version 5.0.1 the waittime is dependent on the SF
			// So for SF12 we wait longer (2^7 == 128 uSec) and for SF7 4 uSec.
			//delayMicroseconds( (0x01 << ((uint8_t)sf - 5 )) );
			//if (gwayConfig.cad) 									// XXX 180520 make sure we start reading asap in hop
			//	delayMicroseconds( RSSI_WAIT );						// Wait some microseconds less

			rssi = readRegister(REG_RSSI);							// Read the RSSI
			_rssi = rssi;											// Read the RSSI in the state variable

			_event=0;												// Make 0, as soon as we have an interrupt
			detTime=micros();										// mark time that preamble detected

#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_PRE)) {
				String response = "SCAN:: ";
				mStat(intr, response);
				mPrint(response);
			}
#			endif //_MONITOR
			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);
			writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);			// reset all interrupt flags
			opmode(OPMODE_RX_SINGLE);								// set reg 0x01 to 0x06 for receiving
			
		}//if

		// CDDONE
		// We received a CDDONE int telling us that we received a message on this
		// frequency and possibly on one of its SF. Only when the incoming message
		// matches the SF then also CDDETD is raised.
		// If so, we switch to CAD state where we will wait for CDDETD event.
		//
		else if (intr & IRQ_LORA_CDDONE_MASK) {

			opmode(OPMODE_CAD);
			rssi = readRegister(REG_RSSI);							// Read the RSSI

#			if _MONITOR>=1
			if ((debug>=2) && (pdebug & P_SCAN)) {
				String response = "SCAN:: CDDONE: ";
				mStat(intr, response);
				mPrint(response);
			}
#			endif //_MONITOR
			// We choose the generic RSSI as a sorting mechanism for packages/messages
			// The pRSSI (package RSSI) is calculated upon successful reception of message
			// So we expect that this value makes little sense for the moment with CDDONE.
			// Set the rssi as low as the noise floor. Lower values are not recognized then.
			// Every cycle starts with gwayConfig.ch==0 and sf=SF7 (or the set init SF)
			//
			if (rssi > (RSSI_LIMIT - (gwayConfig.hop * 7)))		// Is set to 35, or 29 for HOP
			{
#				if _MONITOR>=1
				if ((debug>=2) && (pdebug & P_SCAN)) {
					String response = "SCAN:: -> CAD: ";
					mStat(intr, response);
					mPrint(response);
				}
#				endif //_MONITOR
				_state = S_CAD;										// promote to next level
				_event=0;
			}

			// If the RSSI is not big enough we skip the CDDONE
			// and go back to scanning
			else {
#				if _MONITOR>=1
				if ((debug>=2) && (pdebug & P_SCAN)) {
					String response = "SCAN:: rssi=";
					response += String(rssi);
					response += ": ";
					mStat(intr, response);
					mPrint(response);
				}
#				endif //_MONITOR
				_state = S_SCAN;
			}

			// Clear the CADDONE flag
			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);
			writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);
			doneTime = micros();									// Need CDDONE or other intr to reset timeout			

		}//SCAN CDDONE 

		// So if we are here then we are in S_SCAN and the interrupt is not
		// CDDECT or CDDONE. it is probably soft interrupt _event==1
		// So if gwayConfig.hop we change the frequency and restart the
		// interrupt in order to check for CDONE on other frequencies
		// if gwayConfig.hop we start at the next frequency, hop () sets the sf to SF7.
		// If we are at the end of all frequencies, reset frequencies and sf
		// and go to S_SCAN state.
		//
		// Note:: We should make sure that all frequencies are scanned in a row
		// and when we switch to gwayConfig.ch==0 we should stop for a while
		// to allow system processing.
		// We should make sure that we enable webserver etc every once in a while.
		// We do this by changing _event to 1 in loop() only for gwayConfig.hop and
		// use _event=0 for non hop.
		//
		else if (intr == 0x00) 
		{
			_event=0;												// XXX 26/12/2017 !!! NEED
		}

		// Unkown Interrupt, so we have an error
		//
		else {
#			if _MONITOR>=1
			if ((debug>=0) && (pdebug & P_SCAN)) {
				String response = "SCAN unknown:: ";
				mStat(intr, response);
				mPrint(response);
			}
#			endif //_MONITOR
			_state=S_SCAN;
			//_event=1;												// XXX 19/06/03 loop until interrupt
			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);
			writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);
		}

	  break; // S_SCAN


	  // ----------------------------------------------------------------------------------------
	  // S_CAD: In CAD mode we scan every SF for high RSSI until we have a DETECT.
	  // Reason is the we received a CADDONE interrupt so we know a message is received
	  // on the frequency but may be on another SF.
	  //
	  // If message is of the right frequency and SF, IRQ_LORA_CDDETD_MSAK interrupt
	  // is raised, indicating that we can start beging reading the message from SPI.
	  //
	  // DIO0 interrupt IRQ_LORA_CDDONE_MASK in state S_CAD==2 means that we might have
	  // a lock on the Freq but not the right SF. So we increase the SF
	  //
	  case S_CAD:

		// Intr=IRQ_LORA_CDDETD_MASK
		// We have to set the sf based on a strong RSSI for this channel
		// Also we set the state to S_RX and start receiving the message
		//
		if (intr & IRQ_LORA_CDDETD_MASK) {

			// Set RXDONE interrupt to dio0, RXTOUT to dio1
			writeRegister(REG_DIO_MAPPING_1, (
				MAP_DIO0_LORA_RXDONE | 
				MAP_DIO1_LORA_RXTOUT | 
				MAP_DIO2_LORA_NOP |
				MAP_DIO3_LORA_CRC ));

			// Accept no interrupts except RXDONE or RXTOUT
			_event=0;								

			// if CDECT, make state S_RX so we wait for RXDONE intr

			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) ~(
				IRQ_LORA_RXDONE_MASK | 
				IRQ_LORA_RXTOUT_MASK |
				IRQ_LORA_HEADER_MASK |
				IRQ_LORA_CRCERR_MASK ));

			// Reset all interrupts as soon as possible
			// But listen ONLY to RXDONE and RXTOUT interrupts 
			//writeRegister(REG_IRQ_FLAGS, IRQ_LORA_CDDETD_MASK | IRQ_LORA_RXDONE_MASK);

			// If we want to reset CRC, HEADER and RXTOUT flags as well
			writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF );			// XXX 180326, reset all CAD Detect interrupt flags

			//_state = S_RX;										// XXX 180521 Set state to start receiving
			opmode(OPMODE_RX_SINGLE);								// set reg 0x01 to 0x06, initiate READ

			delayMicroseconds( RSSI_WAIT );							// Wait some microseconds less
			//delayMicroseconds( (0x01 << ((uint8_t)sf - 5 )) );
			rssi = readRegister(REG_RSSI);							// Read the RSSI
			_rssi = rssi;											// Read the RSSI in the state variable

			detTime = micros();
#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_CAD)) {
				String response = "CAD:: ";
				mStat(intr, response);
				mPrint(response);
			}
#			endif //_MONITOR
			_state = S_RX;											// Set state to start receiving

		}// CDDETD

		// Intr == CADDONE
		// So we scan this SF and if not high enough ... next
		//
		else if (intr & IRQ_LORA_CDDONE_MASK) {
			// If this is not the max SF, increment the SF and try again
			// Depending on the frequency scheme this is for example SF8, SF10 or SF12
			// We expect on other SF get CDDETD
			//
			if (((uint8_t)sf) < freqs[gwayConfig.ch].upHi) {

				sf = (sf_t)((uint8_t)sf+1);							// Increment sf
				setRate(sf, 0x04);									// Set SF with CRC==on

				opmode(OPMODE_CAD);									// Scanning mode

				delayMicroseconds(RSSI_WAIT);
				rssi = readRegister(REG_RSSI);						// Read the RSSI

#				if _MONITOR>=1
				if ((debug>=3) && (pdebug & P_CAD)) {
					mPrint("S_CAD:: CDONE, SF=" + String(sf) );
				}

				// reset interrupt flags for CAD Done
				_event=0;											// XXX 180324, when increasing SF loop, ws 0x00
				writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);	// Accept all
				//writeRegister(REG_IRQ_FLAGS, IRQ_LORA_CDDONE_MASK | IRQ_LORA_CDDETD_MASK);
				writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF );		// Clear all

#				endif //_MONITOR
			}

			// If we reach the highest SF for the frequency plan,
			// we should go back to SCAN state
			//
			else {

				// Reset Interrupts
				_event=1;											// reset soft intr, to state machine again
				writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);	// Reset the interrupt mask
				writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF );		// or IRQ_LORA_CDDONE_MASK

				_state = S_SCAN;									// As soon as we reach SF12 do something
				sf = (sf_t) freqs[gwayConfig.ch].upLo;
				cadScanner();										// Which will reset SF to lowest SF

#				if _MONITOR>=1		
				if ((debug>=3) && (pdebug & P_CAD)) {
					mPrint("CAD->SCAN:: " + String(intr) );
				}
#				endif //_MONITOR
			}
			doneTime = micros();									// We need CDDONE or other intr to reset timeout

		} //CAD CDDONE

		// if this interrupt is not CDECT or CDDONE then probably is 0x00
		// This means _event was set but there was no real interrupt (yet).
		// So we clear _event and wait for next (soft) interrupt.
		// We stay in the CAD state because CDDONE means something is 
		// coming on this frequency so we wait on CDECT.
		//
		else if (intr == 0x00) {
#			if _MONITOR>=1
			if ((debug>=3) && (pdebug & P_CAD)) {
				mPrint ("CAD:: intr is 0x00");
			}
#			endif //_MONITOR
			//_event=1;												// Stay in CAD _state until real interrupt
		}

		// else we do not recognize the interrupt. We print an error
		// and restart scanning. If hop we start at gwayConfig.ch==0
		//
		else {
#			if _MONITOR>=1
			if ( debug>=0) { 
				mPrint("ERROR CAD: Unknown::" + String(intr) ); 
			}
#			endif //_MONITOR
			_state = S_SCAN;
			sf = SF7;
			cadScanner();											// Scan and set SF7
			
			// Reset Interrupts
			_event=1;												// If unknown interrupt, restarts
			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);		// Reset the interrupt mask
			writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);			// Reset all interrupts

		}
	  break; //S_CAD
	  
	  // ----------------------------------------------------------------------------------------
	  // If we receive an RXDONE interrupt on dio0 with state==S_RX
	  // 	So we should handle the received message
	  // Else if it is RXTOUT interrupt
	  //	Timeout, so we handle this interrupt, and get modem out of standby
	  // Else
	  //	Go back to SCAN
	  //
	  case S_RX:

		if (intr & IRQ_LORA_RXDONE_MASK) {

#			if _CRCCHECK>=1
			// We have to check for CRC error which will be visible AFTER RXDONE is set.
			// CRC errors might indicate that the reception is not OK.
			// Could be CRC error or message too large.
			// CRC error checking requires DIO3
			//
			if (intr & IRQ_LORA_CRCERR_MASK) {
#				if _MONITOR>=1
				if ((debug>=0) && (pdebug & P_RX)) {
					String response = "^ CRC ERROR:: ";
					mStat(intr, response);
				}
#				endif //_MONITOR

				if ((gwayConfig.cad) || (gwayConfig.hop)) {
					sf = SF7;
					_state = S_SCAN;
					cadScanner();
				}
				else {
					_state = S_RX;
					rxLoraModem();
				}

				// Reset interrupts
				_event=0;												// CRC error
				writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);		// Reset the interrupt mask
				writeRegister(REG_IRQ_FLAGS, (uint8_t) (
					IRQ_LORA_RXDONE_MASK | 
					IRQ_LORA_RXTOUT_MASK | 
					IRQ_LORA_HEADER_MASK | 
					IRQ_LORA_CRCERR_MASK ));

				break;													// Get out of loop
			}// RX-CRC mask
#			endif //_CRCCHECK

			// If we are here, no CRC error occurred, start timer
#			if _DUSB>=1 || _MONITOR>=1
				uint32_t rxDoneTime = micros();	
#			endif	

			// There should not be an error in the message
			LoraUp.payLoad[0]= 0x00;								// Empty the message

			// If receive S_RX error, 
			// - print Error message
			// - Set _state to SCAN
			// - Set _event=1 so that we loop until we have an interrupt
			// - Reset the interrupts
			// - break
			// NOTE: receivePacket also increases .ok0 - .ok2 counter

			if((LoraUp.size = receivePkt(LoraUp.payLoad)) <= 0) {
#				if _MONITOR>=1
				if ((debug>=0) && (pdebug & P_RX)) {
					String response = "sMachine:: ERROR S-RX: size=" + String(LoraUp.size);
					mPrint(response);
				}
#				endif //_MONITOR

				_event=1;
				writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);	// Reset the interrupt mask
				writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);

				_state = S_SCAN;
				break;
			}

#			if _MONITOR>=1
			if ((debug >=2) && (pdebug & P_RX)) {
				String response  = "RXDONE:: dT=";
				response += String(rxDoneTime - detTime);
				mStat(intr, response);
				mPrint(response);
			}
#			endif //_MONITOR

			// Do all register processing in this section
			uint8_t value = readRegister(REG_PKT_SNR_VALUE);		// 0x19; 
			if ( value & 0x80 ) {									// The SNR sign bit is 1
				
				value = ( ( ~value + 1 ) & 0xFF ) >> 2;				// Invert and divide by 4
				LoraUp.snr = -value;
			}
			else {
				// Divide by 4
				LoraUp.snr = ( value & 0xFF ) >> 2;
			}

			// Packet RSSI
			LoraUp.prssi = readRegister(REG_PKT_RSSI);				// read register 0x1A, packet rssi

			// Correction of RSSI value based on chip used.	
			if (sx1272) {											// Is it a sx1272 radio?
				LoraUp.rssicorr = 139;
			} else {												// Probably SX1276 or RFM95
				LoraUp.rssicorr = 157;
			}

			LoraUp.sf = readRegister(REG_MODEM_CONFIG2) >> 4;

			// If read was successful, read the package from the LoRa bus
			//
			if (receivePacket() <= 0) {								// read is not successful
#				if _MONITOR>=1
				if ((debug>=0) && (pdebug & P_RX)) {
					mPrint("sMach:: ERROR receivePacket");
				}
#				endif //_MONITOR
			}
			
			// Set the modem to receiving BEFORE going back to user space.
			// 
			if ((gwayConfig.cad) || (gwayConfig.hop)) {
				_state = S_SCAN;
				sf = SF7;
				cadScanner();
			}
			else {
				_state = S_RX;
				rxLoraModem();
			}

			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);
			writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);			// Reset the interrupt mask
			eventTime=micros();										// There was an event for receive
			_event=0;
		}// RXDONE

		// RXOUT: 
		// We did receive message receive timeout
		// This is the most common event in hop mode, possibly due to the fact
		// that receiving has started too late in the middle of a message
		// (according to the documentation). So is there a way to start receiving
		// immediately without delay.
		//
		else if (intr & IRQ_LORA_RXTOUT_MASK) {

			// Make sure we reset all interrupts
			// and get back to scanning
			_event=0;												// Is set by interrupt handlers
			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00 );
			writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);			// reset all interrupts

			// If RXTOUT we put the modem in cad state and reset to SF7
			// If a timeout occurs here we reset the cadscanner
			//
			if ((gwayConfig.cad) || (gwayConfig.hop)) {
				// Set the state to CAD scanning
#				if _MONITOR>=1
				if ((debug>=2) && (pdebug & P_RX)) {
					String response = "RXTOUT:: ";
					mStat(intr, response);
					mPrint(response);
				}
#				endif //_MONITOR
				sf = SF7;
				cadScanner();										// Start the scanner after RXTOUT
				_state = S_SCAN;									// New state is scan
			}

			// If not in cad mode we are in single channel single sf mode.
			//
			else {
				_state = S_RX;										// Receive when interrupted
				rxLoraModem();
			}

			eventTime=micros();										//There was an event for receive
			doneTime = micros();									// We need CDDONE or other intr to reset timeout

		}// RXTOUT

		else if (intr & IRQ_LORA_HEADER_MASK) {
			// This interrupt means we received an header successfully
			// which is normall an indication of RXDONE
			//writeRegister(REG_IRQ_FLAGS, IRQ_LORA_HEADER_MASK);
#			if _MONITOR>=1
			if ((debug>=3 ) && (pdebug & P_RX)) {
				mPrint("RX HEADER:: " + String(intr));
			}
#			endif //_MONITOR
			//_event=1;
		}

		// If we did not receive a hard interrupt
		// Then probably do not do anything, because in the S_RX
		// state there always comes a RXTOUT or RXDONE interrupt
		//
		else if (intr == 0x00) {
#			if _MONITOR>=1
			if ((debug>=3) && (pdebug & P_RX)) {
				mPrint("S_RX no INTR:: " + String(intr));
			}
#			endif //_MONITOR
		}

		// The interrupt received is not RXDONE, RXTOUT or HEADER
		// therefore we wait. Make sure to clear the interrupt
		// as HEADER interrupt comes just before RXDONE
		else {							
#			if _MONITOR>=1
			if ((debug>=0) && (pdebug & P_RX)) {
				mPrint("R S_RX:: no RXDONE, RXTOUT, HEADER:: " + String(intr));
			}
#			endif //_MONITOR
			//writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00 );
			//writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);
		}// int not RXDONE or RXTOUT

	  break; // S_RX


	  // ----------------------------------------------------------------------------------------
	  // Start the transmission of a message in state S-TX (DOWN)
	  // This is not an interrupt state, we use this state to start transmission
	  // the interrupt TX-DONE tells us that the transmission was successful.
	  // It therefore is no use to set _event==1 as transmission might
	  // not be finished in the next loop iteration
	  //
	  case S_TX:

		// We need a timeout for this case. In case there does not come an interrupt,
		// then there will not be a TXDONE but probably another CDDONE/CDDETD before
		// we have a timeout in the main program (Keep Alive)
		if (intr == 0x00) {
#			if _MONITOR>=1
			if ((debug>=2) && (pdebug & P_TX)) {
				mPrint("TX:: 0x00");
			}
#			endif //_MONITOR
			_event= 1;
		}

		_state=S_TXDONE;
		_event=1;													// Or remove the break below

#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_TX)) {
			String response="TX fini:: ";
			mStat(intr, response);
			mPrint(response); 
		}
#		endif //_MONITOR

	  break; // S_TX


	  // ----------------------------------------------------------------------------------------
	  // After the transmission is completed by the hardware, 
	  // the interrupt TXDONE is raised telling us that the tranmission
	  // was successful.
	  // If we receive an interrupt on dio0 _state==S_TX it is a TxDone interrupt
	  // Do nothing with the interrupt, it is just an indication.
	  // send Packet switch back to scanner mode after transmission finished OK
	  //
	  case S_TXDONE:
		if (intr & IRQ_LORA_TXDONE_MASK) {

#			if _MONITOR>=1
			if ((debug>=1) && (pdebug & P_TX)) {
				String response =  "v OK, stateMachine TXDONE: rcvd=";
				printInt(micros(),response);
				if (micros() < LoraDown.tmst) {
					response += ", diff=- " ;
					printInt(LoraDown.tmst - micros(), response );
				}
				else {
					response += ", diff=";
					printInt(micros()-LoraDown.tmst, response);
				}
				mPrint(response);
			}
#			endif //_MONITOR

			// After transmission reset to receiver
			if ((gwayConfig.cad) || (gwayConfig.hop)) {				// XXX 26/02
				// Set the state to CAD scanning
				_state = S_SCAN;
				sf = SF7;
				cadScanner();										// Start the scanner after TX cycle
			}
			else {
				_state = S_RX;
				rxLoraModem();		
			}

			_event=0;

			if (protocol == 1) {							// Got from the downstream message
#				if _MONITOR>=1
				if ((debug>=2) && (pdebug & P_TX)) {
					mPrint("^ TX_ACK:: readUdp: protocol version 1");
				}
#				endif
				break;										// return
			}

#			if _MONITOR>=1
			if ((debug>=2) && (pdebug & P_TX)) {
				mPrint("^ TX_ACK:: readUDP: protocol version 2+");
			}
#			endif //_MONITOR

			// UP: Now respond with an TX_ACK
			// Byte 3 is 0x05; see para 5.2.6 of spec
			buff[0]= buff_down[0];							// As read from the Network Server
			buff[1]= buff_down[1];							// Token 1, copied from downstream
			buff[2]= buff_down[2];							// Token 2
			buff[3]= TX_ACK;								// ident == 0x05;
			buff[4]= 0;										// Not error means "\0"
			// If it is an error, please look at para 6.1.2

			yield();

			// Only send the PULL_ACK to the UDP socket that just sent the data!!!
			Udp.beginPacket(remoteIpNo, remotePortNo);

			// XXX We should format the message before sending up with UDP

			if (Udp.write((unsigned char *)buff, 12) != 12) {
#				if _MONITOR>=1
				if (debug>=0) {
					mPrint("^ readUdp:: ERROR: PULL_ACK write");
				}
#				endif //_MONITOR
			}
			else {
#				if _MONITOR>=1
				if ((debug>=2) && (pdebug & P_TX)) {
					mPrint("^ readUdp:: TX_ACK: micros="+String(micros()));
				}
#				endif //_MONITOR
			}

			if (!Udp.endPacket()) {
#				if _MONITOR>=1
				if ((debug>=0) && (pdebug & P_TX)) {
					mPrint("^ readUdp:: PULL_ACK: ERROR Udp.endPacket");
				}
#				endif //_MONITOR
			}
			yield();
		}

		// If a soft _event==0 interrupt and no transmission finished:
		else if ( intr != 0 ) {
#			if _MONITOR>=1
			if ((debug>=0) && (pdebug & P_TX)) {
				String response =  "TXDONE:: ERROR unknown intr=";
				mStat(intr, response);
				mPrint(response);
			} //_MONITOR
#			endif
			writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);
			writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);			// reset interrupt flags
			_event=0;
			_state=S_SCAN;
		}

		// intr == 0
		// TX_DONE interrupt is not received in time
		else {

			// Increase timer. If timer exceeds certain value (7 seconds!), reset
			// After sending a message with S_TX, we have to receive a TXDONE interrupt
			// within 7 seconds according to spec, or there is a problem.
			if ( sendTime > micros() ) {
				sendTime = 0;				// This could be omitted for usigned ints
#				if _MONITOR>=1
				if ((debug>=1) && (pdebug & P_TX)) {
					mPrint("v Warning:: intr=0 received, sendTime=0");
				}
#				endif //_MONITOR
			}
			if (( _state == S_TXDONE ) && (( micros() - sendTime) > 8000000 )) {
#				if _MONITOR>=1
				if ((debug>=1) && (pdebug & P_TX)) {
					mPrint("v Warning:: TXDONE not received, reset receiver");
				}
#				endif //_MONITOR
				startReceiver();
			}
		}

		// ONLY NOW WE START TO MONITOR THE PULL_RESP MESSAGE.
#		if _MONITOR>=1
		if ((debug>=1) && (pdebug & P_TX)) {
			//uint8_t * data = buff_down + 4;
			//data[packetSize-4] = 0;
			//mPrint("v TXDONE:: PULL_RESP: size="+String(packetSize)+", data="+String((char *)data)); 
		}
#		endif //_MONITOR

		// The interrupts should be set by the _loramodem.ino functions
		//writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);		// receive all
		//writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);				// reset interrupt flags

	  break; // S_TXDONE	  

	  
	  // ----------------------------------------------------------------------------------------
	  // If _STATE is in an undefined state
	  // If such a thing happens, we should re-init the interface and 
	  // make sure that we pick up next interrupt
	  default:
#		if _MONITOR>=1
		if ((debug>=0) && (pdebug & P_PRE)) {
			mPrint("ERR state=" + String(_state));	
		}
#		endif //_MONITOR

		if ((gwayConfig.cad) || (gwayConfig.hop)) {
#			if _MONITOR>=1
			if (debug>=0) {
				String response = "default:: Unknown _state ";
				mStat(intr, response);
				mPrint(response);
			}
#			endif //_MONITOR
			_state = S_SCAN;
			sf = SF7;
			cadScanner();											// Restart the state machine
			_event=0;									
		}
		else														// Single channel AND single SF
		{
			_state = S_RX;
			rxLoraModem();
			_event=0;
		}
		writeRegister(REG_IRQ_FLAGS_MASK, (uint8_t) 0x00);
		writeRegister(REG_IRQ_FLAGS, (uint8_t) 0xFF);				// Reset all interrupts
		eventTime=micros();											// Reset event for unkonwn state
		
	  break;// default
	}// switch(_state)
	
	return;
}
