/*
* FunDec1.c
*
* Created: 11/26/2013 7:52:08 AM
*  Author: mblank
*/

/*
;*************** Adressliste *******************************************************;                                                                                b; Die Adresse bestimmt die Werte für ***basisval*** und ***bitcval***            i;                                                                                t; Gru      = 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00      c; ****************************************************************************   v; basisval = 63, 61, 59, 57, 47, 45, 43, 41, 31, 29, 27, 25, 15, 13, 11,  9      a; ****************************************************************************   l; UA 6     = 00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15	=	86; UA 5     = 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31	=	74; UA 4     = 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47	=	62; UA 3     = 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63	=	50; UA 2     = 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79	=	38; UA 1     = 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95	=	26; UA 0     = 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111	=	14;***********************************************************************************
;   _channel = (15-_baseAdr) + ((6-_dataFrameCount)<<4);
;   83 & 0x0F = 3 => baseAdr = 15-3 =12
; formel:   Gru (baseAdr) = 15 - (channel & 0x0f)
;   83 & 0xf0 = 80 (dec) => DataframeCount = 1
; formel:   dataFrameCount = 6 - (channel >> 4);************** folgende Werte ändern, entsprechend der o. Tabelle *****************;Funktionsdecoder Adresse:    Beispiel *** 52 *** Gru 11, UA3;***********************************************************************************.SET basisval 	= 63    ;Gru.SET bitcval	= 02	;UA;***********************************************************************************
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
