#include <Time.h>

/* X Columns

	15		11		7		3
	
	14		10		6		2
	
	13		9		5		1
	
	12		8		4		0
*/

/* Y Columns

	15		11		7		3
	
	14		10		6		2
	
	13		9		5		1
	
	12		8		4		0
*/

/* Z Columns

	15		11		7		3
	
	14		10		6		2
	
	13		9		5		1
	
	12		8		4		0
*/

/* Levels
	3
	
	2
	
	1
	
	0
*/

const int frameDelay = 120;
const int columnPins[16] = { 28, 29, 26, 27, 32, 33, 30, 31, 36, 37, 34, 35, 41, 40, 39, 38 };
const int levelPins[4] = { 24, 25, 22, 23 };

long randomNumber = 0;
const int RenderFlagSize = (sizeof(int) * 8);
const int RenderFlagCount = (64 / RenderFlagSize);
int RenderFlag[RenderFlagCount] = { 0, 0, 0, 0 };

void setup()
{
	for (int i = 0; i < 16; ++i)
	{
		pinMode(columnPins[i], OUTPUT);	
		digitalWrite(columnPins[i], LOW);	
	}
	
	for (int i = 0; i < 4; ++i)
	{
		pinMode(levelPins[i], OUTPUT);	
		digitalWrite(levelPins[i], HIGH);	
	}
	
	randomSeed(analogRead(0));
	
	Serial.begin(9600);
	Serial.println("Program Start!");
}

void DebugOutputRenderFlags()
{
	Serial.print("Render Flags: ");
	for (int i = 0; i < RenderFlagCount; ++i)
	{
		Serial.print("{ ");
		for (int j = 0; j < RenderFlagSize; ++j)
		{
			Serial.print(((RenderFlag[i] & (1 << j)) != 0) ? "1" : "0");
			if (j < RenderFlagSize - 1) Serial.print(", ");
		}
		Serial.print(" }");
		if (i < RenderFlagCount - 1) Serial.print(", ");
	}
	Serial.println();
}

void WaitAndRender(int delayMillis = frameDelay)
{
	for (unsigned long time = millis(); time + delayMillis > millis(); RenderDisplay()) {}
}

bool GetBit(int index)
{
	return ((RenderFlag[index / RenderFlagSize] & (1 << (index % RenderFlagSize))) != 0);
}

void SetBit(int index, bool value)
{
	if (value) 	RenderFlag[index / RenderFlagSize] |= (1 << (index % RenderFlagSize));
	else 		RenderFlag[index / RenderFlagSize] &= ~(1 << (index % RenderFlagSize));
}

void SetXColumn(int index, bool value)
{
	SetBit(index +  0, value);
	SetBit(index + 16, value);
	SetBit(index + 32, value);
	SetBit(index + 48, value);
}

void SetYColumn(int index, bool value)
{
	SetBit(index * 4 + 0, value);
	SetBit(index * 4 + 1, value);
	SetBit(index * 4 + 2, value);
	SetBit(index * 4 + 3, value);
}

void SetZColumn(int index, bool value)
{
	SetBit((index / 4) * 16 + (index % 4) +  0, value);
	SetBit((index / 4) * 16 + (index % 4) +  4, value);
	SetBit((index / 4) * 16 + (index % 4) +  8, value);
	SetBit((index / 4) * 16 + (index % 4) + 12, value);
}

void ClearMap()
{
	memset(RenderFlag, 0, 8);
}

void RunLightTest()
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			digitalWrite(columnPins[i], HIGH);
			digitalWrite(levelPins[j], LOW);
			delay(250);
			digitalWrite(columnPins[i], LOW);
			digitalWrite(levelPins[j], HIGH);
		}
	}
}

void RenderDisplay()
{
	//  Note: For each column, we throw on the power and then turn on power for each of the lights in that level that are on, so we only need to delay 1 for each 4 lights... fixes the flashing issue
	for (int i = 0; i < 16; ++i)
	{
		digitalWrite(columnPins[i], HIGH);
		
		for (int j = 0; j < 4; ++j)
			if (GetBit(i * 4 + j))
				digitalWrite(levelPins[j], LOW);
		
		delay(1);
		for (int j = 0; j < 4; ++j) digitalWrite(levelPins[j], HIGH);
		digitalWrite(columnPins[i], LOW);
	}
}

void Image_XAxis(int level)
{
	if (level >= 4) return;
	
	ClearMap();
	SetYColumn( 0 + level, true);
	SetYColumn( 4 + level, true);
	SetYColumn( 8 + level, true);
	SetYColumn(12 + level, true);
}

void Image_YAxis(int level)
{
	if (level >= 4) return;
	
	ClearMap();
	SetZColumn( 0 + level, true);
	SetZColumn( 4 + level, true);
	SetZColumn( 8 + level, true);
	SetZColumn(12 + level, true);
}

void Image_ZAxis(int level)
{
	if (level >= 4) return;
	
	ClearMap();
	SetZColumn(0 + (level * 4), true);
	SetZColumn(1 + (level * 4), true);
	SetZColumn(2 + (level * 4), true);
	SetZColumn(3 + (level * 4), true);
}

void Image_XAxisToYAxis1()
{
	ClearMap();
	SetXColumn(0, true);
	SetXColumn(1, true);
	SetXColumn(6, true);
	SetXColumn(7, true);
}

void Image_XAxisToYAxis2()
{
	ClearMap();
	SetXColumn(0, true);
	SetXColumn(5, true);
	SetXColumn(10, true);
	SetXColumn(11, true);
}

void Image_XAxisToYAxis3()
{
	ClearMap();
	SetXColumn(0, true);
	SetXColumn(5, true);
	SetXColumn(10, true);
	SetXColumn(15, true);
}

void Image_XAxisToYAxis4()
{
	ClearMap();
	SetXColumn(0, true);
	SetXColumn(5, true);
	SetXColumn(10, true);
	SetXColumn(14, true);
}

void Image_XAxisToYAxis5()
{
	ClearMap();
	SetXColumn(0, true);
	SetXColumn(4, true);
	SetXColumn(9, true);
	SetXColumn(13, true);
}

void Image_XAxisToZAxis1()
{
	ClearMap();
	SetYColumn(0, true);
	SetYColumn(4, true);
	SetYColumn(9, true);
	SetYColumn(13, true);
}

void Image_XAxisToZAxis2()
{
	ClearMap();
	SetYColumn(0, true);
	SetYColumn(5, true);
	SetYColumn(10, true);
	SetYColumn(14, true);
}

void Image_XAxisToZAxis3()
{
	ClearMap();
	SetYColumn(0, true);
	SetYColumn(5, true);
	SetYColumn(10, true);
	SetYColumn(15, true);
}

void Image_XAxisToZAxis4()
{
	ClearMap();
	SetYColumn(0, true);
	SetYColumn(5, true);
	SetYColumn(10, true);
	SetYColumn(11, true);
}

void Image_XAxisToZAxis5()
{
	ClearMap();
	SetYColumn(0, true);
	SetYColumn(1, true);
	SetYColumn(6, true);
	SetYColumn(7, true);
}

void Image_ReactorCore1()
{
	ClearMap();
	SetXColumn(3, true);
	SetXColumn(15, true);
	SetZColumn(3, true);
	SetZColumn(15, true);
}

void Image_ReactorCore2()
{
	ClearMap();
	SetXColumn(2, true);
	SetXColumn(14, true);
	SetZColumn(2, true);
	SetZColumn(14, true);
}

void Image_ReactorCore3()
{
	ClearMap();
	SetXColumn(1, true);
	SetXColumn(13, true);
	SetZColumn(1, true);
	SetZColumn(13, true);
}

void Image_ReactorCore4()
{
	ClearMap();
	SetXColumn(0, true);
	SetXColumn(12, true);
	SetZColumn(0, true);
	SetZColumn(12, true);
}

void Image_ReactorCore5()
{
	ClearMap();
	SetBit(20, true);
	SetBit(24, true);
	SetBit(36, true);
	SetBit(40, true);
}

void Image_ReactorCore6()
{
	ClearMap();
	SetBit(21, true);
	SetBit(25, true);
	SetBit(37, true);
	SetBit(41, true);
}

void Image_ReactorCore7()
{
	ClearMap();
	SetBit(22, true);
	SetBit(26, true);
	SetBit(38, true);
	SetBit(42, true);
}

void Image_ReactorCore8()
{
	ClearMap();
	SetBit(23, true);
	SetBit(27, true);
	SetBit(39, true);
	SetBit(43, true);
}

void Pattern_XAxis()
{
	Image_XAxis(0);
	WaitAndRender();
	Image_XAxis(1);
	WaitAndRender();
	Image_XAxis(2);
	WaitAndRender();
	Image_XAxis(3);
	WaitAndRender();
	Image_XAxis(2);
	WaitAndRender();
	Image_XAxis(1);
	WaitAndRender();
}

void Pattern_YAxis()
{
	Image_YAxis(0);
	WaitAndRender();
	Image_YAxis(1);
	WaitAndRender();
	Image_YAxis(2);
	WaitAndRender();
	Image_YAxis(3);
	WaitAndRender();
	Image_YAxis(2);
	WaitAndRender();
	Image_YAxis(1);
	WaitAndRender();
}

void Pattern_ZAxis()
{
	Image_ZAxis(0);
	WaitAndRender();
	Image_ZAxis(1);
	WaitAndRender();
	Image_ZAxis(2);
	WaitAndRender();
	Image_ZAxis(3);
	WaitAndRender();
	Image_ZAxis(2);
	WaitAndRender();
	Image_ZAxis(1);
	WaitAndRender();
}

void Pattern_XAxisToYAxis()
{
	Image_XAxis(0);
	WaitAndRender();
	Image_XAxisToYAxis1();
	WaitAndRender();
	Image_XAxisToYAxis2();
	WaitAndRender();
	Image_XAxisToYAxis3();
	WaitAndRender();
	Image_XAxisToYAxis4();
	WaitAndRender();
	Image_XAxisToYAxis5();
	WaitAndRender();
}

void Pattern_YAxisToXAxis()
{
	Image_YAxis(0);
	WaitAndRender();
	Image_XAxisToYAxis5();
	WaitAndRender();
	Image_XAxisToYAxis4();
	WaitAndRender();
	Image_XAxisToYAxis3();
	WaitAndRender();
	Image_XAxisToYAxis2();
	WaitAndRender();
	Image_XAxisToYAxis1();
	WaitAndRender();
}

void Pattern_XAxisToZAxis()
{
	Image_XAxis(0);
	WaitAndRender();
	Image_XAxisToZAxis1();
	WaitAndRender();
	Image_XAxisToZAxis2();
	WaitAndRender();
	Image_XAxisToZAxis3();
	WaitAndRender();
	Image_XAxisToZAxis4();
	WaitAndRender();
	Image_XAxisToZAxis5();
	WaitAndRender();
}

void Pattern_ZAxisToXAxis()
{
	Image_ZAxis(0);
	WaitAndRender();
	Image_XAxisToZAxis5();
	WaitAndRender();
	Image_XAxisToZAxis4();
	WaitAndRender();
	Image_XAxisToZAxis3();
	WaitAndRender();
	Image_XAxisToZAxis2();
	WaitAndRender();
	Image_XAxisToZAxis1();
	WaitAndRender();
}

void Pattern_Rainfall()
{
	unsigned long time = millis();
	
	static const int raindropCount = 8;
	int Raindrops[raindropCount];
	for (int i = 0; i < raindropCount; ++i) Raindrops[i] = (random(16) * 4) + random(4);
	
	for (int i = 0; i < 20000 / frameDelay; ++i)
	{
		//  Set the current pattern
		ClearMap();
		for (int j = 0; j < raindropCount; ++j) SetBit(Raindrops[j], true);
		
		//  Render the current pattern
		WaitAndRender();
		
		//  Update the current pattern
		for (int j = 0; j < raindropCount; ++j)
		{
			if (Raindrops[j] % 4 == 0) Raindrops[j] = (random(16) * 4) + 2 + random(2);
			else Raindrops[j]--;
		}
	}
}

void Pattern_RandomYColumns()
{
	unsigned long time = millis();
	
	static const int columnCount = 1;
	int Columns[columnCount];
	for (int i = 0; i < columnCount; ++i) Columns[i] = random(16);
	
	for (int i = 0; i < 20000 / frameDelay; ++i)
	{
		//  Set the current pattern
		ClearMap();
		for (int j = 0; j < columnCount; ++j) SetYColumn(Columns[j], true);
		
		//  Render the current pattern
		WaitAndRender();
		
		//  Update the current pattern
		for (int j = 0; j < columnCount; ++j) Columns[j] = random(16);
	}
}

void Pattern_TwisterHalf()
{
	for (int i = 0; i < 3; ++i)
	{
		ClearMap();		SetYColumn(1, true);	SetYColumn(5, true);
		WaitAndRender();
		ClearMap();		SetYColumn(2, true);	SetYColumn(6, true);
		WaitAndRender();
		ClearMap();		SetYColumn(3, true);	SetYColumn(6, true);
		WaitAndRender();
		ClearMap();		SetYColumn(7, true);	SetYColumn(6, true);
		WaitAndRender();
		ClearMap();		SetYColumn(11, true);	SetYColumn(10, true);
		WaitAndRender();
		ClearMap();		SetYColumn(15, true);	SetYColumn(10, true);
		WaitAndRender();
		ClearMap();		SetYColumn(14, true);	SetYColumn(10, true);
		WaitAndRender();
		ClearMap();		SetYColumn(13, true);	SetYColumn(9, true);
		WaitAndRender();
		ClearMap();		SetYColumn(12, true);	SetYColumn(9, true);
		WaitAndRender();
		ClearMap();		SetYColumn(8, true);	SetYColumn(9, true);
		WaitAndRender();
		ClearMap();		SetYColumn(4, true);	SetYColumn(5, true);
		WaitAndRender();
		ClearMap();		SetYColumn(0, true);	SetYColumn(5, true);
		WaitAndRender();
	}
}

void Pattern_TwisterFull()
{
	for (int i = 0; i < 3; ++i)
	{
		ClearMap();		SetYColumn(1, true);	SetYColumn(5, true);		SetYColumn(10, true);	SetYColumn(14, true);
		WaitAndRender();
		ClearMap();		SetYColumn(2, true);	SetYColumn(6, true);		SetYColumn(13, true);	SetYColumn(9, true);
		WaitAndRender();
		ClearMap();		SetYColumn(3, true);	SetYColumn(6, true);		SetYColumn(9, true);	SetYColumn(12, true);
		WaitAndRender();
		ClearMap();		SetYColumn(7, true);	SetYColumn(6, true);		SetYColumn(9, true);	SetYColumn(8, true);
		WaitAndRender();
		ClearMap();		SetYColumn(11, true);	SetYColumn(10, true);		SetYColumn(5, true);	SetYColumn(4, true);
		WaitAndRender();
		ClearMap();		SetYColumn(15, true);	SetYColumn(10, true);		SetYColumn(5, true);	SetYColumn(0, true);
		WaitAndRender();
		ClearMap();		SetYColumn(14, true);	SetYColumn(10, true);		SetYColumn(5, true);	SetYColumn(1, true);
		WaitAndRender();
		ClearMap();		SetYColumn(13, true);	SetYColumn(9, true);		SetYColumn(6, true);	SetYColumn(2, true);
		WaitAndRender();
		ClearMap();		SetYColumn(12, true);	SetYColumn(9, true);		SetYColumn(6, true);	SetYColumn(3, true);
		WaitAndRender();
		ClearMap();		SetYColumn(8, true);	SetYColumn(9, true);		SetYColumn(6, true);	SetYColumn(7, true);
		WaitAndRender();
		ClearMap();		SetYColumn(4, true);	SetYColumn(5, true);		SetYColumn(10, true);	SetYColumn(11, true);
		WaitAndRender();
		ClearMap();		SetYColumn(0, true);	SetYColumn(5, true);		SetYColumn(10, true);	SetYColumn(15, true);
		WaitAndRender();
	}
}

void Pattern_LightUpOneByOne()
{
	int lightList[64];
	
	for (int i = 0; i < 64; ++i) lightList[i] = i;
	for (int i = 0; i < 64; ++i)
	{
		int otherIndex = random(64);
		if (otherIndex == i) continue;
		lightList[i] ^= lightList[otherIndex];
		lightList[otherIndex] ^= lightList[i];
		lightList[i] ^= lightList[otherIndex];
	}
	
	ClearMap();
	WaitAndRender(25);
	for (int i = 0; i < 64; ++i)
	{
		SetBit(lightList[i], true);
		WaitAndRender(50);
	}
	
	for (int i = 0; i < 64; ++i)
	{
		SetBit(lightList[i], false);
		WaitAndRender(50);
	}
}

void Pattern_Elevators()
{
	unsigned long time = millis();
	
	int Elevators[16];
	for (int i = 0; i < 16; ++i) Elevators[i] = (random(2) == 0) ? 0 : 3;
	
	const int patternTime = 15000;
	const int actionTime = 35;
	
	for (int i = 0; i < (patternTime + actionTime * 3) / frameDelay; ++i)
	{
		//  Set the current pattern
		ClearMap();
		for (int j = 0; j < 16; ++j) SetBit(4 * j + Elevators[j], true);
		
		//  Render the current pattern
		WaitAndRender();
		
		//  Choose an elevator and shift it
		int shiftingElevator = random(16);
		bool up = GetBit(4 * shiftingElevator);
		for (int j = 0; j < 3; ++j)
		{
			SetBit(up ? (4 * shiftingElevator + j) : (4 * shiftingElevator + 3 - j), false);
			SetBit(up ? (4 * shiftingElevator + j + 1) : (4 * shiftingElevator + 3 - j - 1), true);
			WaitAndRender(actionTime);
		}
		Elevators[shiftingElevator] = (Elevators[shiftingElevator] == 0) ? 3 : 0;
	}
}

void PatternReactorCore()
{
	Image_ReactorCore1();
	WaitAndRender(100);
	Image_ReactorCore2();
	WaitAndRender(100);
	Image_ReactorCore3();
	WaitAndRender(100);
	Image_ReactorCore4();
	WaitAndRender(100);
	Image_ReactorCore5();
	WaitAndRender(100);
	Image_ReactorCore6();
	WaitAndRender(100);
	Image_ReactorCore7();
	WaitAndRender(100);
	Image_ReactorCore8();
	WaitAndRender(100);
	Image_ReactorCore7();
	WaitAndRender(100);
	Image_ReactorCore6();
	WaitAndRender(100);
	Image_ReactorCore5();
	WaitAndRender(100);
	Image_ReactorCore4();
	WaitAndRender(100);
	Image_ReactorCore3();
	WaitAndRender(100);
	Image_ReactorCore2();
	WaitAndRender(100);
	Image_ReactorCore1();
	WaitAndRender(100);
}

void loop()
{
	PatternReactorCore();
	PatternReactorCore();
	PatternReactorCore();
	Pattern_TwisterHalf();
	Pattern_TwisterFull();
	Pattern_Elevators();
	Pattern_LightUpOneByOne();
	Pattern_RandomYColumns();
	Pattern_Rainfall();
	Pattern_XAxis();
	Pattern_XAxisToYAxis();
	Pattern_YAxis();
	Pattern_YAxisToXAxis();
	Pattern_XAxis();
	Pattern_XAxisToZAxis();
	Pattern_ZAxis();
	Pattern_ZAxisToXAxis();
}
