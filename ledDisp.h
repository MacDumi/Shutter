#ifndef LedDisp
#define LedDisp


#include <avr/io.h>

#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

//connections
#define CLK         5
#define LOAD        6
#define DATA        7

//state of the display
#define OFF     0x0C00
#define ON      0x0C01


#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) // clear bit
#define sbi(port,bit) 	((port) |= (1 << (bit)) )  //set bit

#define CLK_HIGH    sbi(PORTD, CLK) //clock high
#define CLK_LOW    cbi(PORTD, CLK) //clock low

#define DATA_HIGH    sbi(PORTD, DATA) //data high
#define DATA_LOW    cbi(PORTD, DATA) //data low

#define LOAD_HIGH    sbi(PORTD, LOAD) //load high
#define LOAD_LOW    cbi(PORTD, LOAD) //load low

void init();
void sendOUT (unsigned int data) ;
void test();
void shutdown(int state);
void decode(uint8_t digits);
void scanLimit(uint8_t digits);

void setIntensity(uint8_t intensity);
void printInt (uint16_t number);

void printFloat (float number, uint8_t dp);
void attention();
#endif
