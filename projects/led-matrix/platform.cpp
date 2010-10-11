// NEEDS SPI

#include "WProgram.h"
#include <SPI.h>

#define DATAOUT 11 // MOSI
#define DATAIN 12 // MISO
#define SPICLOCK 13 // SCLK
#define SLAVESELECT 10 // CS

char findTop(unsigned char x);
void setupMap();

volatile bool jumpFlag;
unsigned char timeSinceGap = 0;
char jumping = 0;
char height = 1;
unsigned char environment[8];
unsigned char clouds[8];
unsigned char color_buffer[8][8];

typedef struct
{
    char x, y;
} pair;

pair player;

void buttonJump()
{
    jumpFlag = true;
}

void setup()
{
    // SPI Bus setup
    SPI.begin();
    SPI.setClockDivider(2);

    // Set the pin modes for the RGB matrix
    pinMode(DATAOUT, OUTPUT);
    pinMode(DATAIN, INPUT);
    pinMode(SPICLOCK, OUTPUT);
    pinMode(SLAVESELECT, OUTPUT);

    pinMode(2, INPUT);
    attachInterrupt(1, buttonJump, FALLING);

    // Make sure the RGB matrix is deactivated
    digitalWrite(SLAVESELECT,HIGH);

    delay(100);

    digitalWrite(SLAVESELECT, LOW);
    delay(10);
    SPI.transfer('%');
    SPI.transfer(1);
    delay(10);
    digitalWrite(SLAVESELECT, HIGH);

    setupMap();
}

void setupMap()
{
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
        clouds[i] >>= 1;
    }

    if((!(random(10) == 1)) || (timeSinceGap < 5))
    {
        for(int i = 0; i <= height; i++)
        {
            environment[i] |= 0x01 << 7;
        }
    }
    else
    {
        timeSinceGap = 0;
    }

    timeSinceGap++;

    // create new clouds
    clouds[7] |= (random(4) == 0) << 7;
    clouds[6] |= (random(10) == 0) << 7;

    if(random(5) == 2)
        height += random(2) ? 1 : -1;

    if(height < 0)
        height = 0;

    if(height > 3)
        height = 3;
}

char findTop(unsigned char x)
{
    for(int i = 7; i >= 0; i--)
    {
        if(environment[i] & (0x01 << x))
            return i;
    }

    return -1;
}

void flushBuffer()
{
    digitalWrite(SLAVESELECT, LOW);
    for(int y = 0; y < 8; y++)
        for(int x = 7; x >= 0; x--)
            SPI.transfer(color_buffer[y][x]);
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
        unsigned char env = environment[y];
        unsigned char cloud = clouds[y];

        for(int x = 0; x < 8; x++)
        {
            color_buffer[x][y] = (0x01 & env) ? 0x04 : ((0x01 & cloud) ? 0xFF : 0x00);
            env >>= 1;
            cloud >>= 1;
        }
    }
}

void attemptMoveForward()
{
    if(findTop(player.x + 1) - (player.y) < 0)
    {
        player.x++;
    }
}

void attemptJump()
{
    char top = findTop(player.x);

    if(jumping == 0 && (player.y == top + 1))
    {
        jumping = 3;
    }
}

void deathSequence()
{
    for(int y = 0; y < 8; y++)
    {
        for(int x = 0; x < 8; x++)
        {
            color_buffer[x][y] = 0x00;
        }
    }

    for(int w = 0; w <= 8; w++)
    {
        for(int y = 0; y < w; y++)
        {
            for(int x = 0; x < w; x++)
            {
                color_buffer[x][y] = 0xE0;
            }
        }

        flushBuffer();

        delay(20);
    }

    for(int b = 0; b <= 5; b++)
    {
        for(int y = 0; y < 8; y++)
        {
            for(int x = 0; x < 8; x++)
            {
                color_buffer[x][y] = 0xE0 * (b % 2);
            }
        }

        flushBuffer();
        delay(50);
    }

    for(int f = 7; f >= 0; f--)
    {
        for(int y = 0; y < 8; y++)
        {
            for(int x = 0; x < 8; x++)
            {
                color_buffer[x][y] = f << 5;
            }
        }

        flushBuffer();
        delay(50);
    }
}

void updatePlayerPosition()
{
    //update locations

    if(player.x > 3)
    {
        player.x = 3;
        shiftWorldRight();
    }

    player.y += (jumping ? 1 : -1);

    if(jumping > 0)
        jumping--;

    char top = findTop(player.x);

    if(player.y < top + 1 && top != -1)
        player.y = top + 1;

    if(player.y == 0 && top == -1)
    {
        deathSequence();

        setupMap();
    }
}

void loop()
{
    if(jumpFlag)
    {
        jumpFlag = false;
        attemptJump();
    }
    
    if(digitalRead(2) == LOW)
    {
        attemptMoveForward();
    }

    updatePlayerPosition();

    drawEnvironment();
    drawPlayer();

    flushBuffer();

    delay(120);
}
