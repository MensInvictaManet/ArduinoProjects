#include <FastLED.h>

#define LED_STRIP_PIN     2         //  The NeoPixel string data pin

#define DELAY_TIME      25

#define NUM_LEDS          76       //  The number of LEDs we want to alter
#define MAX_LEDS          76       //  The number of LEDs on the full strip
#define NUM_LEDS_VIRTUAL  MAX_LEDS + 10   //  The number of LEDs to virtually travel. AKA the loop will continue until the position passes this number
#define BRIGHTNESS        60        //  The number (0 to 200) for the brightness setting)
#define DEBUG_OUTPUT      true

CRGB leds[MAX_LEDS];
int position = 0;

struct Color
{
public:
  Color(int r, int g, int b) : 
    R(r),
    G(g),
    B(b)
  {}

  int R;
  int G;
  int B;
  
  inline bool isZero()    const { return (R == 0 && G == 0 && B == 0);  }
  inline int getValue()   const { return R + G + B; }
};

enum LightGroupNames 
{
  LG_LEFT_LEFT,
  LG_LEFT_TOP,
  LG_LEFT_RIGHT,
  LG_LEFT_BOTTOM,
  LG_CENTER_LEFT,
  LG_CENTER_TOP,
  LG_CENTER_RIGHT,
  LG_CENTER_BOTTOM,
  LG_RIGHT_LEFT,
  LG_RIGHT_TOP,
  LG_RIGHT_RIGHT,
  LG_RIGHT_BOTTOM
};

struct LightGroup
{
public:
  LightGroup(int start, int end) : 
    Start(start),
    End(end)
  {}
  
  LightGroup(int groupIndex)
  {
    switch (groupIndex)
    {
    case LG_LEFT_LEFT:      Start = 0;    End = 79;   break;
    case LG_LEFT_TOP:     Start = 80;   End = 124;  break;
    case LG_LEFT_RIGHT:     Start = 125;  End = 149;  break;
    case LG_LEFT_BOTTOM:    Start = 150;  End = 194;  break;
    case LG_CENTER_LEFT:    Start = 195;  End = 219;  break;
    case LG_CENTER_TOP:     Start = 220;  End = 244;  break;
    case LG_CENTER_RIGHT:   Start = 245;  End = 269;  break;
    case LG_CENTER_BOTTOM:    Start = 270;  End = 294;  break;
    case LG_RIGHT_LEFT:     Start = 295;  End = 319;  break;
    case LG_RIGHT_TOP:      Start = 320;  End = 364;  break;
    case LG_RIGHT_RIGHT:    Start = 365;  End = 444;  break;
    case LG_RIGHT_BOTTOM:   Start = 445;  End = 489;  break;
    }
  }
  
  int Start;
  int End;
};

void SetGroupColor(int group, CRGB color)
{
  LightGroup groupPair(group);
  for (int i = groupPair.Start; i <= groupPair.End; ++i) { leds[i] = color; }
  FastLED.show();
}

//  The basic colors to warp through for GlowFlow
#define COMMON_COLOR_COUNT    15
const int colors[COMMON_COLOR_COUNT][3] = 
{
  { 0, 0, 0 },    //  Black
  { 147, 112, 219 },  //  Medium Purple
  { 199, 21, 133 }, //  Medium Violet Red
  { 255, 20, 147 }, //  Deep Pink
  { 255, 0, 0 },    //  Red
  { 255, 140, 0 },  //  Dark Orange
  { 255, 69, 0 },   //  Orange Red
  { 255, 165, 0 },  //  Orange
  { 200, 200, 0 },  //  Yellow
  { 034, 139, 034 },  //  Forest Green
  { 0, 250, 154 },  //  Medium Spring Green
  { 032, 178, 170 },  //  Light Sea Green
  { 0, 100, 0 },    //  Dark Green
  { 0, 255, 255 },    //  Cyan
  { 070, 130, 180 },  //  Steel Blue
};

//////////////////////////////
//  HELPER FUNCTIONS
//////////////////////////////
CRGB Wheel(byte wheelPosition)
{
    wheelPosition = 255 - wheelPosition;
    if(wheelPosition < 85)
    {
        return CRGB(255 - wheelPosition * 3, 0, wheelPosition * 3);
    }
    else if(wheelPosition < 170)
    {
        wheelPosition -= 85;
        return CRGB(0, wheelPosition * 3, 255 - wheelPosition * 3);
    }
    else
    {
        wheelPosition -= 170;
        return CRGB(wheelPosition * 3, 255 - wheelPosition * 3, 0);
    }
}

void ClearStrip()
{
  fill_solid(leds, MAX_LEDS, CRGB::Black);
}

inline bool IsPositionOnStrip(int position)
{
  return ((position >= 0) && (position < NUM_LEDS));
}

//////////////////////////////
//  PATTERN FUNCTIONS
//////////////////////////////
void Pacman()
{
  static int pacmanPosition = 0;
  
  ClearStrip();
  int P1 = ((pacmanPosition >= NUM_LEDS_VIRTUAL) ? 0 : pacmanPosition);
  int P2 = (((P1 - 3) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 3) : (P1 - 3));
  int P3 = (((P1 - 5) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 5) : (P1 - 5));
  int P4 = (((P1 - 7) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 7) : (P1 - 7));
  int P5 = (((P1 - 9) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 9) : (P1 - 9));
  
  if (IsPositionOnStrip(P1)) leds[P1] = CRGB::Yellow;
  if (IsPositionOnStrip(P2)) leds[P2] = CRGB::Red;
  if (IsPositionOnStrip(P3)) leds[P3] = CRGB::Cyan;
  if (IsPositionOnStrip(P4)) leds[P4] = CRGB::Orange;
  if (IsPositionOnStrip(P5)) leds[P5] = CRGB::Magenta;
  
  FastLED.show();
  pacmanPosition = P1 + 1;
  delay(DELAY_TIME);
}

void RainbowFlow1()
{
  static int hue = 0;
  hue++;
  fill_rainbow(leds, MAX_LEDS, hue, -3);
  FastLED.show();
}

void RainbowFlow2(int speed = 1, bool berzerk = false)
{
  static int rainbowPosition = 0;
  for (int i = 0; i < MAX_LEDS; ++i)
    {
      leds[i] = Wheel(((i * 256 / MAX_LEDS) + rainbowPosition) & 255);
      if (berzerk) rainbowPosition++;
    }
    
    if (!berzerk) rainbowPosition += speed;
  FastLED.show();
}

void Fire()
{
  int r = 255;
  int g = r - 80;
  int b = 40;

  for (int i = 0; i < MAX_LEDS; i++)
  {
    int flicker = random(0,150);
    int r1 = r - flicker;
    int g1 = g - flicker;
    int b1 = b - flicker;
    if (g1 < 0) g1 = 0;
    if (r1 < 0) r1 = 0;
    if (b1 < 0) b1 = 0;
    leds[i] = CRGB(r1, g1, b1);
  }
  FastLED.show();
  delay(random(50,150));
}

void GlowFlow()
{
  static Color colorCurrent(0, 0, 0);
  static Color colorDelta(0, 0, 0);
  static bool colorDisplay = true;
  static const int colorDeltaDelay = 15;
  static const int minColorTime = 5000;
  static unsigned long nextChangeTime = 0;
  static bool adding = false;
  
  adding = false;
  if (colorDelta.R > 0) { colorDelta.R -= 1; colorCurrent.R = min(255, colorCurrent.R + 1); delay(colorDeltaDelay); adding = true; }
  if (colorDelta.G > 0) { colorDelta.G -= 1; colorCurrent.G = min(255, colorCurrent.G + 1); delay(colorDeltaDelay); adding = true; }
  if (colorDelta.B > 0) { colorDelta.B -= 1; colorCurrent.B = min(255, colorCurrent.B + 1); delay(colorDeltaDelay); adding = true; }
  if (adding == false)
  {
    if (colorDelta.R < 0) { colorDelta.R += 1; colorCurrent.R = max(0,   colorCurrent.R - 1); delay(colorDeltaDelay); }
    if (colorDelta.G < 0) { colorDelta.G += 1; colorCurrent.G = max(0,   colorCurrent.G - 1); delay(colorDeltaDelay); }
    if (colorDelta.B < 0) { colorDelta.B += 1; colorCurrent.B = max(0,   colorCurrent.B - 1); delay(colorDeltaDelay); }
  }
  
  CRGB currentCRGB(colorCurrent.R, colorCurrent.G, colorCurrent.B);
  fill_solid(leds, MAX_LEDS, currentCRGB);
  FastLED.show();
    
  if (colorDelta.isZero() && nextChangeTime < millis())
  {
    int newColorIndex = 0;
    do
    {
      newColorIndex = 1 + random(COMMON_COLOR_COUNT - 1);
      colorDelta.R = colors[newColorIndex][0] - colorCurrent.R;
      colorDelta.G = colors[newColorIndex][1] - colorCurrent.G;
      colorDelta.B = colors[newColorIndex][2] - colorCurrent.B;
    } while (colorDelta.isZero());
    
    nextChangeTime = millis() + minColorTime;
  }
}

void GroupTest()
{
  ClearStrip();
  SetGroupColor(LG_LEFT_LEFT, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_LEFT_TOP, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_LEFT_RIGHT, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_LEFT_BOTTOM, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_CENTER_LEFT, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_CENTER_TOP, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_CENTER_RIGHT, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_CENTER_BOTTOM, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_RIGHT_LEFT, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_RIGHT_TOP, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_RIGHT_RIGHT, CRGB::Red);
  FastLED.show();
  delay(250);
  
  ClearStrip();
  SetGroupColor(LG_RIGHT_BOTTOM, CRGB::Red);
  FastLED.show();
  delay(250);
}

void setup()
{
  //  Setup the LED strip and color all LEDs black
  FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  ClearStrip();
  FastLED.show();
  
  //  Set up the serial connection
  Serial.begin(9600);
  
  //  Seed the random number generator
  randomSeed(analogRead(0));
}

void loop()
{
  static const long int PatternTime = 10000;
  static long int PatternCount = 7;
  static int state = 0;
  
  if (Serial.available() > 0)
  {
    state = Serial.read(); // used to read incoming data
    
    switch(state)// see what was sent to the board
    {
    case '1': // if the the one was sent
      PatternCount = 3;
      break;
    case '0': // if 0 was sent
      PatternCount = 7;
      break;
    default:
      break; 
    }
  }
  
  long int patternIndex = (millis() % (PatternTime * PatternCount) / PatternTime);
  switch (patternIndex)
  {
    case 0:   RainbowFlow1();       break;
    case 1:   RainbowFlow2();       break;
    case 2:   RainbowFlow2(0, true);    break;
    case 3:   Pacman();         break;
    case 4:   Fire();           break;
    case 5:   GlowFlow();         break;
    default:  GroupTest();        break;
  }
}


