// NEEDS Ethernet

#include "WProgram.h"
#include "Ethernet.h"
#include "OSCClass.h"

const int ledPin =  7;
const int rxPin = 2;

OSCMessage recMes;
OSCMessage sendMes;
OSCClass osc(&recMes);

byte serverMac[] = { 0x00, 0x22, 0x15, 0x6b, 0x6a, 0x42 };
byte serverIp[] = { 10, 0, 1, 30 };
int serverPort = 10001;

byte destIp[] = { 10, 0, 1, 23 };
int destPort = 60000;

char *topAddress="/ard";
char *subAddress[1]={ "/test1"  };

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(rxPin, INPUT);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  Ethernet.begin(serverMac, serverIp);

  osc.begin(serverPort);
  osc.flush();

  sendMes.setIp(destIp);
  sendMes.setPort(destPort);
  sendMes.setTopAddress(topAddress);
  sendMes.setSubAddress(subAddress[0]);
}

int count = 0;
int total = 0;

void loop()
{
  count += digitalRead(rxPin);
  total++;

  if(total >= 100)
  {
    float good = !(count >= (total * 0.9));
    digitalWrite(ledPin, good);
    total = count = 0;
    sendMes.setArgs("f" , &good);
    osc.sendOsc(&sendMes);
  }

  delay(1);
}
