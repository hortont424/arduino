#include "WProgram.h"
#include <SPI.h>

//Define the SPI Pin Numbers
#define DATAOUT 11//MOSI
#define DATAIN  12//MISO
#define SPICLOCK  13//sck
#define SLAVESELECT 10//ss

unsigned long num = 0;

//Define the variables we'll need later in the program
char color_buffer [64];

void setup()
{
  //SPI Bus setup
  //SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);	//Enable SPI HW, Master Mode, divide clock by 16    //SPI Bus setup

  SPI.begin();
  SPI.setClockDivider(2);

  //Set the pin modes for the RGB matrix
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(SLAVESELECT,OUTPUT);

  //Make sure the RGB matrix is deactivated
  digitalWrite(SLAVESELECT,HIGH);

}

void loop()
{
  delay(100);

  digitalWrite(SLAVESELECT, LOW);
  delay(10);
  SPI.transfer('%');
  SPI.transfer('1');
  delay(10);
  digitalWrite(SLAVESELECT, HIGH);

  while(1)
  {
    unsigned long temp = num++;

    for(int i = 0; i < 64; i++)
    {
      do
      {
        color_buffer[i] = random(255);
      }
      while(color_buffer[i] == '%');

      /*if(temp & 0x01)
        color_buffer[i] = 0xE0;
      else
        color_buffer[i] = 0x00;

      temp = temp >> 1;*/
    }

    //Activate the RGB Matrix
    digitalWrite(SLAVESELECT, LOW);
    //delay(1);
    //Send the color buffer to the RGB Matrix
    for(int LED=0; LED<64; LED++){
      //spi_transfer(color_buffer[LED]);
      SPI.transfer(color_buffer[LED]);
    }
    //Deactivate the RGB matrix.
    //delay(1);
    digitalWrite(SLAVESELECT, HIGH);
  }
}
