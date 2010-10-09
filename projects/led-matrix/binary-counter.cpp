// NEEDS SPI

#include "WProgram.h"
#include <SPI.h>

#define DATAOUT 11 // MOSI
#define DATAIN 12 // MISO
#define SPICLOCK 13 // SCLK
#define SLAVESELECT 10 // CS

unsigned long num = 0;
char color_buffer[64];

void setup()
{
    // SPI Bus setup
    SPI.begin();
    SPI.setClockDivider(2);

    // Set the pin modes for the RGB matrix
    pinMode(DATAOUT, OUTPUT);
    pinMode(DATAIN, INPUT);
    pinMode(SPICLOCK,OUTPUT);
    pinMode(SLAVESELECT,OUTPUT);

    // Make sure the RGB matrix is deactivated
    digitalWrite(SLAVESELECT,HIGH);

    delay(100);

    digitalWrite(SLAVESELECT, LOW);
    delay(10);
    SPI.transfer('%');
    SPI.transfer(1);
    delay(10);
    digitalWrite(SLAVESELECT, HIGH);
}

void loop()
{
    unsigned long temp = num++;

    for(int i = 0; i < 64; i++)
    {
        if(temp & 0x01)
            color_buffer[i] = 0xE0;
        else
            color_buffer[i] = 0x00;

        temp = temp >> 1;
    }

    digitalWrite(SLAVESELECT, LOW);
    for(int LED=0; LED<64; LED++)
        SPI.transfer(color_buffer[LED]);
    digitalWrite(SLAVESELECT, HIGH);
}
