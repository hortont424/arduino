#include "WProgram.h"

/*
 *  Arduino Intervalometer
 *  Tim Horton, 2008
 */

enum {INTERVAL, BULB, INTERVALBULB, TRIGGER};

/*
 *  TODOs
 */

// Easter egg!
// Speed up encoder more, later in the 'minute' range
// Slow down encoder in second range/normally
// Hour representation?
// Either need to support more digits, or arbitrarily limit durations
// Fix when trigger is triggered and someone (ROBB) stops it (and it keeps open)
// Why does trigger reset not get put back when we're done triggering!?
// External power supply?
// preferences: save if we want to use the LED, contrast, etc. in eeprom

/*
 *  Pin Assignment
 */

/* Analog Pins */

int triggerInput =  2; // Feedback for trigger reset

/* Digital Pins */

int cameraShutter = 0;	// Pulse low for shutter release

int triggerReset =  1;	// Resets trigger latch on low pulse
						// Prevents trigger if held low

int encoderPinA =	2;	// HW interrupt
int encoderPinB =	3;	// HW interrupt

int lcdPower = 		4;	// Pull low to power the LCD backlight
int lcdEnable =		7;	// LCD Enable - pin 6 on LCD module
int lcdClock =		8;	// Shift register clock
int lcdData =		9;	// Shift register data

int potSelect =		10; // SPI (SS) for digital pots
int potData =		11; // SPI (MOSI) for digital pots
int badSPIpin = 	12;	// SPI (MISO) for nothing, unusable
int potClock =		13; // SPI (SCK) for digital pots

int buttonA = 		15; // Left pushbutton
int buttonB = 		16; // Right pushbutton
int encoderButton =	17; // Encoder pushbutton

/*
 *  Digital Pot Assignment
 */

int ledRPot = 3;
int ledGPot = 1;
int ledBPot = 0;

int timerPot = 2;
int contrastPot = 5;

/////////////////////////////

volatile int writing = 0;

int currentMode = INTERVAL, selected = 0, changeSelected = 0;

// Debouncing time variables
unsigned long lastToggleRunning = 0;
unsigned long lastModeUpdate = 0;
unsigned long lastSelectedUpdate = 0;
unsigned long lastShutter = 0;

// Update statuses

volatile int updateHeader = 1, updateEncoder = 1, running = 0;

// Exposure and Timelapse durations
volatile long exposureTime = 0, lapseTime = 0;
volatile int exposureRepresentation = 0, lapseRepresentation = 0;

volatile int contrastValue = 1000;

/*
 *  Higher level hardware functions
 */

void pulsePin(int pin, int value)
{
	digitalWrite(pin, !value);
	delayMicroseconds(1);
	digitalWrite(pin, value);
	delayMicroseconds(1);
	digitalWrite(pin, !value);
	delay(1);
}

void parallelShiftOut(int firstPin, int lastPin, int & value)
{
	int i;
	for(i = firstPin; i <= lastPin; i++)
	{
		digitalWrite(i, value & 0x01);
		value >>= 1;
	}
}

/*
 *  Digital Pot Control Functions
 */

char spi_transfer(volatile char data)
{
	SPDR = data;
	while (!(SPSR & (1<<SPIF))) {};
	return SPDR;
}

void digitalPotInit()
{
	byte i;
	byte clr;
	pinMode(potData, OUTPUT);
	pinMode(potClock,OUTPUT);
	pinMode(potSelect,OUTPUT);
	digitalWrite(potSelect,HIGH);
	SPCR = (1<<SPE)|(1<<MSTR);
	clr=SPSR;
	clr=SPDR;
	delay(10);
	for (i=0;i<6;i++)
		write_pot(i,255);
}

byte write_pot(int address, int value)
{
	digitalWrite(potSelect,LOW);
	spi_transfer(address);
	spi_transfer(value);
	digitalWrite(potSelect,HIGH);
}

/*
 *  LCD Control Functions
 */

void lcdDataWrite(byte a)
{
	if(writing)
		return;

	writing = 1;
	shiftOut(lcdData, lcdClock, LSBFIRST, 0x20 + ((a >> 4) & 0xF));
	pulsePin(lcdEnable, HIGH);

	shiftOut(lcdData, lcdClock, LSBFIRST, 0x20 + (a & 0xF));
	pulsePin(lcdEnable, HIGH);

	delay(1);

	writing = 0;
}

void lcdCommandWrite(int a)
{
	if(writing)
		return;

	writing = 1;
	shiftOut(lcdData, lcdClock, LSBFIRST, (a >> 4) & 0xF);
	pulsePin(lcdEnable, HIGH);

	shiftOut(lcdData, lcdClock, LSBFIRST, a & 0xF);
	pulsePin(lcdEnable, HIGH);

	delay(1);

	writing = 0;
}

void lcdNumberWrite(int nr)
{
	int n1 = nr/100;
	int n2 = (nr - n1 * 100) / 10;

	if(n2)
		lcdDataWrite('0' + n2);

	nr = nr - n1 * 100 - n2 * 10;
	lcdDataWrite('0' + nr);
}

void lcdHome(int row)
{
	lcdCommandWrite(0x02);

	delay(4);

	if(row == 1)
		lcdCommandWrite(0xC0);

	delay(1);
}

void lcdClear()
{
	lcdCommandWrite(0x01);
	delay(4);
}

/*
 *  Hardware initialization functions
 */

void lcdInit()
{
	int i = 0;

	pinMode(lcdEnable, OUTPUT);
	pinMode(lcdData, OUTPUT);
	pinMode(lcdClock, OUTPUT);

	digitalWrite(lcdClock,LOW);
	digitalWrite(lcdData,LOW);
	digitalWrite(lcdEnable,LOW);

	// there's a chance that when we drop to not running
	// on top of the arduino bootloader, we'll need a somewhat
	// significant delay here (called for in the spec, but the bl is slow)

	lcdCommandWrite(0x03); delay(5);
	lcdCommandWrite(0x03); delay(1);
	lcdCommandWrite(0x03); delay(1);
	lcdCommandWrite(0x02); delay(4);
	lcdCommandWrite(0x06); delay(1);
	lcdCommandWrite(0x0C); delay(1);
	lcdCommandWrite(0x01); delay(4);
	lcdCommandWrite(0x80); delay(1);

	pinMode(lcdPower, OUTPUT);
	digitalWrite(lcdPower, HIGH);
}

void encoderInit()
{
	pinMode(encoderPinA, INPUT);
	digitalWrite(encoderPinA, HIGH);
	pinMode(encoderPinB, INPUT);
	digitalWrite(encoderPinB, HIGH);

	attachInterrupt(0, doEncoderA, CHANGE);
	attachInterrupt(1, doEncoderB, CHANGE);
}

void setup (void)
{
	pinMode(cameraShutter, OUTPUT);
	digitalWrite(cameraShutter, HIGH);

	pinMode(triggerReset, OUTPUT);
	// keep the reset high except when in trigger mode, so we don't accidentally trigger!
	digitalWrite(triggerReset, LOW);

	digitalPotInit();
	lcdInit();
	encoderInit();
	writeLED(0,0,128);

	write_pot(timerPot, 255);
}

void incrementValue()
{
	if(changeSelected)
	{
		contrastValue++;
		updateContrast();
		return;
	}

	if(selected == 0)
	{
		if(lapseRepresentation == 0)
			lapseTime++;
		else
			lapseTime += 60;
	}
	else
	{
		if(exposureRepresentation == 0)
			exposureTime++;
		else
			exposureTime += 60;

		if(exposureTime > lapseTime)
			lapseTime = exposureTime;
	}
}

void decrementValue()
{
	if(changeSelected)
	{
		contrastValue--;
		updateContrast();
		return;
	}

	if(selected == 0)
	{
		if(lapseRepresentation == 0 || lapseTime == 60) // careful around transition; thanks, nate
			lapseTime--;
		else
			lapseTime -= 60;

		if(lapseTime < 0)
			lapseTime = 0;
	}
	else
	{
		if(exposureRepresentation == 0 || exposureTime == 60)
			exposureTime--;
		else
			exposureTime -= 60;

		if(exposureTime < 0)
			exposureTime = 0;
	}
}

void updateTimeRepresentations()
{
	if(lapseTime >= 60)
		lapseRepresentation = 1;
	else
		lapseRepresentation = 0;

	if(exposureTime >= 60)
		exposureRepresentation = 1;
	else
		exposureRepresentation = 0;

	updateEncoder = 1;
}

void doEncoderA()
{
	if(running || writing)
		return;

	noInterrupts();
	delayMicroseconds(3000); // maximum bounce time, accd. to spec.

	if (digitalRead(encoderPinA) == HIGH)
	{
		if (digitalRead(encoderPinB) == LOW)
			incrementValue();
		else
			decrementValue();
	}
	else
	{
		if (digitalRead(encoderPinB) == HIGH)
			incrementValue();
		else
			decrementValue();
	}

	updateTimeRepresentations();

	updateEncoder = 1;
	interrupts();
}

void doEncoderB()
{
	if(running || writing)
		return;

	noInterrupts();
	delayMicroseconds(3000);

	if (digitalRead(encoderPinB) == HIGH)
	{
		if (digitalRead(encoderPinA) == HIGH)
			incrementValue();
		else
			decrementValue();
	}
	else
	{
		if (digitalRead(encoderPinA) == LOW)
			incrementValue();
		else
			decrementValue();
	}

	updateTimeRepresentations();

	updateEncoder = 1;
	interrupts();
}

void switchModes()
{
	unsigned long diff = (millis() - lastModeUpdate);

	if(diff < 300) // careful about the overflow...
		return;

	lastModeUpdate = millis();

	currentMode++;
	if(currentMode > 3)
		currentMode = 0;

	if(currentMode == BULB)
		selected = 1;
	else
		selected = 0;

	if(currentMode == TRIGGER)
		digitalWrite(triggerReset, HIGH);
	else
		digitalWrite(triggerReset, LOW);

	if(currentMode == TRIGGER)
		writeLED(200, -1, 0);
	else if(currentMode == BULB)
		writeLED(0, -1, 128);
	else if(currentMode == INTERVALBULB)
		writeLED(0, -1, 128);
	else if(currentMode == INTERVAL)
		writeLED(0, -1, 128);


	updateHeader = 1;
}

void switchSelected()
{
	if(currentMode == INTERVALBULB)
	{
		unsigned long diff = (millis() - lastSelectedUpdate);

		if(diff < 300) // careful about the overflow...
			return;

		lastSelectedUpdate = millis();

		changeSelected = 1;
	}

	updateEncoder = 1;
}

void toggleRunning()
{
	unsigned long diff = (millis() - lastToggleRunning);

	if(diff < 300) // careful about the overflow...
		return;

	lastToggleRunning = millis();

	running = !running;

	if(running)
	{
		writeLED(200, -1, -1);
	}
	else
	{
		writeLED(0,-1,-1);
		if(currentMode == TRIGGER)
			writeLED(200, -1, -1);
	}

	updateEncoder = 1;
}

void drawTimecode(int secs, int rep)
{
	if(rep == 0)
	{
		lcdNumberWrite(secs);
		lcdDataWrite('"');
	}
	else
	{
		lcdNumberWrite(secs/60);
		lcdDataWrite('\'');
	}
}

int timecodeLength(int secs, int rep)
{
	int count = 2;

	if(rep)
		secs = secs/60;

	int n1 = secs/100;
	int n2 = (secs - n1 * 100) / 10;

	if(n2)
		count++;

	return count;
}

char *modeHeader[4] = {"Interval", "            Bulb", "Interval    Bulb", "Trigger"};

int fadeUp = 1;

void updateContrast()
{
	write_pot(contrastPot, map(contrastValue, 0, 1000, 25, 60));
}

void writeLED(int r, int g, int b)
{
	if(r > -1)
		write_pot(ledRPot, map(255-r, 0, 255, 15, 150));
	if(g > -1)
		write_pot(ledGPot, map(255-g, 0, 255, 15, 150));
	if(b > -1)
		write_pot(ledBPot, map(255-b, 0, 255, 15, 150));
}

void loop(void)
{
	if(fadeUp)
	{
		updateContrast();

		if((contrastValue -= 1) <= 0)
			fadeUp = 0;
	}

	if(updateHeader)
	{
		lcdHome(0);

		int count;
		for (count = 0; modeHeader[currentMode][count] != 0; count++)
			lcdDataWrite(modeHeader[currentMode][count]);
		for (; count < 16; count++)
			lcdDataWrite(' ');

		updateHeader = 0;
		updateEncoder = 1;
	}

	if(updateEncoder)
	{
		lcdHome(1);

		if(currentMode != BULB)
			drawTimecode(lapseTime, lapseRepresentation);

		int width = 0;

		if(currentMode != BULB)
			width += timecodeLength(lapseTime, lapseRepresentation);
		if(currentMode != INTERVAL && currentMode != TRIGGER)
			width += timecodeLength(exposureTime, exposureRepresentation);

		if(selected == 0)
		{
			lcdDataWrite(' '); width++;
			lcdDataWrite(127); width++;
		}
		else
		{
			width += 2;
		}

		for(int i = 16; i > width; i--)
			lcdDataWrite(' ');

		if(selected == 1)
		{
			lcdDataWrite(126);
			lcdDataWrite(' ');
		}

		if(currentMode != INTERVAL && currentMode != TRIGGER)
			drawTimecode(exposureTime, exposureRepresentation);

		updateEncoder = 0;
	}

	if(!digitalRead(buttonA))
		toggleRunning();

	if(running)
	{
		digitalWrite(lcdPower, LOW);

		if(currentMode == TRIGGER)
		{
			if(analogRead(triggerInput) < 100) // 100 might change with different resistors, make sure it works!
			{
				writeLED(-1, 128, -1);
				digitalWrite(triggerReset, HIGH);
				delay(100); // this should probably be at least the time of the delay from signal (in the 555)...
				digitalWrite(triggerReset, LOW);
				delay(10);
				digitalWrite(triggerReset, HIGH);
				writeLED(-1, 0, -1);
			}

			return;
		}

		unsigned long diff = millis() - lastShutter;

		int adjustedLapseTime = lapseTime;
		if(currentMode != INTERVAL)
			adjustedLapseTime -= exposureTime;

		if(diff > (adjustedLapseTime * 1000)) // careful about the overflow...
		{
			writeLED(-1, 128, -1);
			digitalWrite(cameraShutter, LOW);

			if(currentMode == INTERVAL)
				delay(100);
			else
				delay(exposureTime * 1000); //delay for length of exposure
				// biggest problem with this is that you can't stop a bulb (of either type)
				// in the middle... you have to power off the intervalometer; same as you would have
				// to do with a camera, I suppose, so people might be used to it.
				// however, we can get around this by looping and checking millis()...

			digitalWrite(cameraShutter, HIGH);
			lastShutter = millis();

			if(currentMode == BULB)
				running = 0;

			writeLED(-1, 0, -1);
		}
	}
	else
	{
		digitalWrite(lcdPower, HIGH);

		if(!digitalRead(buttonB))
			switchModes();

		if(!digitalRead(encoderButton))
			switchSelected();
		else if(changeSelected)
		{
			selected = !selected;
			changeSelected = 0;
			updateEncoder = 1;
		}
	}
}
