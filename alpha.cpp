#include "WProgram.h"
#include <SPI.h>

#define DATAOUT 11 // MOSI
#define DATAIN 12 // MISO
#define SPICLOCK 13 // SCLK
#define SLAVESELECT 10 // CS

char color_buffer[64];

unsigned char space[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned char uppercase[26][8] = {
    {0x38, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0x00},
    {0x78, 0x44, 0x44, 0x78, 0x44, 0x44, 0x78, 0x00},
    {0x38, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x00},
    {0x78, 0x44, 0x44, 0x44, 0x44, 0x44, 0x78, 0x00},
    {0x7C, 0x40, 0x40, 0x78, 0x40, 0x40, 0x7C, 0x00},
    {0x7C, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x00},
    {0x38, 0x44, 0x40, 0x5C, 0x44, 0x44, 0x38, 0x00},
    {0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0x00},
    {0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00},
    {0x0E, 0x04, 0x04, 0x04, 0x24, 0x24, 0x18, 0x00},
    {0x44, 0x44, 0x48, 0x70, 0x48, 0x44, 0x44, 0x00},
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x78, 0x00},
    {0x82, 0xC6, 0xAA, 0x92, 0x82, 0x82, 0x82, 0x00},
    {0x44, 0x64, 0x54, 0x54, 0x4C, 0x44, 0x44, 0x00},
    {0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00},
    {0x70, 0x48, 0x48, 0x70, 0x40, 0x40, 0x40, 0x00},
    {0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x0C},
    {0x78, 0x44, 0x44, 0x78, 0x50, 0x48, 0x44, 0x00},
    {0x38, 0x44, 0x40, 0x38, 0x04, 0x44, 0x38, 0x00},
    {0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00},
    {0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00},
    {0x44, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x00},
    {0x82, 0x82, 0x82, 0x54, 0x54, 0x28, 0x28, 0x00},
    {0x44, 0x44, 0x28, 0x10, 0x28, 0x44, 0x44, 0x00},
    {0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x00},
    {0x7C, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7C, 0x00}
};

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

unsigned char characterNumber = 0;
char * str = "HELLOMATTMAP";

void loop()
{
    while(1)
    {
        unsigned char id = str[characterNumber] - 'A';

        for(int i = 0; i < 8; i++)
        {
            unsigned char v = uppercase[id][i];

            for(int j = 0; j < 8; j++)
            {
                color_buffer[(8 * i) + j] = (v & 0x01) * 0xE0;
                v >>= 1;
            }
        }

        if(++characterNumber > strlen(str) - 1)
            characterNumber = 0;

        digitalWrite(SLAVESELECT, LOW);
        for(int LED=0; LED<64; LED++)
            SPI.transfer(color_buffer[LED]);
        digitalWrite(SLAVESELECT, HIGH);

        delay(500);
    }
}