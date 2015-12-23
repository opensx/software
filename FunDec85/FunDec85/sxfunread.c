/*
* FunDec1.c
*
* Created: 11/26/2013 7:52:08 AM
*  Author: mblank
*/

/*
;*************** Adressliste *******************************************************
;   _channel = (15-_baseAdr) + ((6-_dataFrameCount)<<4);
;   83 & 0x0F = 3 => baseAdr = 15-3 =12
; formel:   Gru (baseAdr) = 15 - (channel & 0x0f)
;   83 & 0xf0 = 80 (dec) => DataframeCount = 1
; formel:   dataFrameCount = 6 - (channel >> 4)
*/


#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "sxfuntiny.h"

#define LIMIT  5
#define MAXCOUNTER (LIMIT + LIMIT)
#define PWM 3


volatile uint8_t counter[2];
uint8_t pwmCount;
uint8_t sxCopy;

void setup() {

    wdt_enable(WDTO_1S);    // enable watchdog

    counter[0] = counter[1] = LIMIT;
    data_received = FALSE;
    pwmCount=0;
    
    myBaseAdr = 15 - (((uint8_t)DECODER_ADDRESS) & 0x0f);
    myFrameCount = 6 - (((uint8_t)DECODER_ADDRESS) >> 4);

    #if defined (__AVR_ATtiny9__)
    // DDRB = 0b00001010;  // output PB1 and PB3
    // SX1 and SX2 are inputs
    //  PCICR |= (1 << PCIE0);     // set PCIE0 to enable PCINT
    #elif defined (__AVR_ATtiny84A__)
    DDRB = 0x01;  // output PB0 = F0
    DDRA = 0x02;  // output PA1 = F1

    GIMSK =  (1 << SXPCIE);     // set PCIE1 to enable PCINT
    //PCMSK1 Register controls which pins contribute to the pin change interrupts
    PCMSK1 = (1 << SX1INT) | (1 << SX2INT) ;   // set PCINT to trigger an interrupt on state change

    #elif defined (__AVR_ATtiny85__)
    DDRB = (1 << OUT0) ||(1 << OUT1);

    GIMSK =  (1 << SXPCIE);     // set PCIE0 to enable PCINT
    //PCMSK Register controls which pins contribute to the pin change interrupts
    PCMSK = (1 << SX1INT) | (1 << SX2INT) ;   // set PCINT to trigger an interrupt on state change
    #endif
    sei();
}

/*ISR(TIM0_COMPA_vect) {

static uint8_t pwm_cnt=0;
//uint8_t tmp=0, i=0, j=1;

OCR0A += (uint16_t)T_PWM;

if ((sxData & 0x02) ) { // && (pwm_setting[0] > pwm_cnt) ) {
PORTA |= (1 << PA1);
} else {
PORTA &= ~(1 << PA1) ;
}


if (pwm_cnt==(uint8_t)(PWM_STEPS-1))
pwm_cnt=0;
else
pwm_cnt++;
}   */

int main() {

    setup();

    while(1)
    {
        wdt_reset();
       // _delay_ms(20);

        if (data_received == TRUE) {
            sxCopy = sx;
            data_received = FALSE;
            // new data available, update filter
            // new data can be available at max every 77ms
            for (uint8_t i=0; i<2 ; i++) {
                if (sxCopy & (1 << (i+SUB_ADDRESS))) {
                    counter[i]++;
                    if (counter[i] > MAXCOUNTER ) { counter[i] = MAXCOUNTER; }

                    } else {
                    if (counter[i] != 0) counter[i]--;
                }
            }

            
#if defined (__AVR_ATtiny9__)
#elif defined (__AVR_ATtiny84A__)
            if (counter[0] >= LIMIT) {
                PORTB |= (1 << PB0);
                } else {
                PORTB &= ~(1 << PB0) ;
            }
            
            if ( counter[1] >= LIMIT) {
                PORTA |= (1 << PA1);
                } else {
                PORTA &= ~(1 << PA1) ;
            }
#elif defined (__AVR_ATtiny85__)
            if (counter[0] >= LIMIT) {
                PORTB |= (1 << OUT0);
                } else {
                PORTB &= ~(1 << OUT0) ;
            }
            
            if ( counter[1] >= LIMIT) {
                PORTB |= (1 << OUT1);
                } else {
                PORTB &= ~(1 << OUT1) ;
            }
#else
// this is not supported, must have info about hardware
#error "CPU NOT Supported in sxfunread.c"
#endif

        }

    }
    
}