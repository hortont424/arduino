// NEEDS SPI

#include "WProgram.h"
#include <SPI.h>

#define DATAOUT 11 // MOSI
#define DATAIN 12 // MISO
#define SPICLOCK 13 // SCLK
#define SLAVESELECT 10 // CS

unsigned char findTop(unsigned char x);

char jumping = 0;
char height = 1;
unsigned char environment[8];
unsigned char color_buffer[8][8];

typedef struct
{
    char x, y;
} pair;

pair player;

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

    // set up map
    for(int i = 0; i < 8; i++)
    {
        environment[i] = 0x00;
    }

    for(int i = 0; i <= height; i++)
        environment[i] = 0xFF;

    player.x = 1;
    player.y = findTop(player.x) + 1;
}

void shiftWorldRight()
{
    for(int i = 0; i < 8; i++)
    {
        environment[i] >>= 1;
    }

    if(!(random(10) == 1))
    {
        for(int i = 0; i <= height; i++)
        {
            environment[i] |= 0x01 << 7;
        }
    }

    if(random(5) == 2)
        height += random(2) ? 1 : -1;

    if(height < 0)
        height = 0;

    if(height > 3)
        height = 3;
}

unsigned char findTop(unsigned char x)
{
    for(int i = 7; i >= 0; i--)
    {
        if(environment[i] & (0x01 << x))
            return i;
    }

    return 0;
}

void flushBuffer()
{
    digitalWrite(SLAVESELECT, LOW);
    for(int y = 7; y >= 0; y--)
        for(int x = 7; x >= 0; x--)
            SPI.transfer(color_buffer[x][y]);
    digitalWrite(SLAVESELECT, HIGH);
}

void drawPlayer()
{
    if(player.y < 8)
        color_buffer[player.x][player.y] = 0xE0;
}

void drawEnvironment()
{
    for(int y = 0; y < 8; y++)
    {
        unsigned char temp = environment[y];

        for(int x = 0; x < 8; x++)
        {
            color_buffer[x][y] = (0x01 & temp) * 0x01;
            temp >>= 1;
        }
    }
}

void updatePlayerPosition()
{
    //update locations

    player.x++;

    if(player.x > 6)
    {
        player.x = 6;
        shiftWorldRight();
    }

    player.y += (jumping ? 1 : -1);

    if(jumping > 0)
        jumping--;

    unsigned char top = findTop(player.x);

    if(player.y < top + 1)
        player.y = top + 1;

    if(random(10) == 1 && jumping == 0 && (player.y == top + 1))
    {
        jumping = 3;
    }
}

void loop()
{

    updatePlayerPosition();

    drawEnvironment();
    drawPlayer();

    flushBuffer();

    delay(100);
}
