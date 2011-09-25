#include "WProgram.h"

#define CLK_DPIN 0x04
#define OUT_DPIN 0x02
#define IN_DPIN 0x01
#define CS_DPIN 0x08

#define SET_DPIN(x) (PORTD |= x)
#define GET_DPIN(x) ((PIND & x) != 0)
#define CLEAR_DPIN(x) (PORTD &= ~x)

int LED = 8;


byte cmd_RDSR = B00000101;
byte cmd_WRSR = B00000001;
byte cmd_SR = B01000001;

void setup()
{
    DDRD |= (CS_DPIN | CLK_DPIN | OUT_DPIN);

    pinMode(LED, OUTPUT);

    SET_DPIN(CS_DPIN);
}


void spi_write(byte b)
{
    byte work = b;

    for(int i = 0; i < 8; ++i)
    {
        CLEAR_DPIN(OUT_DPIN);
        SET_DPIN(((work & 0x80) >> 6) | CLK_DPIN);
        work <<= 1;
        CLEAR_DPIN(CLK_DPIN);
    }
}

byte spi_read()
{
    byte read = 0;

    for(int i = 0; i < 8; ++i)
    {
        SET_DPIN(CLK_DPIN);
        read |= GET_DPIN(IN_DPIN);
        CLEAR_DPIN(CLK_DPIN);
        read <<= 1;
    }

    return read;
}

void loop()
{
    CLEAR_DPIN(CS_DPIN);
    spi_write(cmd_WRSR);
    spi_write(cmd_SR);
    SET_DPIN(CS_DPIN);
    CLEAR_DPIN(CS_DPIN);
    spi_write(cmd_RDSR);
    byte output = spi_read();
    SET_DPIN(CS_DPIN);

    byte showUs = output;
    for(int i = 1; i <= 8; i++)
    {
        digitalWrite(LED, showUs & 0x80);
        delay(10);
        showUs <<= 1;
    }

    delay(1000);
}