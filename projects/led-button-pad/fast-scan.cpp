#include "WProgram.h"

inline void writeRow(int col, int a, int b, int c, int d)
{
  PORTD = ((~(0x01 << col)) & B00010111) | ((a>0) << 3) | ((b>0) << 5);
  PORTB = ((c>0) << 1) | ((d>0) << 2);
}

void setup()
{
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);

  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  digitalWrite(0, HIGH);
  digitalWrite(1, HIGH);
  digitalWrite(2, HIGH);
  digitalWrite(4, HIGH);

  digitalWrite(3, LOW);
  digitalWrite(5, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
}

int thing = 0;

void loop ()
{
  int mod = ++thing >> 8;
  writeRow(0, thing & 1, thing & 2, thing & 4, thing & 8);
  writeRow(1, thing & 16, thing & 32, thing & 64, thing & 128);
  writeRow(2, mod & 1, mod & 2, mod & 4, mod & 8);
  writeRow(4, mod & 16, mod & 32, mod & 64, mod & 128);
}
