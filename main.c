#define F_CPU 8000000UL

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "ledDisp.h"

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) // clear bit
#define sbi(port,bit) 	((port) |= (1 << (bit)) )  //set bit

//inputs and outputs
#define LED         0 //trigger indicator
#define TRIG        2 //trigger in
#define FET         1 //MOSFET
#define ExBt        5 //external button to open the shutter
#define Bt0         2 //buttons to increment the counter
#define Bt1         3
#define Bt2         4
#define Bt3         0
#define Bt4         1
#define AUTO        4 //mode select
#define SHUTTER     3 //shuter mode indicator LED


volatile int count;
int auto_sh= 0, active = 0, s_flag=0, shutter=0, done_flag=0;
uint8_t lastB0=1, lastB1=1, lastB2=1, lastB3=1, lastB4=1, lastExBt=1;
int count_initial;
uint8_t _ext[]={0x00, 0x0F, 0x37, 0x4F, 0x00}, _auto[]={0x00, 0x7E, 0x0F, 0x3E, 0x77},
            _done[]={0x4F, 0x15, 0x7E, 0x3D, 0x00}; //characters for words

void autotest()
{
  sbi(PORTD, AUTO);
  sbi(PORTD, SHUTTER);
  sbi(PORTC, LED);
  test();
  cbi(PORTD, AUTO);
  cbi(PORTD, SHUTTER);
  cbi(PORTC, LED);
}
void setup()
{
  //interrupt on INT_0 falling edge
  sbi(MCUCR, ISC01);
  cbi(MCUCR, ISC00);
  sbi(GICR, INT0);
  sbi(SREG, 7);
  //setting up I/O
  sbi(DDRC, LED);
  cbi(DDRD, TRIG);
  cbi(DDRC, ExBt);
  cbi(DDRC, Bt0);
  cbi(DDRC, Bt1);
  cbi(DDRC, Bt2);
  cbi(DDRB, Bt3);
  cbi(DDRB, Bt4);
  sbi(DDRC, FET);
  sbi(DDRD, AUTO);
  sbi(DDRD, SHUTTER);

  init(); //init display
  autotest(); //test the display
  _delay_ms(500);
  scanLimit(0x05); //scan 5digits
  decode(0x1F);//decode 5 digits
  setIntensity(0x0A); //set intensity to 0x0A
  shutdown(ON); //change state to ON
}

void write_auto()
{
  decode(0); //turn off decoding
  //display "AUTO" word
  for (int i=1; i<6; i++)
  {
    sendOUT((i<<8)|_auto[i-1]);
  }
  _delay_ms(500);
  decode(0x1F); //turn ON decoding
}

void write_done()
{
  decode(0); //turn off decoding
  //display "DONE"
  for (int i=1; i<6; i++)
  {
    sendOUT((i<<8)|_done[i-1]);
  }
  _delay_ms(400);
  decode(0x1F); //turn on decoding
  done_flag=0; //set the flag to false
}


void write_ext()
{
  decode(0); //turn off decoding
  //display "EXT"
  for (int i=1; i<6; i++)
  {
    sendOUT((i<<8)|_ext[i-1]);
  }
  _delay_ms(500);
  decode(0x1F); //turn on decoding
}

void button0()
{
  //+100 counts
  if(!shutter)
  {
    uint8_t temp = (count/100)%10;
    count = (temp!=9)? count+100 : count-900;

  }
}

void button2()
{
  //mode select
  auto_sh = (auto_sh ==0)? 1:0;
  if(auto_sh)
  {
    sbi(PORTD, AUTO);
    write_auto();


  } else
  {
    cbi(PORTD, AUTO);
    write_ext();

  }
}

void button1()
{
    //+1000 counts
  if(!shutter)
  {
    uint8_t temp = (count/1000)%10;
    count = (temp!=9)? count+1000 : count-9000;

  }
}

void button3()
{
    //+10 counts
  if(!shutter)
  {
    uint8_t temp = (count/10)%10;
    count = (temp!=9)? count+10 : count-90;

  }
}

void button4 ()
{
    //+1 count
  if(!shutter)
  {
    uint8_t temp = count%10;
    count = (temp!=9)? count+1 : count-9;

  }
}

void checkButtons()
{
  //check buttons
  uint8_t B0 = (PINC & _BV(PC2)) ? 1 : 0;
  uint8_t B1 = (PINC & _BV(PC3)) ? 1 : 0;
  uint8_t B2 = (PINC & _BV(PC4)) ? 1 : 0;
  uint8_t B3 = (PINB & _BV(PB0)) ? 1 : 0;
  uint8_t B4 = (PINB & _BV(PB1)) ? 1 : 0;
  uint8_t EXT = (PINC & _BV(PC5)) ? 1 : 0;
  if (B0 != lastB0 && B0== 0) button0();
  if (B1 != lastB1 && B1== 0) button1();
  if (B2 != lastB2 && B2== 0) button2();
  if (B3 != lastB3 && B3== 0) button3();
  if (B4 != lastB4 && B4== 0) button4();
  if (EXT != lastExBt && EXT== 0)
  {
    s_flag=1;
    count_initial=count;
  }
  lastB0 = B0;
  lastB1 = B1;
  lastB2 = B2;
  lastB3 = B3;
  lastB4 = B4;
  lastExBt = EXT;
}

void loop(void)
{
  //infinite loop
  checkButtons();
  (done_flag)? write_done() : printInt(count);

}

int main(void) {
  //arduino like routine
  setup();
  for(;;) {
    loop();
  }
}

ISR(INT0_vect)
{
  //external interrupt
  if(auto_sh && shutter)
  {

    if(count==1)
    {
      s_flag=1;
      count=count_initial+1; //when finished set the counter to original value
    }
    if(count!=0) count--; //decrement the counter
  }

  if(s_flag)
  {
    _delay_ms(10);
    if(!shutter )
    {
      if (count!=0 )
      {
        //open the shutter
        sbi(PORTC, FET);
        sbi(PORTD, SHUTTER);
      }
      else if(!auto_sh)
      {
        //open the shutter if in manual mode
        sbi(PORTC, FET);
        sbi(PORTD, SHUTTER);
      }
    } else
    {
      //close shutter
      cbi(PORTC, FET);
      cbi(PORTD, SHUTTER);
      if(auto_sh) done_flag=1;
    }
    shutter=!shutter; //set the flag
    s_flag=0;
  }

  PORTC^=(1<<LED); //trigger indicator
}
