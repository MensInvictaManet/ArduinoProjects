/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */
 
const int animFrameCount = 3;
const bool heartAnim[animFrameCount][8][8] = 
{
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 1, 1, 0, 0, 0 },
		{ 0, 0, 0, 1, 1, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 1, 1, 0, 0, 1, 1, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 1, 1, 1, 1, 1, 1, 0 },
		{ 0, 0, 1, 1, 1, 1, 0, 0 },
		{ 0, 0, 0, 1, 1, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 }
	}
};

unsigned long animFrameMillis = 500;
unsigned long animFrameMillisTotal = animFrameMillis * animFrameCount;


//const char pinRow[8] = { 8, 13, 7, 11, 0, 6, 1, 4 };
//const char pinCol[8] = { 12, 2, 3, 9, 5, 10, 14, 15 };

const char pinRow[8] = { 2, 3, 4, 5, 6, 7, 8, 9 };
const char pinCol[8] = { 10, 11, 12, 13, A0, A1, A2, A3 };

void ClearScreen()
{
	for (int i = 0; i < 8; ++i) { digitalWrite(pinRow[i], LOW); pinMode(pinRow[i], INPUT); }
	for (int i = 0; i < 8; ++i) { digitalWrite(pinCol[i], LOW); pinMode(pinCol[i], INPUT); }
}

void DrawAnimFrame(int index)
{
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			if (heartAnim[index][j][i] == 1)
			{
				pinMode(pinRow[i], OUTPUT);
				digitalWrite(pinRow[i], HIGH);
				
				pinMode(pinCol[j], OUTPUT);
				digitalWrite(pinCol[j], LOW);
				
				digitalWrite(pinRow[i], LOW);
				digitalWrite(pinCol[j], LOW);
				pinMode(pinRow[i], INPUT);
				pinMode(pinCol[j], INPUT);
			}
		}
	}
}

// the setup function runs once when you press reset or power the board
void setup()
{
	ClearScreen();
}

// the loop function runs over and over again forever
void loop()
{
	DrawAnimFrame((millis() % animFrameMillisTotal) / animFrameMillis);
}

