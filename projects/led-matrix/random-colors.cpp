// NEEDS SPI

#include "WProgram.h"
#include <SPI.h>

#define DATAOUT 11 // MOSI
#define DATAIN 12 // MISO
#define SPICLOCK 13 // SCLK
#define SLAVESELECT 10 // CS

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
    for(int i = 0; i < 64; i++)
    {
        do
        {
            color_buffer[i] = random(255);
        }
        while(color_buffer[i] == '%');
    }

    digitalWrite(SLAVESELECT, LOW);
    for(int LED=0; LED<64; LED++)
        SPI.transfer(color_buffer[LED]);
    digitalWrite(SLAVESELECT, HIGH);
}
