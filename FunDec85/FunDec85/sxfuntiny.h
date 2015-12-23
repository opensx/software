/*
* sxfuntiny.h
*
*
=========================================
FUSES beim ATtiny85:
=========================================
SELFPRGEN = [ ]
RSTDISBL  = [ ]
DWEN      = [ ]
SPIEN     = [X]
WDTON     = [ ]
EESAVE    = [ ]
BODLEVEL  = 2V7
CKDIV8    = [ ]
CKOUT     = [ ]
SUT_CKSEL = INTRCOSC_8MHZ_6CK_14CK_64MS

EXTENDED  = 0xFF (valid)
HIGH      = 0xDD (valid)
LOW       = 0xE2 (valid)
==========================================

* Created: 11/26/2013 7:57:51 AM
*  Author: mblank
*/


#ifndef SXFUNTINY_H_
#define SXFUNTINY_H_

#define F_CPU 8000000UL  // 8 MHz
#define F_PWM 100                       // PWM-Frequenz in Hz
#define PWM_STEPS 255                   // PWM-Schritte pro Zyklus(1..255)

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <avr/wdt.h>

#if defined (__AVR_ATtiny9__)

// define pins for attiny9 fun-dec1 board
#define SX1    PB0     // pin of track1 input =PCINT0
#define PCINT_PIN SX1
#define SX2    PB2     // pin of track2 input =INT0
#define OUT0   PB1     // F0
#define OUT1   PB3     // F1

#elif defined (__AVR_ATtiny84A__)
// define pins for loco-decoder board
#define SX1    PB1     // pin of track1 input =PCINT9
#define SX2    PB2     // pin of track2 input =PCINT10
#define SX1INT PCINT9
#define SX2INT PCINT10
#define SXPCIE PCIE1    // 1 for INTs >=8
#define INTERUPT_VECTOR  PCINT1_vect

//  #define OUT0   PB0     // F0
//  #define OUT1   PA1     // F1

#elif defined (__AVR_ATtiny85__)
// define pins for loco-decoder board
#define SX1    PB3     // pin of track1 input
#define SX2    PB2     // pin of track2 input 
#define SX1INT PCINT2
#define SX2INT PCINT3
#define SXPCIE PCIE    // for ATtiny85 

#define OUT0   PB0     // F0
#define OUT1   PB1     // F1

#define INTERUPT_VECTOR  PCINT0_vect

#else
// this is not supported, must have info about hardware
#error "CPU NOT Supported"
#endif

// defines for state machine
#define SYNC  0
#define ADDR  1
#define DATA  2

#define MAX_DATACOUNT      7    // 7 data frames in 1 SYNC Channel
#define MAX_DATABITCOUNT   12   // 12 bits in 1 frame

#define MAX_CHANNEL_NUMBER 112   // SX channels

#define TYPE_SIGNAL_DECODER 4    // decodes all 8 bits of SX signal
#define TYPE_MOTOR_DECODER  5     // decodes higher or lower 4 bits of SX signal
#define TYPE_LED_DECODER    6       // decodes higher or lower 2 bits of SX signal

#define DECODER_ADDRESS    83     // will be address of decoder
#define SUB_ADDRESS        4      // 
#define DECODER_TYPE       TYPE_LED_DECODER

#define LOW   0
#define HIGH  1

#define TRUE  1
#define FALSE 0


#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? (bitSet(value, bit)) : (bitClear(value, bit)))
#define invert(bitvalue) (bitvalue ? (0x00) : (0x01) )

void switchAdr(void);
void switchData(void);
void resetSync(void);


volatile uint8_t sx; // contains data for one (the selected) sx channel, unfiltered
volatile uint8_t data_received; //flag indicating that new data has been detected.

uint8_t myBaseAdr;
uint8_t myFrameCount;

#define FIMAX 15
volatile uint8_t pwm[2];   // pwm setting for channels
volatile uint8_t fadeindex[2];  


#endif /* SXFUNTINY_H_ */