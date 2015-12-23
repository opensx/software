/*
** sx-wdec84-tiny.h
*
*  Created on: 07 Nov 2015
*  changed  15 Nov 2015
*
*  Author: Michael Blank
*/


#ifndef SX_WDEC84_TINY_H_
#define SX_WDEC84_TINY_H_

#define F_CPU 8000000UL  // 8 MHz

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
// define pins for sx-wdec84 board
#define SX1    PB1     // pin of track1 input =PCINT9
#define SX2    PB2     // pin of track2 input =PCINT10
#define SX1INT PCINT9
#define SX2INT PCINT10
#define SXPCIE PCIE1    // 1 for INTs >=8

//  #define OUT0   PB0     // F0
//  #define OUT1   PA1     // F1

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

#define LOW   0
#define HIGH  1



#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? (bitSet(value, bit)) : (bitClear(value, bit)))
#define invert(bitvalue) (bitvalue ? (0x00) : (0x01) )

void switchAdr(void);
void switchData(void);

volatile uint8_t sxData;   // to store the SX data byte of selected channel

volatile uint8_t progAdr;   // to store the SX data for programming
volatile uint8_t progF1bit; // to store the SX data for programming
volatile uint8_t progF2bit; // to store the SX data for programming

// address and bits for functions used for decoder
uint8_t decSXAdr;
uint8_t f1bit;      // sxData bit which defined state of F1
uint8_t f2bit;      // sxData bit which defined state of F1

#endif /* SXFUNTINY_H_ */