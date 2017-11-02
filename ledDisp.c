#include "ledDisp.h"

void init()
{
  //initialize the pins
  sbi(DDRD, CLK);
  sbi(DDRD, LOAD);
  sbi(DDRD, DATA);
}

void sendOUT (unsigned int data)  //send 16 bits of data
{
  //starting with the most significant bit
  unsigned int i = 0x8000; // to send 8 bits use 0x80
  CLK_LOW;               // clock low.
  LOAD_LOW;              // latch low.

  while(i > 0x0000)
  {
    if (data & i)   // Look at just one data bit
    {
      DATA_HIGH;  //if the bit is high ->DATA_HIGH
    }
    else
    {
      DATA_LOW;   //else ->DATA_LOW
    }
    CLK_HIGH;        // clock high
    i >>= 1;      // shift i to the right
    CLK_LOW;         // clock low
  }
  LOAD_HIGH;            // spin RCK high to latch SR data.
}

void test()
{
  //test the display
    sendOUT(0xFFFF);
    _delay_ms(250);
    sendOUT(0xFF00);
}

void shutdown(int state)
{
  //change state
  sendOUT(state);
}

void decode(uint8_t digits)
{
  //decode digits
 sendOUT((0x09 << 8)|digits);
}

void scanLimit(uint8_t digits)
{
  //nr of digits
  sendOUT((0x0B <<8)|digits);
}

void setIntensity(uint8_t intensity)
{
  //set the intensiy of the display
  sendOUT((0x0A << 8)|intensity);
}

void printInt (uint16_t number)
{
  //print an integer
  uint16_t temp=number;
  uint8_t digit=0;
  for (uint8_t i=0; i<5; i++)
  {
   if (temp==0 && i!=0)
    {
      digit = 0x0F;

    }
    else
    {
      digit = temp % 10;
      temp = temp/10;
    }
    sendOUT(((1+i)<<8)| digit);
  }
}

void printFloat (float number, uint8_t dp)
{
  //print a float
  int temp=(int)(number*pow(10,dp));
  uint8_t digit=0;
  for (uint8_t i=0; i<5; i++)
  {
   if (temp!=0)
    {
    digit = temp % 10;
    temp = temp/10;
    if(i==dp) digit= digit | 0x80;
    }
    else
    {
    digit = 0x0F;
    }
    sendOUT(((1+i)<<8)| digit);
  }
}

void attention()
{
  //blink the display
  shutdown(OFF);
  _delay_ms(150);
  shutdown(ON);
  _delay_ms(150);
}
