/*
* sx-wdec84-read.c
*
* Created: 06 Nov 2015
* changed  15 Nov 2015

* Gleis Weichendekoder "sx-wdec84" (besser px-wdec84) für ATtiny84

*  Fuses beim ATtiny84 : 
SELFPRGEN = [ ]
RSTDISBL = [ ]
DWEN = [ ]
SPIEN = [X]
WDTON = [ ]
EESAVE = [ ]
BODLEVEL = DISABLED
CKDIV8 = [ ]
CKOUT = [ ]
SUT_CKSEL = INTRCOSC_8MHZ_6CK_14CK_64MS_DEFAULT

EXTENDED = 0xFF (valid)
HIGH = 0xDF (valid)
LOW = 0xE2 (valid)

*  pins for sx-wdec84 board (defined .h files)
*  SX1 = PB1     // pin of track1 input =PCINT9
*  SX2 = PB2     // pin of track2 input =PCINT10
*  F1A = PA7     // 1. Funktion (sxData bit cleared)
*  F1B = PB0     // 1. Funktion (sxData bit set)
*  F2A = PA2     // 1. Funktion (sxData bit cleared)
*  F2B = PA3     // 1. Funktion (sxData bit set)

*  LED = PA1    (LED output, for adr prog)
*  BTN = PA0    (input, for adr prog button)

*  Author: mblank
*/
#define F_CPU 8000000UL


#include "sx-wdec84-tiny.h"
#include "sx-wdec84-read.h"
#include "stdlib.h"

extern volatile uint32_t count;
int startupCount;

uint8_t EEMEM adrEE = 83;     //  => SX Ch0 for programming, dec address
uint8_t EEMEM f1EE = 1;       //  => SX Ch1 for programming, f1 bit
uint8_t EEMEM f2EE = 2;       //  => SX Ch2 for programming, f2 bit

uint8_t f1State = UNKNOWN_STATE;
uint8_t f2State = UNKNOWN_STATE;

void setup() {

    wdt_enable(WDTO_2S);    // enable watchdog
    sxData = 0;

#if defined (__AVR_ATtiny9__)
    // DDRB = 0b00001010;  // output PB1 and PB3
    // SX1 and SX2 are inputs
    //  PCICR |= (1 << PCIE0);     // set PCIE0 to enable PCINT
#elif defined (__AVR_ATtiny84A__)
// ********************* TODO ****************************
    DDRB = (1 << F1B);  // output PB0 = F0
    DDRA = (1 << F1A) | (1 << F2A) | (1 << F2B) | (1 << LED);
    PORTA |= (1 << BTN);    // pullup for button
	PORTA &= ~( (1 << LED) | (1 << F1A) | (1 << F2A) | (1 << F2B) ) ;  // LED and functions off
	PORTB &= ~(1 << F1B);
    GIMSK =  (1 << SXPCIE);     // set PCIE1 to enable PCINT
    //PCMSK1 Register controls which pins contribute to the pin change interrupts
    PCMSK1 = (1 << SX1INT) | (1 << SX2INT) ;   // set PCINT to trigger an interrupt on state change
#endif
    program = NO;
    init_from_eeprom();
    startupCount = rand() / 40 + 100;   // range 500ms to ~5s
    sei();
}

void init_from_eeprom()  {

	adrEE = eeprom_read_byte(&adrEE);

	// in case there is nonsense here, program the inital address in EE
	if ((adrEE == 0) || (adrEE > 103)) {
		adrEE = INITIAL_ADDRESS;
		eeprom_write_byte(&adrEE, INITIAL_ADDRESS);
		eeprom_write_byte(&f1EE,F1_BIT);
		eeprom_write_byte(&f2EE,F2_BIT);
	}

	// now we can work with this address
	decSXAdr = adrEE;

	f1bit = eeprom_read_byte(&f1EE);
	f2bit = eeprom_read_byte(&f2EE);

}

int key_pressed()
{  
	/* the button is pressed when BUTTON_BIT is clear */
	if (bitRead(PINA, BTN) == LOW)
	{
		_delay_ms(KEY_DEBOUNCE_MILLIS);
		if (bitRead(PINA, BTN) == LOW) return YES;
	}

	return NO;
}

uint8_t value2bit(uint8_t val, uint8_t def) {
	uint8_t ret = def;
	switch(val) {
		case 1:
		ret = 0;
		break;
		case 2:
		ret = 1;
		break;
		case 4:
		ret = 2;
		break;
		case 8:
		ret = 3;
		break;
		case 16:
		ret = 4;
		break;
		case 32:
		ret = 5;
		break;
		case 64:
		ret = 6;
		break;
		case 128:
		ret = 7;
		break;
	}
	return ret;
}

void startProgramming()  {
	PORTA |= (1 << LED);
	program = YES;
}

void finishProgramming()  {
	program = NO;
	PORTA &= ~(1 << LED) ;

	// get address from SX channel 0
	uint8_t adr = progAdr;
	if ((adr > 0) && (adr <= 103)) {
		// some sanity check
		eeprom_write_byte(&adrEE, adr);
		decSXAdr = adr;
		
		// get f1bit SX channel 1
		f1bit =  value2bit(progF1bit,F1_BIT);
		// only a single bit should have been set in progF1bit
		eeprom_write_byte(&f1EE,f1bit);


		// get f2bit SX channel 2
		f2bit = value2bit(progF2bit,F2_BIT);
		eeprom_write_byte(&f2EE,f2bit);
		
	}
}

void blink() {
	static uint8_t blinkCount = 0;
	blinkCount ++;
	if (blinkCount > 7) {
		blinkCount = 0;
		// toggle
		if (bitRead(PINA,LED)) {
			PORTA &= ~(1 << LED) ;  // off
			} else {
			PORTA |= (1 << LED);  // on
		}
	}
}


void toggle_program_mode() {
	if (program == YES) {
		finishProgramming();
	}
	else {
		startProgramming();
	}
}


int main() {

    setup();
	
	// uint8_t f1state = UNKNOWN_STATE;
	// TODO uint8_t f2state = UNKNOWN_STATE;

    while(1)
    {
        wdt_reset();
		if (count < startupCount) {
			PORTA |= (1 << LED);  // LED on / debug
			; // do nothing
		} else {
			if (key_pressed() == YES) 
			{
				toggle_program_mode();
				_delay_ms(LOCK_KEY_MILLIS);  // totzeit nach key druck
			}

			_delay_ms(WAIT_MILLIS);
			if (program) {
				blink();
			} else { // normal operation
				PORTA &= ~(1 << LED) ; // switch off LED	
				uint8_t f1 = bitRead(sxData,f1bit);   
				if (f1 != f1State) {
					f1State = f1;
					if (f1) {
						// set F1B
						PORT_F1A &= ~(1 << F1A);
						PORT_F1B |= (1 << F1B);
						_delay_ms(TURNOUT_ON_MILLIS);
						PORT_F1B &= ~(1 << F1B);
						_delay_ms(WAIT_MILLIS);
					} else {
						PORT_F1A |= (1 << F1A);
						PORT_F1B &= ~(1 << F1B);
						_delay_ms(TURNOUT_ON_MILLIS);
						PORT_F1A &= ~(1 << F1A);
						_delay_ms(WAIT_MILLIS);
					}
				}
        
		        uint8_t f2 = bitRead(sxData,f2bit);
				if (f2 != f2State) {
					f2State = f2;
					if (f2) {
						// set F2B
						PORT_F2A &= ~(1 << F2A);
						PORT_F2B |= (1 << F2B);
						_delay_ms(TURNOUT_ON_MILLIS);
						PORT_F2B &= ~(1 << F2B);
						_delay_ms(WAIT_MILLIS);
					} else {
						PORT_F2A |= (1 << F2A);
						PORT_F2B &= ~(1 << F2B);
						_delay_ms(TURNOUT_ON_MILLIS);
						PORT_F2A &= ~(1 << F2A);
						_delay_ms(WAIT_MILLIS);
					}
				}
			}   // (program)
		} // (count - startupCount)
	} // while(1)
}
