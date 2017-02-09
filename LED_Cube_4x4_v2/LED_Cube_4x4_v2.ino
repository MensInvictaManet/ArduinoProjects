#include <Time.h>

/* X Columns

  15    11    7   3
  
  14    10    6   2
  
  13    9   5   1
  
  12    8   4   0
*/

/* Y Columns

  15    11    7   3
  
  14    10    6   2
  
  13    9   5   1
  
  12    8   4   0
*/

/* Z Columns

  15    11    7   3
  
  14    10    6   2
  
  13    9   5   1
  
  12    8   4   0
*/

/* Levels
  3
  
  2
  
  1
  
  0
*/

#define PATTERN_SWITCH_BUTTON   44
#define COLOR_SWITCH_BUTTON     45
#define BUTTON_DELAY            250

unsigned long buttonTimer = 0;

const int frameDelay = 120;
const int columnPinsR[16] = { A4, A5, A6, A7, A8, 48, A10, A11, A12, A13, A14, A15, 22, 23, 24, 25 };
const int columnPinsG[16] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2, A3 };
const int columnPinsB[16] = { 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41 };
const int levelPins[4] = { 52, 51, 50, 53 };

enum Colors
{
  COLOR_BLACK   = 0,
  COLOR_RED     = 1,
  COLOR_GREEN   = 2,
  COLOR_YELLOW  = 3,
  COLOR_BLUE    = 4,
  COLOR_MAGENTA = 5,
  COLOR_AQUA    = 6,
  COLOR_WHITE   = 7
};
char CurrentColor = COLOR_RED;

int CurrentPattern = 10;

const int bitsPerByte = 8;
const int pixelCount = 64;
const int bitCountPerPixel = 4;
const int dataPerInt = (sizeof(int) * bitsPerByte) / bitCountPerPixel;
const unsigned int RenderDataSize = pixelCount * bitCountPerPixel; // 256
const unsigned int RenderIntCount = (RenderDataSize / (sizeof(int) * bitsPerByte)); // 16
unsigned int RenderData[RenderIntCount];

long patternStartTime = 0;
long currentPatternTime = 0;

//  Pattern_Elevators
int Elevators[16];

// Pattern_LightUpOneByOne
int LightList[pixelCount];

//  Pattern_Rainfall
const int RaindropCount = 8;
int Raindrops[RaindropCount];

void setup()
{
  //  Clear the render data to black
  for (int i = 0; i < RenderIntCount; ++i) RenderData[i] = 0;
  
  for (int i = 0; i < 16; ++i)
  {
    pinMode(columnPinsR[i], INPUT); 
    pinMode(columnPinsG[i], INPUT); 
    digitalWrite(columnPinsR[i], LOW); 
    digitalWrite(columnPinsG[i], LOW); 
  }
  
  for (int i = 0; i < 4; ++i)
  {
    pinMode(levelPins[i], OUTPUT);  
    digitalWrite(levelPins[i], LOW);
  }

  //  Set the button pins to INPUT_PULLUP and reset the button timer
  pinMode(PATTERN_SWITCH_BUTTON, INPUT_PULLUP); // connect internal pull-up
  pinMode(COLOR_SWITCH_BUTTON, INPUT_PULLUP); // connect internal pull-up
  
  randomSeed(analogRead(0));

  //  Pattern-specific prep calls
  Pattern_Elevators_Prep();
  Pattern_LightUpOneByOne_Prep();
  Pattern_Rainfall_Prep();
  
  Serial.begin(9600);
  Serial.println("Program Start!");
}

void SetCurrentPatternTime(long patternLength)
{
  long currentTime = millis();
  currentPatternTime = currentTime - patternStartTime;
  while (currentPatternTime > patternLength)
  {
    Serial.println("Finished pattern!");
    patternStartTime = currentTime;
    currentPatternTime -= patternLength;
  }
}

void DebugOutputRenderFlags()
{
  Serial.print("Render Flags: ");
  for (int i = 0; i < RenderIntCount; ++i)
  {
    Serial.print("{ ");
    for (int j = 0; j < dataPerInt; ++j)
    {
      char pixelData = ((RenderData[i] >> (j * 4)) & 7);
      Serial.print(pixelData);
      if (j < dataPerInt - 1) Serial.print(", ");
    }
    Serial.print(" }");
  }
  Serial.println();
}

void WaitAndRender(int delayMillis = frameDelay)
{
  for (unsigned long time = millis(); time + delayMillis > millis(); RenderDisplay()) {}
}

char GetBit(int index)
{
  return ((RenderData[index / dataPerInt] >> ((index % dataPerInt) * bitCountPerPixel)) & 15);
}

void SetBit(int index, char value)
{
  int bitData = (value << ((index % dataPerInt) * bitCountPerPixel));
  RenderData[index / dataPerInt] &= ~(15 << ((index % dataPerInt) * bitCountPerPixel));
  RenderData[index / dataPerInt] |= bitData;
}

void SetXColumn(int index, bool value)
{
  SetBit(index +  0, value ? CurrentColor : COLOR_BLACK);
  SetBit(index + 16, value ? CurrentColor : COLOR_BLACK);
  SetBit(index + 32, value ? CurrentColor : COLOR_BLACK);
  SetBit(index + 48, value ? CurrentColor : COLOR_BLACK);
}

void SetYColumn(int index, bool value)
{
  SetBit(index * 4 + 0, value ? CurrentColor : COLOR_BLACK);
  SetBit(index * 4 + 1, value ? CurrentColor : COLOR_BLACK);
  SetBit(index * 4 + 2, value ? CurrentColor : COLOR_BLACK);
  SetBit(index * 4 + 3, value ? CurrentColor : COLOR_BLACK);
}

void SetZColumn(int index, bool value)
{
  SetBit((index / 4) * 16 + (index % 4) +  0, value ? CurrentColor : COLOR_BLACK);
  SetBit((index / 4) * 16 + (index % 4) +  4, value ? CurrentColor : COLOR_BLACK);
  SetBit((index / 4) * 16 + (index % 4) +  8, value ? CurrentColor : COLOR_BLACK);
  SetBit((index / 4) * 16 + (index % 4) + 12, value ? CurrentColor : COLOR_BLACK);
}

void ClearMap()
{
  memset(RenderData, 0, RenderIntCount * (sizeof(int)));
}

void RunLightTest()
{
  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      digitalWrite(columnPinsR[i], HIGH);
      digitalWrite(columnPinsG[i], HIGH);
      digitalWrite(levelPins[j], LOW);
      delay(250);
      digitalWrite(columnPinsR[i], LOW);
      digitalWrite(columnPinsG[i], LOW);
      digitalWrite(levelPins[j], HIGH);
    }
  }
}

void RenderDisplay()
{
  //  Note: For each column, we throw on the power and then turn on power for each of the lights in that level that are on, so we only need to delay 1 for each 4 lights... fixes the flashing issue
  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      char color = GetBit(i * 4 + j);
      if (color == COLOR_BLACK) continue;

      //  Turn the column pins to LOW to enable the grounding
      pinMode(columnPinsR[i], (color & 1) ? OUTPUT : INPUT);
      pinMode(columnPinsG[i], (color & 2) ? OUTPUT : INPUT);
      pinMode(columnPinsB[i], (color & 4) ? OUTPUT : INPUT);
      digitalWrite(columnPinsR[i], (color & 1) ? LOW : HIGH);
      digitalWrite(columnPinsG[i], (color & 2) ? LOW : HIGH);
      digitalWrite(columnPinsB[i], (color & 4) ? LOW : HIGH);

      //  Set the power output to ON if the bit is set
      digitalWrite(levelPins[j], HIGH);

      //  Render this for a single millisecond before moving on
      delay(1);

      //  Reset all values to default
      digitalWrite(levelPins[j], LOW);
      digitalWrite(columnPinsR[i], HIGH);
      digitalWrite(columnPinsG[i], HIGH);
      digitalWrite(columnPinsB[i], HIGH);
      pinMode(columnPinsR[i], INPUT);
      pinMode(columnPinsG[i], INPUT);
      pinMode(columnPinsB[i], INPUT);
    }
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
  SetBit(20, CurrentColor);
  SetBit(24, CurrentColor);
  SetBit(36, CurrentColor);
  SetBit(40, CurrentColor);
}

void Image_ReactorCore6()
{
  ClearMap();
  SetBit(21, CurrentColor);
  SetBit(25, CurrentColor);
  SetBit(37, CurrentColor);
  SetBit(41, CurrentColor);
}

void Image_ReactorCore7()
{
  ClearMap();
  SetBit(22, CurrentColor);
  SetBit(26, CurrentColor);
  SetBit(38, CurrentColor);
  SetBit(42, CurrentColor);
}

void Image_ReactorCore8()
{
  ClearMap();
  SetBit(23, CurrentColor);
  SetBit(27, CurrentColor);
  SetBit(39, CurrentColor);
  SetBit(43, CurrentColor);
}

void PatternReactorCore()
{
  const long patternFrameTime = 100;
  SetCurrentPatternTime(patternFrameTime * 15);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   Image_ReactorCore1(); break;
    case 1:   Image_ReactorCore2(); break;
    case 2:   Image_ReactorCore3(); break;
    case 3:   Image_ReactorCore4(); break;
    case 4:   Image_ReactorCore5(); break;
    case 5:   Image_ReactorCore6(); break;
    case 6:   Image_ReactorCore7(); break;
    case 7:   Image_ReactorCore8(); break;
    case 8:   Image_ReactorCore7(); break;
    case 9:   Image_ReactorCore6(); break;
    case 10:  Image_ReactorCore5(); break;
    case 11:  Image_ReactorCore4(); break;
    case 12:  Image_ReactorCore3(); break;
    case 13:  Image_ReactorCore2(); break;
    case 14:  Image_ReactorCore1(); break;
  }
}

void Pattern_TwisterHalf()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 12);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   ClearMap();   SetYColumn(1, true);  SetYColumn(5, true);  break;
    case 1:   ClearMap();   SetYColumn(2, true);  SetYColumn(6, true);  break;
    case 2:   ClearMap();   SetYColumn(3, true);  SetYColumn(6, true);  break;
    case 3:   ClearMap();   SetYColumn(7, true);  SetYColumn(6, true);  break;
    case 4:   ClearMap();   SetYColumn(11, true); SetYColumn(10, true); break;
    case 5:   ClearMap();   SetYColumn(15, true); SetYColumn(10, true); break;
    case 6:   ClearMap();   SetYColumn(14, true); SetYColumn(10, true); break;
    case 7:   ClearMap();   SetYColumn(13, true); SetYColumn(9, true);  break;
    case 8:   ClearMap();   SetYColumn(12, true); SetYColumn(9, true);  break;
    case 9:   ClearMap();   SetYColumn(8, true);  SetYColumn(9, true);  break;
    case 10:  ClearMap();   SetYColumn(4, true);  SetYColumn(5, true);  break;
    case 11:  ClearMap();   SetYColumn(0, true);  SetYColumn(5, true);  break;
  }
}

void Pattern_TwisterFull()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 12);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   ClearMap();   SetYColumn(1, true);  SetYColumn(5, true);    SetYColumn(10, true); SetYColumn(14, true);  break;
    case 1:   ClearMap();   SetYColumn(2, true);  SetYColumn(6, true);    SetYColumn(13, true); SetYColumn(9, true);  break;
    case 2:   ClearMap();   SetYColumn(3, true);  SetYColumn(6, true);    SetYColumn(9, true);  SetYColumn(12, true);  break;
    case 3:   ClearMap();   SetYColumn(7, true);  SetYColumn(6, true);    SetYColumn(9, true);  SetYColumn(8, true);  break;
    case 4:   ClearMap();   SetYColumn(11, true); SetYColumn(10, true);   SetYColumn(5, true);  SetYColumn(4, true); break;
    case 5:   ClearMap();   SetYColumn(15, true); SetYColumn(10, true);   SetYColumn(5, true);  SetYColumn(0, true); break;
    case 6:   ClearMap();   SetYColumn(14, true); SetYColumn(10, true);   SetYColumn(5, true);  SetYColumn(1, true); break;
    case 7:   ClearMap();   SetYColumn(13, true); SetYColumn(9, true);    SetYColumn(6, true);  SetYColumn(2, true);  break;
    case 8:   ClearMap();   SetYColumn(12, true); SetYColumn(9, true);    SetYColumn(6, true);  SetYColumn(3, true);  break;
    case 9:   ClearMap();   SetYColumn(8, true);  SetYColumn(9, true);    SetYColumn(6, true);  SetYColumn(7, true);  break;
    case 10:  ClearMap();   SetYColumn(4, true);  SetYColumn(5, true);    SetYColumn(10, true); SetYColumn(11, true);  break;
    case 11:  ClearMap();   SetYColumn(0, true);  SetYColumn(5, true);    SetYColumn(10, true); SetYColumn(15, true);  break;
  }
}

void Pattern_Elevators_Prep()
{
  for (int i = 0; i < 16; ++i) Elevators[i] = (random(2) == 0) ? 0 : 3;
}

void Pattern_Elevators()
{
  const long patternFrameTime = 50;
  SetCurrentPatternTime(patternFrameTime * 4 * 100);
  
  static int elevatorShifting = 0;
  static bool elevatorUp = false;
  static int elevatorShiftCount = -1;
  static int elevatorActionCount = -1;
  
  int shiftCount = currentPatternTime / (patternFrameTime * 4);
  if (shiftCount != elevatorShiftCount)
  {
    elevatorShiftCount = shiftCount;
    elevatorShifting = random(16);
    elevatorUp = GetBit(4 * elevatorShifting);
  }

  int actionCount = (currentPatternTime / patternFrameTime) % 4;
  if (actionCount != elevatorActionCount)
  {
    elevatorActionCount = actionCount;
    
    Elevators[elevatorShifting] = elevatorUp ? actionCount : 3 - actionCount;//(4 * elevatorShifting + actionCount + 1) : (4 * elevatorShifting + 3 - actionCount - 1);
    
    ClearMap();
    for (int j = 0; j < 16; ++j) SetBit(4 * j + Elevators[j], CurrentColor);
  }
}

void Pattern_LightUpOneByOne_Prep()
{
  for (int i = 0; i < pixelCount; ++i) LightList[i] = i;
  for (int i = 0; i < pixelCount; ++i)
  {
    int otherIndex = random(pixelCount);
    if (otherIndex == i) continue;
    LightList[i] ^= LightList[otherIndex];
    LightList[otherIndex] ^= LightList[i];
    LightList[i] ^= LightList[otherIndex];
  }

  ClearMap();
}

void Pattern_LightUpOneByOne()
{
  const long patternFrameTime = 50;
  SetCurrentPatternTime(patternFrameTime * pixelCount * 2 * 100);

  static int LightUpOneByOne_Index = -1;
  int currentIndex = (currentPatternTime / (patternFrameTime * pixelCount * 2));
  if (LightUpOneByOne_Index != currentIndex)
  {
    LightUpOneByOne_Index = currentIndex;
    Pattern_LightUpOneByOne_Prep();
  }

  static int LightUpOneByOne_Frame = -1;
  if (((currentPatternTime / patternFrameTime) % (pixelCount * 2)) != LightUpOneByOne_Frame)
  {
    LightUpOneByOne_Frame++;
    LightUpOneByOne_Frame = LightUpOneByOne_Frame % (pixelCount * 2);
    
    if (LightUpOneByOne_Frame < pixelCount) SetBit(LightList[LightUpOneByOne_Frame], CurrentColor);
    else SetBit(LightList[LightUpOneByOne_Frame - pixelCount], COLOR_BLACK);
  }
}

void Pattern_RandomYColumns()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 1000);

  static int columnIndex = -1;
  static int RandomYColumns_Index = -1;
  int currentIndex = (currentPatternTime / patternFrameTime);
  if (RandomYColumns_Index != currentIndex)
  {
    RandomYColumns_Index = currentIndex;
    
    int newIndex = random(16);
    while (newIndex == columnIndex) newIndex = random(16);
    columnIndex = newIndex;
    
    ClearMap();
    SetYColumn(columnIndex, true);
  }
}

void Pattern_Rainfall_Prep()
{
  for (int i = 0; i < RaindropCount; ++i) Raindrops[i] = (random(16) * 4) + random(4);
}

void Pattern_Rainfall()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 15000);

  static int rainfallActionCount = -1;
  int actionCount = currentPatternTime / patternFrameTime;
  if (actionCount == rainfallActionCount) return;
  else rainfallActionCount = actionCount;

  //  Update the collection of raindrops
  for (int i = 0; i < RaindropCount; ++i)
  {
    if (Raindrops[i] % 4 == 0) Raindrops[i] = (random(16) * 4) + 2 + random(2);
    else Raindrops[i]--;
  }

  //  Render the raindrops
  ClearMap();
  for (int i = 0; i < RaindropCount; ++i) SetBit(Raindrops[i], CurrentColor);
}

void Pattern_XAxis()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 6);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   Image_XAxis(0); break;
    case 1:   Image_XAxis(1); break;
    case 2:   Image_XAxis(2); break;
    case 3:   Image_XAxis(3); break;
    case 4:   Image_XAxis(2); break;
    case 5:   Image_XAxis(1); break;
  }
}

void Pattern_YAxis()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 6);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   Image_YAxis(0); break;
    case 1:   Image_YAxis(1); break;
    case 2:   Image_YAxis(2); break;
    case 3:   Image_YAxis(3); break;
    case 4:   Image_YAxis(2); break;
    case 5:   Image_YAxis(1); break;
  }
}

void Pattern_ZAxis()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 6);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   Image_ZAxis(0); break;
    case 1:   Image_ZAxis(1); break;
    case 2:   Image_ZAxis(2); break;
    case 3:   Image_ZAxis(3); break;
    case 4:   Image_ZAxis(2); break;
    case 5:   Image_ZAxis(1); break;
  }
}

void Pattern_XtoYtoXtoZ()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 48);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   Image_XAxis(0); break;
    case 1:   Image_XAxis(1); break;
    case 2:   Image_XAxis(2); break;
    case 3:   Image_XAxis(3); break;
    case 4:   Image_XAxis(2); break;
    case 5:   Image_XAxis(1); break;
    case 6:   Image_XAxis(0); break;
    case 7:   Image_XAxisToYAxis1(); break;
    case 8:   Image_XAxisToYAxis2(); break;
    case 9:   Image_XAxisToYAxis3(); break;
    case 10:   Image_XAxisToYAxis4(); break;
    case 11:   Image_XAxisToYAxis5(); break;
    case 12:   Image_YAxis(0); break;
    case 13:   Image_YAxis(1); break;
    case 14:   Image_YAxis(2); break;
    case 15:   Image_YAxis(3); break;
    case 16:   Image_YAxis(2); break;
    case 17:   Image_YAxis(1); break;
    case 18:   Image_YAxis(0); break;
    case 19:   Image_XAxisToYAxis5(); break;
    case 20:   Image_XAxisToYAxis4(); break;
    case 21:   Image_XAxisToYAxis3(); break;
    case 22:   Image_XAxisToYAxis2(); break;
    case 23:   Image_XAxisToYAxis1(); break;
    case 24:   Image_XAxis(0); break;
    case 25:   Image_XAxis(1); break;
    case 26:   Image_XAxis(2); break;
    case 27:   Image_XAxis(3); break;
    case 28:   Image_XAxis(2); break;
    case 29:   Image_XAxis(1); break;
    case 30:   Image_XAxis(0); break;
    case 31:   Image_XAxisToZAxis1(); break;
    case 32:   Image_XAxisToZAxis2(); break;
    case 33:   Image_XAxisToZAxis3(); break;
    case 34:   Image_XAxisToZAxis4(); break;
    case 35:   Image_XAxisToZAxis5(); break;
    case 36:   Image_ZAxis(0); break;
    case 37:   Image_ZAxis(1); break;
    case 38:   Image_ZAxis(2); break;
    case 39:   Image_ZAxis(3); break;
    case 40:   Image_ZAxis(2); break;
    case 41:   Image_ZAxis(1); break;
    case 42:   Image_ZAxis(0); break;
    case 43:   Image_XAxisToZAxis5(); break;
    case 44:   Image_XAxisToZAxis4(); break;
    case 45:   Image_XAxisToZAxis3(); break;
    case 46:   Image_XAxisToZAxis2(); break;
    case 47:   Image_XAxisToZAxis1(); break;
  }
}

void Pattern_XAxisToYAxis()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 6);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   Image_XAxis(0); break;
    case 1:   Image_XAxisToYAxis1(); break;
    case 2:   Image_XAxisToYAxis2(); break;
    case 3:   Image_XAxisToYAxis3(); break;
    case 4:   Image_XAxisToYAxis4(); break;
    case 5:   Image_XAxisToYAxis5(); break;
  }
}

void Pattern_YAxisToXAxis()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 6);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   Image_YAxis(0); break;
    case 1:   Image_XAxisToYAxis5(); break;
    case 2:   Image_XAxisToYAxis4(); break;
    case 3:   Image_XAxisToYAxis3(); break;
    case 4:   Image_XAxisToYAxis2(); break;
    case 5:   Image_XAxisToYAxis1(); break;
  }
}

void Pattern_XAxisToZAxis()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 6);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   Image_XAxis(0); break;
    case 1:   Image_XAxisToZAxis1(); break;
    case 2:   Image_XAxisToZAxis2(); break;
    case 3:   Image_XAxisToZAxis3(); break;
    case 4:   Image_XAxisToZAxis4(); break;
    case 5:   Image_XAxisToZAxis5(); break;
  }
}

void Pattern_ZAxisToXAxis()
{
  const long patternFrameTime = frameDelay;
  SetCurrentPatternTime(patternFrameTime * 6);
  
  switch (currentPatternTime / patternFrameTime)
  {
    case 0:   Image_ZAxis(0); break;
    case 1:   Image_XAxisToZAxis5(); break;
    case 2:   Image_XAxisToZAxis4(); break;
    case 3:   Image_XAxisToZAxis3(); break;
    case 4:   Image_XAxisToZAxis2(); break;
    case 5:   Image_XAxisToZAxis1(); break;
  }
}

void loop()
{
  unsigned long currentMillis = millis();
  if (buttonTimer < currentMillis)
  {
    if (digitalRead(PATTERN_SWITCH_BUTTON) == LOW) //  Read the pattern switch button
    {
      buttonTimer = currentMillis + BUTTON_DELAY;
      CurrentPattern++;
      if (CurrentPattern > 10) CurrentPattern = 0;
    }
    else if (digitalRead(COLOR_SWITCH_BUTTON) == LOW) //  Read the color switch button
    {
      buttonTimer = currentMillis + BUTTON_DELAY;
      CurrentColor++;
      if (CurrentColor > COLOR_WHITE) CurrentColor = 1;
    }
  }
  
  switch (CurrentPattern)
  {
    case 0:   PatternReactorCore();       break;
    case 1:   Pattern_TwisterHalf();      break;
    case 2:   Pattern_TwisterFull();      break;
    case 3:   Pattern_Elevators();        break;
    case 4:   Pattern_LightUpOneByOne();  break;
    case 5:   Pattern_RandomYColumns();   break;
    case 6:   Pattern_Rainfall();         break;
    case 7:   Pattern_XAxis();            break;
    case 8:   Pattern_YAxis();            break;
    case 9:   Pattern_ZAxis();            break;
    case 10:  Pattern_XtoYtoXtoZ();       break;
  }
  
  RenderDisplay();
  return;
}
