/*
 * sx-wdec84-read.h
 *
 *  Created on: 07 Nov 2015
 *  changed  15 Nov 2015
 
 *  Copyright:  Michael Blank
 */

#ifndef SX_WDEC84_READ_H_
#define SX_WDEC84_READ_H_

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

// timing definitions
#define KEY_DEBOUNCE_MILLIS  100
#define LOCK_KEY_MILLIS      250
#define WAIT_MILLIS          50
#define BLINK_MILLIS  		 80
#define TURNOUT_ON_MILLIS    250      // turnout switched on for xx millis

#define INITIAL_ADDRESS    83     // default address of decoder
#define F1_BIT             0      // default bit for F1
#define F2_BIT             1      // default bit for F2

#define LED   PA1    // (LED output, for adr prog)
#define F1A   PA7    // 1. Funktion (sxData bit cleared)
#define PORT_F1A    PORTA
#define F1B   PB0    // 1. Funktion (sxData bit set)
#define PORT_F1B    PORTB
#define F2A   PA2    // 1. Funktion (sxData bit cleared)
#define PORT_F2A    PORTA
#define F2B   PA3    // 1. Funktion (sxData bit set) 
#define PORT_F2B    PORTA
#define BTN   PA0    //(input, for adr prog button)

#define NO 0
#define YES 1

#define A_STATE     0
#define B_STATE     1
#define UNKNOWN_STATE  2


uint8_t program;

void setup(void);
void toggle_program_mode(void);
void finishProgramming(void);
void startProgramming(void);
void init_from_eeprom(void);
void blink(void);
void keyPressed(void);
uint8_t value2bit(uint8_t, uint8_t);

//void setMotor(uint8_t);
//void setSignal(uint8_t);

#endif /* SXREAD_H_ */