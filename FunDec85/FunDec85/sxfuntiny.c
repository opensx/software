/*
* sxfuntiny.c
*
* Created: 11/26/2013 7:57:25 AM
*  Author: mblank
*/
/* 3 different states are distinguished
1. SNYC = looking for a SYNC signal
2. ADDR = (after SYNC received) look for base address (0..15)
3. DATA = (after ADDR decoded) decode the 7 data-bytes

SX Timing
1   Bit             50 us
1   Kanal          600 us (= 12 Bit)
1   Grundrahmen    ca. 4,8 ms
1   Gesamtrahmen   ca.  80 ms (= 16 Grundrahmen)  */

#include "sxfuntiny.h"

volatile uint8_t _pwm_cnt = 0;
uint8_t fade[FIMAX+1] = {0,1,2,3,4,6,8,12,17,24,34,49,69,99,140,200};

uint8_t _lastin;    // last data bit
uint8_t _lastinputb;  // last portb input data
uint8_t _toggle =0;
uint8_t _zeroCount =0;
uint8_t _adrCount =0;
uint8_t _state = SYNC;
uint8_t _dataBitCount;    // bit counting
uint8_t _dataFrameCount;  // frame counting
uint8_t _data = 0;    // 1 data uint8_t
uint8_t _baseAdr = 0;   // base address
uint8_t _triggerBaseAdr =0;
uint8_t _bit;
uint8_t _channel;   // channel from 0 to 15, B3..B0 in sync data
uint8_t _sx_tmp;
uint8_t _data_rec;

// 0  0  0  1  X   1  B3  B2  1  B1  B0  1 == sync frame of 12 bits


void resetSync() {
    _dataFrameCount=0;
    _data = 0;
    _state =SYNC;
    _zeroCount = 0;
    _dataBitCount=0;
    _sx_tmp=0;
    _data_rec=0;
}


void switchAdr() {
    // a SYNC signal was received, now look for a valid
    // base address
    switch(_adrCount) {
        case 0:   // this is the GLEISSPANNUNG bit
        _baseAdr=0;
        break; // ignore
        case 1:
        case 4:
        if (_bit != HIGH) {
            resetSync();
        }
        break; // ignore
        case 2:  // B3
        bitWrite(_baseAdr,3,_bit);
        break;
        case 3:  // B2
        bitWrite(_baseAdr,2,_bit);
        break;
        case 5:  // B1
        bitWrite(_baseAdr,1,_bit);
        break;
        case 6:  // B0
        bitWrite(_baseAdr,0,_bit);
        break;
        case 7: // last "1"
        // _baseAdr is complete !
        if ( (_bit != HIGH)
        ||
        (_baseAdr != myBaseAdr) ){
            resetSync();
            }  else {
            // advance to next state - next we are looking
            // for the 7 data bytes (i.e. 7 SX Channels)
            _state = DATA;
            _dataFrameCount = 0;
            _dataBitCount = 0;
            // _data=0; will be reset in first call to switchData
        }
        break;
    }
}

void switchData() {
    // continue reading _data
    // a total of 7 DATA blocks will be received
    // for a certain base-address

    // calc sx channel from baseAdr and dataFrameCount
    _channel = (15-_baseAdr) + ((6-_dataFrameCount)<<4);

    switch(_dataBitCount) {
        case 2:  // "Trenn_bits"
        case 5:
        case 8:
        if (_bit != HIGH) {
            resetSync();
            } else {
            _dataBitCount++;
        }
        break; // ignore
        case 0:  // D0
        _data=0;
        bitWrite(_data,0,_bit);
        _dataBitCount++;
        break;
        case 1:  // D1
        bitWrite(_data,1,_bit);
        _dataBitCount++;
        break;
        case 3:  // D2
        bitWrite(_data,2,_bit);
        _dataBitCount++;
        break;
        case 4:  // D3
        bitWrite(_data,3,_bit);
        _dataBitCount++;
        break;
        case 6:  // D4
        bitWrite(_data,4,_bit);
        _dataBitCount++;
        break;
        case 7:  // D5
        bitWrite(_data,5,_bit);
        _dataBitCount++;
        break;
        case 9:  // D6
        bitWrite(_data,6,_bit);
        _dataBitCount++;
        break;
        case 10: // D7
        bitWrite(_data,7,_bit);
        _dataBitCount++;
        break;
        case 11:  // == MAX_DATABITCOUNT
        // _bit value should always equal HIGH
        if (_bit != HIGH) {
            resetSync();
            } else {
            if (_channel == DECODER_ADDRESS) {
                _sx_tmp = _data;
                _data_rec = TRUE;
            }
            // increment dataFrameCount to move on the next DATA byte
            // check, if we already reached the last DATA block - in this
            // case move on to the next SX-Datenpaket, i.e. look for SYNC
            _dataFrameCount ++;
            if (_dataFrameCount == MAX_DATACOUNT) {
                if (_data_rec) {
                    sx = _sx_tmp;
                    data_received = TRUE;  // indicate new data for main prog.
                }
                // and move on to SYNC _state
                resetSync();
                } else {
                _dataBitCount = 0;  // reset _bit counter
                // continue with reading next _data uint8_t
            }
        }
    }  //end switch/case _dataBitCount
}


// assuming PCIE1 is set

ISR(INTERUPT_VECTOR)  {
    // Pin Change Interrupt
    
    uint8_t _inputb, _in;
    _inputb = PINB & ( (1 << SX1) | (1 << SX2) );
    
    if  (_inputb == (1 << SX1)) {
        // SX1 toggled and is high
        _in = 1;
        } else if  (_inputb == (1 << SX2)) {
        // SX2 toggled and is high
        _in = 0;
        } else {
        return;
    }
    
    //_lastinputb = _inputb;
    if (_lastin == _in) {
        _bit = LOW;
        } else {
        _bit = HIGH;
    }
    _lastin = _in;

    switch(_state) {
        case SYNC:
        if (_bit == LOW )  {
            _zeroCount++;
            } else {
            if (_zeroCount == 3)  {    // sync bits 0 0 0 1 found
                _state = ADDR;         // advance to next state
                _baseAdr = 0;
                _adrCount = 0;
                } else {  // no valid sync, try again ...
                _zeroCount = 0;       // reset _zeroCounter
            } // endif _zeroCount
        }  // endif _bit==LOW
        break;
        case ADDR:
        switchAdr();
        _adrCount++;
        break;
        case DATA:
        switchData();
    }
    

}