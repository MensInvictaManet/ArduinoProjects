/*
8x8 LED Matrix display code.
Built to work with the LE-MM103 8x8 BiColour LED Matrix (one colour only)
Markings on the Unit are:  GYXM-1388ASRG2 EI4 943 4

Created 23 Dec 2009
by Dustin Kerr

http://www.arduino.cc/en/Tutorial/RowColumnScanning

see also http://www.tigoe.net/pcomp/code/category/arduinowiring/514 for more
*/

//  Controller should only change the arduinoPins depending on where we plug in to the Arduino
const int arduinoPins[24] = { 22, 23, 24, 25, 26, 27, 28, 29, 34, 35, 36, 37, 38, 39, 40, 41, 46, 47, 48, 49, 50, 51, 52, 53 };
const int rowPinIndex[8] = { 8, 13, 7, 11, 0, 6, 1, 4 };
const int colPinIndex[8] = { 12, 2, 3, 9, 5, 10, 14, 15 };

float timeCount = 0;

enum CharacterIndex
{
	CHAR_H			= 0,
	CHAR_E			= 1,
	CHAR_L			= 2,
	CHAR_O			= 3,
	CHAR_SMILE		= 4,
	CHAR_SPACE		= 5,
	CHARACTER_COUNT
};

int characters[CHARACTER_COUNT][8][8] = 
{
	{	// H
		{0,0,1,0,0,0,1,0},
		{0,0,1,0,0,0,1,0},
		{0,0,1,0,0,0,1,0},
		{0,0,1,1,1,1,1,0},
		{0,0,1,1,1,1,1,0},
		{0,0,1,0,0,0,1,0},
		{0,0,1,0,0,0,1,0},
		{0,0,1,0,0,0,1,0}
	},
	{	// E
		{0,0,1,1,1,1,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,1,1,1,0},
		{0,0,0,0,1,1,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,1,1,1,1,1,0}
	},
	{	// L
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,1,0},
		{0,0,1,1,1,1,1,0}
	},
	{	// O
		{0,0,0,1,1,1,0,0},
		{0,0,1,0,0,0,1,0},
		{0,0,1,0,0,0,1,0},
		{0,0,1,0,0,0,1,0},
		{0,0,1,0,0,0,1,0},
		{0,0,1,0,0,0,1,0},
		{0,0,1,0,0,0,1,0},
		{0,0,0,1,1,1,0,0}
	},
	{	// SMILEY
		{0,0,0,0,0,0,0,0},
		{0,1,1,0,0,1,1,0},
		{0,1,1,0,0,1,1,0},
		{0,1,1,0,0,1,1,0},
		{0,0,0,0,0,0,0,0},
		{0,1,0,0,0,0,1,0},
		{0,0,1,0,0,1,0,0},
		{0,0,0,1,1,0,0,0}
	},
	{	// SPACE
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0}
	}
};

const int animationFrames = 12;
const CharacterIndex animation[animationFrames] = { CHAR_H, CHAR_E, CHAR_L, CHAR_L, CHAR_O, CHAR_SPACE, CHAR_SMILE, CHAR_SMILE, CHAR_SMILE, CHAR_SPACE, CHAR_SPACE, CHAR_SPACE };
int currentFrame = 0;

void setup()
{
  	Serial.begin(9600);
  
	//  Make sure all pins are completely off, set to INPUT
	for (int i = 0; i < 8; i++)
	{
		pinMode(arduinoPins[rowPinIndex[i]], INPUT);
		pinMode(arduinoPins[colPinIndex[i]], INPUT);
	}
	
}

void loop()
{
	static unsigned long timeStart = millis();
	static unsigned long lastIndex = 999;
	const unsigned long timeCurrent = millis();
	const unsigned long timeDelta = timeCurrent - timeStart;
	const unsigned long frameIndex = (timeDelta % (animationFrames * 1000)) / 1000;
	
	Serial.print(frameIndex);
	Serial.print("\n");
	
	drawScreen(characters[animation[frameIndex]]);
	lastIndex = frameIndex;
}

void clearScreen()
{
	for (int i = 0; i < 8; ++i)
	{
		digitalWrite(arduinoPins[rowPinIndex[i]], LOW);
		digitalWrite(arduinoPins[colPinIndex[i]], LOW);
	}
}

void drawScreen(int character[8][8])
{
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			// draw some letter bits
			if(character[i][7-j] == 1)
			{
			    pinMode(arduinoPins[rowPinIndex[i]], OUTPUT);
			    pinMode(arduinoPins[colPinIndex[j]], OUTPUT);
				digitalWrite(arduinoPins[rowPinIndex[i]], HIGH);
				digitalWrite(arduinoPins[colPinIndex[j]], LOW);
      		}
      		
      		pinMode(arduinoPins[rowPinIndex[i]], INPUT);
			pinMode(arduinoPins[colPinIndex[j]], INPUT);
    	}
	}
}