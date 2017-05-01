#include <FastLED.h>

#define LED_DATA_PIN            2         //  The WS2801 string data pin
#define LED_CLOCK_PIN           4         //  The WS2801 string clock pin
#define POTENTIOMETER_PIN       A0        //  The data pin to the potentiometer
#define BUTTON_PIN              10        //  The pin that the button is connected to

#define PANEL_WIDTH             28
#define PANEL_HEIGHT            28
#define NUM_LEDS                PANEL_WIDTH * PANEL_HEIGHT        //  The number of LEDs we want to control
#define MAX_LEDS                NUM_LEDS                          //  The number of LEDs on the full strip
#define BRIGHTNESS              60                                //  The number (0 to 200) for the brightness setting)

#define A_BUTTON                0
#define B_BUTTON                1
#define SELECT_BUTTON           2
#define START_BUTTON            3
#define UP_BUTTON               4
#define DOWN_BUTTON             5
#define LEFT_BUTTON             6
#define RIGHT_BUTTON            7

#define NES_DATA_PIN            6
#define NES_CLOCK_PIN           4
#define NES_LATCH_PIN           5

#define TETRIS_BOARD_START_X    9
#define TETRIS_BOARD_START_Y    4
#define TETRIS_BOARD_WIDTH      10
#define TETRIS_BOARD_HEIGHT     20

byte NESRegister = 0; //  We will use this to hold current button states

#define GHOST_CART              false

unsigned long startTime = 0;
unsigned long buttonTimer = 0;
unsigned long millisOffset = 0;
unsigned long nextFrameMillis = 0;
byte patternIndex = 0;

//  Generic values
#define FRAME_MILLIS            333
#define BUTTON_DELAY            400

CRGB leds[MAX_LEDS];

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

CRGB Wheel2(byte wheelPosition)
{
  if(wheelPosition < 85)
  {
    return CRGB(wheelPosition * 3, 255 - wheelPosition * 3, 0);
  }
  else if(wheelPosition < 170)
  {
    wheelPosition -= 85;
    return CRGB(255 - wheelPosition * 3, 0, wheelPosition * 3);
  }
  else
  {
    wheelPosition -= 170;
    return CRGB(0, wheelPosition * 3, 255 - wheelPosition * 3);
  }
}

int LoopedLEDIndex(int index)
{
  index = index % MAX_LEDS;
  while (index < 0) index = (MAX_LEDS + index) % MAX_LEDS;
  return index;
}

struct Color
{
public:
  int R, G, B;
  Color(int r, int g, int b) : R(r), G(g), B(b) {}
  inline bool isZero()                const { return (R == 0 && G == 0 && B == 0);  }
  inline bool isLessThan(int amount)  const { return (abs(R) < amount && abs(G) < amount && abs(B) < amount);  }
};

//  The basic colors
#define COMMON_COLOR_COUNT    55
const byte COMMON_COLORS[COMMON_COLOR_COUNT][3] = 
{
  {   0,   0,   0 },  //  Black
  { 255,   0,   0 },  //  Red
  {   0, 255,   0 },  //  Green
  {   0,   0, 255 },  //  Blue
  { 200, 200,   0 },  //  Yellow
  {   0, 255, 255 },  //  Cyan (Mario Morph BG)
  { 255,   0, 255 },  //  Magenta
  { 255, 255, 255 },  //  White
  { 100, 100, 100 },  //  Pale White (MegaMan BG)
  {  67,  79, 254 },  //  MegaMan Blue 1
  {  36, 242, 205 },  //  MegaMan Blue 2
  { 251, 236, 154 },  //  MegaMan Skin
  { 240, 208, 176 },  //  SMB3 Mario Skin
  { 248, 056,   0 },  //  SMB3 Mario Raccoon 1
  { 240, 144,  88 },  //  SMW2 Skin 1
  { 232,  96,  80 },  //  SMW2 Skin 2
  { 248, 248, 248 },  //  SMW2 Buttons
  { 168,  80,   0 },  //  SMW2 Raccoon 1
  { 120,  64,  40 },  //  SMW2 Raccoon 2
  { 224, 128,  48 },  //  SMW2 Raccoon 3
  { 240, 200,  48 },  //  SMW2 Raccoon 4
  { 248, 184, 128 },  //  SMW2 Raccoon 5
  {  40,  72, 128 },  //  SMW2 Overalls 1 (dark blue)
  {  56, 112, 168 },  //  SMW2 Overalls 2 (light blue)
  {  72, 152, 208 },  //  SMW2 Overalls 3 (lightest blue)
  { 160,   0,   0 },  //  SMW2 ShirtHat 1 (dark red)
  { 200,   0,  24 },  //  SMW2 ShirtHat 2 (light red)
  { 248,  32,  56 },  //  SMW2 ShirtHat 3 (lightest red)
  { 231,  95,  19 },  //  SMB3 Tanuki 1
  { 240, 208, 176 },  //  SMB3 Tanuki 2
  { 144,  56,  24 },  //  SMW2 Tanuki 1 (dark brown)
  { 184,  96,  24 },  //  SMW2 Tanuki 2 (light brown)
  { 216, 128,  24 },  //  SMW2 Tanuki 3 (yellow brown)
  { 184,  96,  24 },  //  SMW2 Tanuki 4 (lightest brown)
  { 248,  56,   0 },  //  SMB3 FireFlower 1
  { 255, 163,  71 },  //  SMB3 FireFlower 2
  { 200,  16,   0 },  //  SMW2 FireFlower 1 (dark red)
  { 248,  64,   0 },  //  SMW2 FireFlower 2 (orange)
  { 248, 104,  32 },  //  SMW2 FireFlower 3 (light orange)
  { 248, 208,  88 },  //  SMW2 FireHat 1 (yellow)
  { 248, 240, 176 },  //  SMW2 FireHat 2 (light yellow)
  {  88,  72,  72 },  //  SMW2 hardhat 1 (dark gray)
  { 136, 120, 120 },  //  SMW2 hardhat 2 (gray)
  { 192, 176, 176 },  //  SMW2 hardhat 3 (light gray)
  { 248, 208, 152 },  //  SMW2 YellowHat 2 (light yellow)
  { 248, 176,  48 },  //  SMW2 YellowHat 1 (yellow)
  { 176,  88,   0 },  //  SMW2 BrownSuit 1 (brown-yellow)
  { 208, 128,  24 },  //  SMW2 BrownSuit 2 (yellow-brown)
  { 255, 165,   0 },  //  Orange
  { 128,   0, 128 },  //  Purple
  { 253,  15,  15 },  //  Pac-Man Blinky Red
  { 253, 150, 180 },  //  Pac-Man Pinky Pink
  {  93, 243, 212 },  //  Pac-Man Inky Blue
  { 255, 171,  77 },  //  Pac-Man Clyde Orange
  {  40,  40,  40 },  //  Tetris Background
};

//  Frame/Animation Helper Functions
inline bool IsNextFrameReady() { return (FrameMillis() >= nextFrameMillis); }
inline void UpdateMillisOffset() { millisOffset = millis(); nextFrameMillis = 0;}
inline unsigned long FrameMillis() { return millis() - millisOffset; }

inline void SetStrip(CRGB color) { fill_solid(leds, MAX_LEDS, color); }
inline void ClearStrip() { SetStrip(CRGB::Black); }
inline bool IsPixelBlack(int x, int y) { return (leds[PositionTranslate(x, y)].r == 0 && leds[PositionTranslate(x, y)].g == 0 && leds[PositionTranslate(x, y)].b == 0); }
inline bool IsPixelBlack(int index) { return (leds[index].r == 0 && leds[index].g == 0 && leds[index].b == 0); }
inline bool IsPositionOnStrip(int pos) { return ((pos >= 0) && (pos < NUM_LEDS)); }
inline bool IsPositionOnStrip(int x, int y) { return ((x >= 0 && x < PANEL_WIDTH) && (y >= 0 && y < PANEL_HEIGHT) && (((y * PANEL_WIDTH + x) >= 0) && ((y * PANEL_WIDTH + x) < NUM_LEDS))); }
inline int PositionTranslate(int pos) { return (((pos / PANEL_WIDTH) & 1) ? (((pos / PANEL_WIDTH + 1) * PANEL_WIDTH) - 1 - (pos % PANEL_WIDTH)) : pos); }
inline int PositionTranslate(int x, int y) { int pos = (y * PANEL_WIDTH + x); return (((pos / PANEL_WIDTH) & 1) ? (((pos / PANEL_WIDTH + 1) * PANEL_WIDTH) - 1 - (pos % PANEL_WIDTH)) : pos); }
inline const CRGB& GetPixel(int x, int y) { if (!IsPositionOnStrip(x, y)) return CRGB::Black; else return leds[PositionTranslate(x, y)]; }
inline byte GetColor(byte colorIndex, int part) { return COMMON_COLORS[colorIndex][part]; }
inline int GetFrame(int animationLength) { return (((FrameMillis() - startTime) / FRAME_MILLIS) % animationLength); }
inline int GetFrame(int animationLength, int frameMillis) { return (((FrameMillis() - startTime) / frameMillis) % animationLength); }

inline int GetRandomVibrantColorIndex()
{
  switch (random(9))
  {
    case 0:   return 1;   break;
    case 1:   return 2;   break;
    case 2:   return 3;   break;
    case 3:   return 4;   break;
    case 4:   return 5;   break;
    case 5:   return 6;   break;
    case 6:   return 7;   break;
    case 7:   return 48;   break;
    case 8:   return 49;   break;
  }
}

inline void SetLED(int pos, byte color)
{
  if (IsPositionOnStrip(PositionTranslate(pos)) == false) return;
  
  leds[PositionTranslate(pos)] = CRGB(GetColor(color, 0), GetColor(color, 1), GetColor(color, 2));
}

inline void SetLED(int pos, CRGB color)
{
  if (IsPositionOnStrip(PositionTranslate(pos)) == false) return;
  
  leds[PositionTranslate(pos)] = color;
}

inline void SetLED(int x, int y, byte color)
{
  if (IsPositionOnStrip(x, y) == false) return;
  
  leds[PositionTranslate(x, y)] = CRGB(GetColor(color, 0), GetColor(color, 1), GetColor(color, 2));
}

inline void SetLED(int x, int y, const CRGB& color)
{
  if (IsPositionOnStrip(x, y) == false) return;
  
  leds[PositionTranslate(x, y)] = color;
}

inline void SetLights(int x, int y, int count, byte color)
{
  for (int i = 0; i < count; ++i)
  {
    if (IsPositionOnStrip(x + i, y) == false) continue;
    SetLED(x + i, y, color);
  }
}

/*
inline void SetLights(int* positions, int count, byte color)
{
  for (int i = 0; i < count; ++i)
  {
    if (positions[i] >= NUM_LEDS)
    {
      Serial.print("ERROR [SetLights(position ");
      Serial.print(positions[i]);
      Serial.println(")]: - Attempting to write to LED that is not in our control");
      continue;
    }
    SetLED(positions[i], color);
  }
}
inline void SetLights(int pos, int count, byte color)
{
  if ((pos + count - 1) >= NUM_LEDS)
  {
    Serial.print("ERROR [SetLights(pos ");
    Serial.print(pos);
    Serial.print(", count ");
    Serial.print(count);
    Serial.println(")]: - Attempting to write to LED that is not in our control");
    count = max(0, NUM_LEDS - pos);
  }

  for (int i = 0; i < count; ++i) SetLED(pos + i, color);
}

inline void SetLights(int pos, int count, byte* colors)
{
  if ((pos + count - 1) >= NUM_LEDS)
  {
    Serial.println("ERROR: Attempting to write to LED that is not in our control");
    count = max(0, NUM_LEDS - pos);
  }

  for (int i = 0; i < count; ++i) SetLED(pos + i, colors[i]);
}
*/

void ResetTetris(int x = 14, int y = 13, byte outerBG = 54, byte innerBG = 0)
{
  SetLights(x - 14, y - 13, 28, outerBG); // START ROW -13
  SetLights(x - 14, y - 12, 28, outerBG); // START ROW -12
  SetLights(x - 14, y - 11, 28, outerBG); // START ROW -11
  SetLights(x - 14, y - 10, 28, outerBG); // START ROW -10
  SetLights(x - 14, y - 9, 9, outerBG); // START ROW -9
  SetLights(x - 5, y - 9, 10, innerBG);
  SetLED(x + 5, y - 9, outerBG);
  SetLights(x + 6, y - 9, 6, innerBG);
  SetLights(x + 12, y - 9, 2, outerBG);
  SetLights(x - 14, y - 8, 9, outerBG); // START ROW -8
  SetLights(x - 5, y - 8, 10, innerBG);
  SetLED(x + 5, y - 8, outerBG);
  SetLights(x + 6, y - 8, 6, innerBG);
  SetLights(x + 12, y - 8, 2, outerBG);
  SetLights(x - 14, y - 7, 9, outerBG); // START ROW -7
  SetLights(x - 5, y - 7, 10, innerBG);
  SetLED(x + 5, y - 7, outerBG);
  SetLights(x + 6, y - 7, 6, innerBG);
  SetLights(x + 12, y - 7, 2, outerBG);
  SetLights(x - 14, y - 6, 9, outerBG); // START ROW -6
  SetLights(x - 5, y - 6, 10, innerBG);
  SetLED(x + 5, y - 6, outerBG);
  SetLights(x + 6, y - 6, 6, innerBG);
  SetLights(x + 12, y - 6, 2, outerBG);
  SetLights(x - 14, y - 5, 9, outerBG); // START ROW -5
  SetLights(x - 5, y - 5, 10, innerBG);
  SetLED(x + 5, y - 5, outerBG);
  SetLights(x + 6, y - 5, 6, innerBG);
  SetLights(x + 12, y - 5, 2, outerBG);
  SetLights(x - 14, y - 4, 9, outerBG); // START ROW -4
  SetLights(x - 5, y - 4, 10, innerBG);
  SetLED(x + 5, y - 4, outerBG);
  SetLights(x + 6, y - 4, 6, innerBG);
  SetLights(x + 12, y - 4, 2, outerBG);
  SetLights(x - 14, y - 3, 9, outerBG); // START ROW -3
  SetLights(x - 5, y - 3, 10, innerBG);
  SetLights(x + 5, y - 3, 9, outerBG);
  SetLights(x - 14, y - 2, 9, outerBG); // START ROW -2
  SetLights(x - 5, y - 2, 10, innerBG);
  SetLights(x + 5, y - 2, 9, outerBG);
  SetLights(x - 14, y - 1, 9, outerBG); // START ROW -1
  SetLights(x - 5, y - 1, 10, innerBG);
  SetLights(x + 5, y - 1, 9, outerBG);
  SetLights(x - 14, y + 0, 9, outerBG); // START ROW +0
  SetLights(x - 5, y + 0, 10, innerBG);
  SetLights(x + 5, y + 0, 9, outerBG);
  SetLights(x - 14, y + 1, 9, outerBG); // START ROW +1
  SetLights(x - 5, y + 1, 10, innerBG);
  SetLights(x + 5, y + 1, 9, outerBG);
  SetLights(x - 14, y + 2, 9, outerBG); // START ROW +2
  SetLights(x - 5, y + 2, 10, innerBG);
  SetLights(x + 5, y + 2, 9, outerBG);
  SetLights(x - 14, y + 3, 9, outerBG); // START ROW +3
  SetLights(x - 5, y + 3, 10, innerBG);
  SetLights(x + 5, y + 3, 9, outerBG);
  SetLights(x - 14, y + 4, 9, outerBG); // START ROW +4
  SetLights(x - 5, y + 4, 10, innerBG);
  SetLights(x + 5, y + 4, 9, outerBG);
  SetLights(x - 14, y + 5, 9, outerBG); // START ROW +5
  SetLights(x - 5, y + 5, 10, innerBG);
  SetLights(x + 5, y + 5, 9, outerBG);
  SetLights(x - 14, y + 6, 9, outerBG); // START ROW +6
  SetLights(x - 5, y + 6, 10, innerBG);
  SetLights(x + 5, y + 6, 9, outerBG);
  SetLights(x - 14, y + 7, 9, outerBG); // START ROW +7
  SetLights(x - 5, y + 7, 10, innerBG);
  SetLights(x + 5, y + 7, 9, outerBG);
  SetLights(x - 14, y + 8, 9, outerBG); // START ROW +8
  SetLights(x - 5, y + 8, 10, innerBG);
  SetLights(x + 5, y + 8, 9, outerBG);
  SetLights(x - 14, y + 9, 9, outerBG); // START ROW +9
  SetLights(x - 5, y + 9, 10, innerBG);
  SetLights(x + 5, y + 9, 9, outerBG);
  SetLights(x - 14, y + 10, 9, outerBG); // START ROW +10
  SetLights(x - 5, y + 10, 10, innerBG);
  SetLights(x + 5, y + 10, 9, outerBG);
  SetLights(x - 14, y + 11, 28, outerBG); // START ROW +11
  SetLights(x - 14, y + 12, 28, outerBG); // START ROW +12
  SetLights(x - 14, y + 13, 28, outerBG); // START ROW +13
  SetLights(x - 14, y + 14, 28, outerBG); // START ROW +14
}

void RainbowFlow1()
{
  static int delayTime = 1;
  static int hueChange = 15;

  if (IsNextFrameReady())
  {
    static int hue = 0;
    hue += hueChange;
    fill_rainbow(leds, MAX_LEDS, hue, -3);
    FastLED.show();
    nextFrameMillis += delayTime;
  }
}

void RainbowFlow2(int hueChange = 1, bool berzerk = false)
{
  static int delayTime = 40;
  
  if (IsNextFrameReady())
  {
    static int rainbowPosition = 0;
    for (int i = 0; i < MAX_LEDS; ++i)
    {
      SetLED(i, Wheel(((i * 256 / MAX_LEDS) + rainbowPosition) & 255));
      if (berzerk) rainbowPosition += hueChange;
    }
    
    if (!berzerk) rainbowPosition += hueChange;
    FastLED.show();
    
    nextFrameMillis += delayTime;
  }
}

void Fire(byte R, byte G, byte B)
{
  static int delayTime = 25;
  static int hueChange = 15;

  if (IsNextFrameReady())
  {
    int r = R;
    int g = G;
    int b = B;
  
    for (int i = 0; i < MAX_LEDS; i++)
    {
      byte flicker = random(0,150);
      byte r1 = r - flicker;
      byte g1 = g - flicker;
      byte b1 = b - flicker;
      if (g1 < 0) g1 = 0;
      if (r1 < 0) r1 = 0;
      if (b1 < 0) b1 = 0;
      leds[i] = CRGB(r1, g1, b1);
    }
    FastLED.show();
    nextFrameMillis += random(delayTime * 2, delayTime * 6);
  }
}

void ColorFire(byte colorChangeSpeed = 5)
{
  static byte R = random(0, 256);
  static byte G = random(0, 256);
  static byte B = random(0, 256);
    
  R += random(1, (colorChangeSpeed + 1) * 1);
  G += random(1, (colorChangeSpeed + 1) * 2);
  B += random(1, (colorChangeSpeed + 1) * 3);

  Fire(R, G, B);
}

void GlowFlow(const int colorChangeSpeed = 1, const unsigned long changeDelay = 1000)
{
  static Color colorCurrent(0, 0, 0);
  static Color colorDelta(0, 0, 0);
  static bool colorDisplay = true;
  static unsigned long nextChangeTime = 0;
  static bool adding = false;
  
  adding = false;
  if (colorDelta.R > 0) { colorDelta.R -= colorChangeSpeed; colorCurrent.R = min(255, colorCurrent.R + colorChangeSpeed); adding = true; }
  if (colorDelta.G > 0) { colorDelta.G -= colorChangeSpeed; colorCurrent.G = min(255, colorCurrent.G + colorChangeSpeed); adding = true; }
  if (colorDelta.B > 0) { colorDelta.B -= colorChangeSpeed; colorCurrent.B = min(255, colorCurrent.B + colorChangeSpeed); adding = true; }
  if (adding == false)
  {
    if (colorDelta.R < 0) { colorDelta.R += colorChangeSpeed; colorCurrent.R = max(0,   colorCurrent.R - colorChangeSpeed); }
    if (colorDelta.G < 0) { colorDelta.G += colorChangeSpeed; colorCurrent.G = max(0,   colorCurrent.G - colorChangeSpeed); }
    if (colorDelta.B < 0) { colorDelta.B += colorChangeSpeed; colorCurrent.B = max(0,   colorCurrent.B - colorChangeSpeed); }
  }
  
  CRGB currentCRGB(colorCurrent.R, colorCurrent.G, colorCurrent.B);
  fill_solid(leds, MAX_LEDS, currentCRGB);
  FastLED.show();
    
  if (colorDelta.isLessThan(colorChangeSpeed) && (millis() >= nextChangeTime))
  {
    int newColorIndex = 0;
    do
    {
      newColorIndex = GetRandomVibrantColorIndex();
      
      colorDelta.R = int(COMMON_COLORS[newColorIndex][0]) - colorCurrent.R;
      colorDelta.G = int(COMMON_COLORS[newColorIndex][1]) - colorCurrent.G;
      colorDelta.B = int(COMMON_COLORS[newColorIndex][2]) - colorCurrent.B;
      
    } while (colorDelta.isLessThan(colorChangeSpeed));
    
    nextChangeTime = millis() + (changeDelay);
  }
}

void TestAnimation()
{
  Serial.println("TestAnimation()");
  int frame = GetFrame(2);
  for (int i = 0; i < PANEL_HEIGHT; ++i)
    for (int j = 0; j < PANEL_WIDTH / 2; ++j)
      SetLED((i * PANEL_WIDTH) + (j * 2) + frame, ((i & 1) ? 1 : 8));
}

void DrawPacManChomp01(int x, int y, byte color)
{
/*
 *     XXXXX
 *   XXXXXXXXX
 *  XXXXXXXXXXX
 *  XXXXXXXXXXX
 * XXXXXXXXXXXXX
 * XXXXXXXXXXXXX
 * XXXXXXXXXXXXX
 * XXXXXXXXXXXXX
 * XXXXXXXXXXXXX
 *  XXXXXXXXXXX
 *  XXXXXXXXXXX
 *   XXXXXXXXX
 *     XXXXX
 */
 
  SetLights(x - 2, y - 6, 5, color); //  START ROW -6
  SetLights(x - 4, y - 5, 9, color); //  START ROW -5
  SetLights(x - 5, y - 4, 11, color); //  START ROW -4
  SetLights(x - 5, y - 3, 11, color); //  START ROW -3
  SetLights(x - 6, y - 2, 13, color); //  START ROW -2
  SetLights(x - 6, y - 1, 13, color); //  START ROW -1
  SetLights(x - 6, y + 0, 13, color); //  START ROW +0
  SetLights(x - 6, y + 1, 13, color); //  START ROW +1
  SetLights(x - 6, y + 2, 13, color); //  START ROW +2
  SetLights(x - 5, y + 3, 11, color); //  START ROW +3
  SetLights(x - 5, y + 4, 11, color); //  START ROW +4
  SetLights(x - 4, y + 5, 9, color); //  START ROW +5
  SetLights(x - 2, y + 6, 5, color); //  START ROW +6
}

void DrawPacManChomp02(int x, int y, byte color)
{
/*
 *     XXXXX
 *   XXXXXXXXX
 *  XXXXXXXXXXX
 *  XXXXXXXXXXX
 * XXXXXXXXXX
 * XXXXXXX
 * XXXX
 * XXXXXXX
 * XXXXXXXXXX
 *  XXXXXXXXXXX
 *  XXXXXXXXXXX
 *   XXXXXXXXX
 *     XXXXX
 */
 
  SetLights(x - 2, y - 6, 5, color); //  START ROW -6
  SetLights(x - 4, y - 5, 9, color); //  START ROW -5
  SetLights(x - 5, y - 4, 11, color); //  START ROW -4
  SetLights(x - 5, y - 3, 11, color); //  START ROW -3
  SetLights(x - 6, y - 2, 10, color); //  START ROW -2
  SetLights(x - 6, y - 1, 7, color); //  START ROW -1
  SetLights(x - 6, y + 0, 4, color); //  START ROW +0
  SetLights(x - 6, y + 1, 7, color); //  START ROW +1
  SetLights(x - 6, y + 2, 10, color); //  START ROW +2
  SetLights(x - 5, y + 3, 11, color); //  START ROW +3
  SetLights(x - 5, y + 4, 11, color); //  START ROW +4
  SetLights(x - 4, y + 5, 9, color); //  START ROW +5
  SetLights(x - 2, y + 6, 5, color); //  START ROW +6
}

void DrawPacManChomp03(int x, int y, byte color)
{
/*
 *     XXXXX
 *   XXXXXXX
 *  XXXXXXX
 *  XXXXXX
 * XXXXXX
 * XXXXX
 * XXXX
 * XXXXX
 * XXXXXX
 *  XXXXXX
 *  XXXXXXX
 *   XXXXXXX
 *     XXXXX
 */
 
  SetLights(x - 2, y - 6, 5, color); //  START ROW -6
  SetLights(x - 4, y - 5, 7, color); //  START ROW -5
  SetLights(x - 5, y - 4, 7, color); //  START ROW -4
  SetLights(x - 5, y - 3, 6, color); //  START ROW -3
  SetLights(x - 6, y - 2, 6, color); //  START ROW -2
  SetLights(x - 6, y - 1, 5, color); //  START ROW -1
  SetLights(x - 6, y + 0, 4, color); //  START ROW +0
  SetLights(x - 6, y + 1, 5, color); //  START ROW +1
  SetLights(x - 6, y + 2, 6, color); //  START ROW +2
  SetLights(x - 5, y + 3, 6, color); //  START ROW +3
  SetLights(x - 5, y + 4, 7, color); //  START ROW +4
  SetLights(x - 4, y + 5, 7, color); //  START ROW +5
  SetLights(x - 2, y + 6, 5, color); //  START ROW +6
}

void PacManChompDanceThrough(int x = -6, int y = 14, int frameLength = 46, int animationTime = 100)
{
  Serial.println("PacManChompDanceThrough()");
  int frame = GetFrame(frameLength, animationTime);
  
  if (frame % 8 == 0)       DrawPacManChomp01(frame + x, y, 4);
  else if (frame % 8 == 1)  DrawPacManChomp02(frame + x, y, 4);
  else if (frame % 8 == 2)  DrawPacManChomp03(frame + x, y, 4);
  else if (frame % 8 == 3)  DrawPacManChomp02(frame + x, y, 4);
  else if (frame % 8 == 4)  DrawPacManChomp01(frame + x, y, 4);
  else if (frame % 8 == 5)  DrawPacManChomp02(frame + x, y, 4);
  else if (frame % 8 == 6)  DrawPacManChomp03(frame + x, y, 4);
  else                      DrawPacManChomp02(frame + x, y, 4);
}

void DrawMsPacManChomp01(int x, int y, byte color1, byte color2, byte color3)
{
/*
 *           OO
 *      XXXXOOO
 *    XXXXXXOOMO
 *   XXXXXXXXXOMOO
 *   XXXXXXXXXXOOO
 *  XXXXXX    XOO
 * MXXXXXXXXXXXXX
 * MMXXXXXXXXXXXX
 * MXXXXXXXXXXXXX
 *  XXXXXXXXX XXX
 *   XXXXXXXXXXX
 *   XXXXXXXXXXX
 *    XXXXXXXXX
 *      XXXXX
 */

  SetLights(x + 3, y - 7, 2, color2); //  START ROW -7
  SetLights(x - 2, y - 6, 4, color1); //  START ROW -6 
  SetLights(x + 2, y - 6, 3, color2);
  SetLights(x - 4, y - 5, 6, color1); //  START ROW -5
  SetLights(x + 2, y - 5, 2, color2);
  SetLED(x + 4, y - 5, color3);
  SetLED(x + 5, y - 5, color2);
  SetLights(x - 5, y - 4, 9, color1); //  START ROW -4
  SetLED(x + 4, y - 4, color2);
  SetLED(x + 5, y - 4, color3);
  SetLights(x + 6, y - 4, 2, color2);
  SetLights(x - 5, y - 3, 10, color1); //  START ROW -3
  SetLights(x + 5, y - 3, 3, color2);
  SetLights(x - 6, y - 2, 5, color1); //  START ROW -2
  SetLED(x + 4, y - 2, color1);
  SetLights(x + 5, y - 2, 2, color2);
  SetLED(x - 7, y - 1, color2); //  START ROW -1
  SetLights(x - 6, y - 1, 13, color1);
  SetLights(x - 7, y + 0, 2, color2); //  START ROW +0
  SetLights(x - 5, y + 0, 12, color1);
  SetLED(x - 7, y + 1, color2); //  START ROW +1
  SetLights(x - 6, y + 1, 13, color1);
  SetLights(x - 6, y + 2, 9, color1); //  START ROW +2
  SetLights(x + 4, y + 2, 3, color1);
  SetLights(x - 5, y + 3, 11, color1); //  START ROW +3
  SetLights(x - 5, y + 4, 11, color1); //  START ROW +4
  SetLights(x - 4, y + 5, 9, color1); //  START ROW +5
  SetLights(x - 2, y + 6, 5, color1); //  START ROW +6
}

void DrawMsPacManChomp02(int x, int y, byte color1, byte color2, byte color3)
{
/*
 *           OO
 *      XXXXOOO
 *    XXXXXXOOMO
 *   XXXXXXXXXOMOO
 *   MMXXX  XXXOOO
 *       XXM XXOO
 *         XXXXXX
 *           XXXX
 *         XXXXXX
 *       XXXX XXX
 *   MMXXXXXXXXX
 *   XXXXXXXXXXX
 *    XXXXXXXXX
 *      XXXXX
 */

  SetLights(x + 3, y - 7, 2, color2); //  START ROW -7
  SetLights(x - 2, y - 6, 4, color2); //  START ROW -6 
  SetLights(x + 2, y - 6, 3, color2);
  SetLights(x - 4, y - 5, 4, color1); //  START ROW -5
  SetLights(x + 2, y - 5, 2, color2);
  SetLED(x + 4, y - 5, color3);
  SetLED(x + 5, y - 5, color2);
  SetLights(x - 5, y - 4, 9, color1); //  START ROW -4
  SetLED(x + 4, y - 4, color2);
  SetLED(x + 5, y - 4, color3);
  SetLights(x + 6, y - 4, 2, color2);
  SetLights(x - 5, y - 3, 2, color2); //  START ROW -3
  SetLights(x - 3, y - 3, 3, color1);
  SetLights(x + 2, y - 3, 3, color1);
  SetLights(x + 5, y - 3, 3, color2);
  SetLights(x - 1, y - 2, 2, color1); //  START ROW -2
  SetLED(x + 1, y - 2, color3);
  SetLights(x + 3, y - 2, 2, color1);
  SetLights(x + 5, y - 2, 2, color2);
  SetLights(x + 1, y - 1, 6, color1); //  START ROW -1
  SetLights(x + 3, y + 0, 4, color1); //  START ROW +0
  SetLights(x + 1, y + 1, 6, color1); //  START ROW +1
  SetLights(x - 1, y + 2, 4, color1); //  START ROW +2
  SetLights(x + 4, y + 2, 3, color1);
  SetLights(x - 5, y + 3, 2, color2); //  START ROW +3
  SetLights(x - 3, y + 3, 9, color1);
  SetLights(x - 5, y + 4, 11, color1); //  START ROW +4
  SetLights(x - 4, y + 5, 9, color1); //  START ROW +5
  SetLights(x - 2, y + 6, 5, color1); //  START ROW +6
}

void DrawMsPacManChomp03(int x, int y, byte color1, byte color2, byte color3)
{
/*
 *           OO
 *     MXXXXOOO
 *      MXXXOOMO
 *       XX XXOMOO
 *        X  XXOOO
 *         XM XOO
 *          XXXXX
 *           XXXX
 *          XXXXX
 *         XX XXX
 *        XXXXXX
 *       XXXXXXX
 *      MXXXXXX
 *     MXXXXX
 */

  SetLights(x + 3, y - 7, 2, color2); //  START ROW -7
  SetLED(x - 3, y - 6, color2); //  START ROW -6 
  SetLights(x - 2, y - 6, 4, color1);
  SetLights(x + 2, y - 6, 3, color2);
  SetLED(x - 2, y - 5, color2); //  START ROW -5
  SetLights(x - 1, y - 5, 3, color1);
  SetLights(x + 2, y - 5, 2, color2);
  SetLED(x + 4, y - 5, color3);
  SetLED(x + 5, y - 5, color2);
  SetLights(x - 1, y - 4, 2, color1); //  START ROW -4
  SetLights(x + 2, y - 4, 2, color1);
  SetLED(x + 4, y - 4, color2);
  SetLED(x + 5, y - 4, color3);
  SetLights(x + 6, y - 4, 2, color2);
  SetLED(x + 0, y - 3, color1); //  START ROW -3
  SetLights(x + 3, y - 3, 2, color1);
  SetLights(x + 5, y - 3, 3, color2);
  SetLED(x + 1, y - 2, color1); //  START ROW -2
  SetLED(x + 2, y - 2, color3);
  SetLED(x + 4, y - 2, color1);
  SetLights(x + 5, y - 2, 2, color2);
  SetLights(x + 2, y - 1, 5, color1); //  START ROW -1
  SetLights(x + 3, y + 0, 4, color1); //  START ROW +0
  SetLights(x + 2, y + 1, 5, color1); //  START ROW +1
  SetLights(x + 1, y + 2, 2, color1); //  START ROW +2
  SetLights(x + 4, y + 2, 3, color1);
  SetLights(x + 0, y + 3, 6, color1); //  START ROW +3
  SetLights(x - 1, y + 4, 7, color1); //  START ROW +4
  SetLED(x - 2, y + 5, color2); //  START ROW +5
  SetLights(x - 1, y + 5, 6, color1);
  SetLED(x - 3, y + 6, color2);  //  START ROW +6
  SetLights(x - 2, y + 6, 5, color1);
}

void MsPacManChompDanceThrough()
{
  Serial.println("MsPacManChompDanceThrough()");
  int frame = GetFrame(48, 120);
  
  if (frame % 8 == 0)       DrawMsPacManChomp01(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 1)  DrawMsPacManChomp01(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 2)  DrawMsPacManChomp02(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 3)  DrawMsPacManChomp02(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 4)  DrawMsPacManChomp03(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 5)  DrawMsPacManChomp03(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 6)  DrawMsPacManChomp02(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else                      DrawMsPacManChomp02(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
}

void DrawPacManGhostWalk01(int x, int y, byte bodyColor = 50, byte eyeWhite = 7, byte eyeBall = 0)
{
  SetLights(x - 2, y - 7, 4, bodyColor); // START ROW -7
  SetLights(x - 4, y - 6, 8, bodyColor); // START ROW -6
  SetLights(x - 5, y - 5, 10, bodyColor); // START ROW -5
  SetLights(x - 6, y - 4, 3, bodyColor); // START ROW -4
  SetLights(x - 3, y - 4, 2, eyeWhite);
  SetLights(x - 1, y - 4, 4, bodyColor);
  SetLights(x + 3, y - 4, 2, eyeWhite);
  SetLED(x + 5, y - 4, bodyColor);
  SetLights(x - 6, y - 3, 2, bodyColor); // START ROW -3
  SetLights(x - 4, y - 3, 4, eyeWhite);
  SetLights(x + 0, y - 3, 2, bodyColor);
  SetLights(x + 2, y - 3, 4, eyeWhite);
  SetLights(x - 6, y - 2, 2, bodyColor); // START ROW -2
  SetLights(x - 4, y - 2, 2, eyeWhite);
  SetLights(x - 2, y - 2, 2, eyeBall);
  SetLights(x + 0, y - 2, 2, bodyColor);
  SetLights(x + 2, y - 2, 2, eyeWhite);
  SetLights(x + 4, y - 2, 2, eyeBall);
  SetLights(x - 7, y - 1, 3, bodyColor); // START ROW -1
  SetLights(x - 4, y - 1, 2, eyeWhite);
  SetLights(x - 2, y - 1, 2, eyeBall);
  SetLights(x + 0, y - 1, 2, bodyColor);
  SetLights(x + 2, y - 1, 2, eyeWhite);
  SetLights(x + 4, y - 1, 2, eyeBall);
  SetLED(x + 6, y - 1, bodyColor);
  SetLights(x - 7, y + 0, 4, bodyColor); // START ROW +0
  SetLights(x - 3, y + 0, 2, eyeWhite);
  SetLights(x - 1, y + 0, 4, bodyColor);
  SetLights(x + 3, y + 0, 2, eyeWhite);
  SetLights(x + 5, y + 0, 2, bodyColor);
  SetLights(x - 7, y + 1, 14, bodyColor); // START ROW +1
  SetLights(x - 7, y + 2, 14, bodyColor); // START ROW +2
  SetLights(x - 7, y + 3, 14, bodyColor); // START ROW +3
  SetLights(x - 7, y + 4, 14, bodyColor); // START ROW +4
  SetLights(x - 7, y + 5, 4, bodyColor); // START ROW +5
  SetLights(x - 2, y + 5, 4, bodyColor);
  SetLights(x + 3, y + 5, 4, bodyColor);
  SetLights(x - 6, y + 6, 2, bodyColor); // START ROW +6
  SetLights(x - 1, y + 6, 2, bodyColor);
  SetLights(x + 4, y + 6, 2, bodyColor);
}

void DrawPacManGhostWalk02(int x, int y, byte bodyColor = 50, byte eyeWhite = 7, byte eyeBall = 0)
{
  SetLights(x - 2, y - 7, 4, bodyColor); // START ROW -7
  SetLights(x - 4, y - 6, 8, bodyColor); // START ROW -6
  SetLights(x - 5, y - 5, 10, bodyColor); // START ROW -5
  SetLights(x - 6, y - 4, 3, bodyColor); // START ROW -4
  SetLights(x - 3, y - 4, 2, eyeWhite);
  SetLights(x - 1, y - 4, 4, bodyColor);
  SetLights(x + 3, y - 4, 2, eyeWhite);
  SetLED(x + 5, y - 4, bodyColor);
  SetLights(x - 6, y - 3, 2, bodyColor); // START ROW -3
  SetLights(x - 4, y - 3, 4, eyeWhite);
  SetLights(x + 0, y - 3, 2, bodyColor);
  SetLights(x + 2, y - 3, 4, eyeWhite);
  SetLights(x - 6, y - 2, 2, bodyColor); // START ROW -2
  SetLights(x - 4, y - 2, 2, eyeWhite);
  SetLights(x - 2, y - 2, 2, eyeBall);
  SetLights(x + 0, y - 2, 2, bodyColor);
  SetLights(x + 2, y - 2, 2, eyeWhite);
  SetLights(x + 4, y - 2, 2, eyeBall);
  SetLights(x - 7, y - 1, 3, bodyColor); // START ROW -1
  SetLights(x - 4, y - 1, 2, eyeWhite);
  SetLights(x - 2, y - 1, 2, eyeBall);
  SetLights(x + 0, y - 1, 2, bodyColor);
  SetLights(x + 2, y - 1, 2, eyeWhite);
  SetLights(x + 4, y - 1, 2, eyeBall);
  SetLED(x + 6, y - 1, bodyColor);
  SetLights(x - 7, y + 0, 4, bodyColor); // START ROW +0
  SetLights(x - 3, y + 0, 2, eyeWhite);
  SetLights(x - 1, y + 0, 4, bodyColor);
  SetLights(x + 3, y + 0, 2, eyeWhite);
  SetLights(x + 5, y + 0, 2, bodyColor);
  SetLights(x - 7, y + 1, 14, bodyColor); // START ROW +1
  SetLights(x - 7, y + 2, 14, bodyColor); // START ROW +2
  SetLights(x - 7, y + 3, 14, bodyColor); // START ROW +3
  SetLights(x - 7, y + 4, 14, bodyColor); // START ROW +4
  SetLights(x - 7, y + 5, 2, bodyColor); // START ROW +5
  SetLights(x - 4, y + 5, 3, bodyColor);
  SetLights(x + 1, y + 5, 3, bodyColor);
  SetLights(x + 5, y + 5, 2, bodyColor);
  SetLED(x - 7, y + 6, bodyColor); // START ROW +6
  SetLights(x - 3, y + 6, 2, bodyColor);
  SetLights(x + 1, y + 6, 2, bodyColor);
  SetLED(x + 6, y + 6, bodyColor);
}

void PacManGhostDanceThrough(int x = 6, int y = 14, int frameLength = 48, int body = 50, int eyeWhite = 7, int eyeBall = 0, int animationTime = 100)
{
  Serial.println("PacManGhostDanceThrough()");
  int frame = GetFrame(frameLength, animationTime);
  
  if (frame % 4 == 0)       DrawPacManGhostWalk01(frame + x, y, body, eyeWhite, eyeBall);
  else if (frame % 4 == 1)  DrawPacManGhostWalk01(frame + x, y, body, eyeWhite, eyeBall);
  else if (frame % 4 == 2)  DrawPacManGhostWalk02(frame + x, y, body, eyeWhite, eyeBall);
  else if (frame % 4 == 3)  DrawPacManGhostWalk02(frame + x, y, body, eyeWhite, eyeBall);
}

void PacManChompDanceThroughPlusGhost()
{
  PacManChompDanceThrough(-6, 14, 120);
  PacManGhostDanceThrough(-26, 14, 120, 50, 7, 0);
  PacManGhostDanceThrough(-46, 14, 120, 51, 7, 0);
  PacManGhostDanceThrough(-66, 14, 120, 52, 7, 0);
  PacManGhostDanceThrough(-86, 14, 120, 53, 7, 0);
}

void DrawSpaceInvader01(int x, int y, byte color1 = 7, byte color2 = 7)
{
/*
 *   X     X
 *    X   X
 *   OOOOOOO 
 *  OO OOO OO
 * XXXXXXXXXXX
 * X XXXXXXX X
 * X X     X X
 *    XX XX
 */
 
  SetLED(x - 3, y - 4, color1); // START ROW -4
  SetLED(x + 3, y - 4, color1);
  SetLED(x - 2, y - 3, color1); // START ROW -3
  SetLED(x + 2, y - 3, color1);
  SetLights(x - 3, y - 2, 7, color2); // START ROW -2
  SetLights(x - 4, y - 1, 2, color2); //  START ROW -1
  SetLights(x - 1, y - 1, 3, color2);
  SetLights(x + 3, y - 1, 2, color2);
  SetLights(x - 5, y, 11, color1); //  START ROW +0
  SetLED(x - 5, y + 1, color1); //  START ROW +1
  SetLights(x - 3, y + 1, 7, color1);
  SetLED(x + 5, y + 1, color1);
  SetLED(x - 5, y + 2, color1); //  START ROW +2
  SetLED(x - 3, y + 2, color1);
  SetLED(x + 3, y + 2, color1);
  SetLED(x + 5, y + 2, color1);
  SetLights(x - 2, y + 3, 2, color1); //  START ROW +3
  SetLights(x + 1, y + 3, 2, color1);
}

void DrawSpaceInvader02(int x, int y, byte color1 = 7, byte color2 = 7)
{
/*
 *   X     X
 *    X   X
 * O OOOOOOO O
 * OOO OOO OOO
 * XXXXXXXXXXX
 *   XXXXXXX 
 *   X     X 
 *  X       X
 */
 
  SetLED(x - 3, y - 4, color1); // START ROW -4
  SetLED(x + 3, y - 4, color1);
  SetLED(x - 2, y - 3, color1); // START ROW -3
  SetLED(x + 2, y - 3, color1);
  SetLED(x - 5, y - 2, color2); // START ROW -2
  SetLights(x - 3, y - 2, 7, color2);
  SetLED(x + 5, y - 2, color2);
  SetLights(x - 5, y - 1, 3, color2); //  START ROW -1
  SetLights(x - 1, y - 1, 3, color2);
  SetLights(x + 3, y - 1, 3, color2);
  SetLights(x - 5, y, 11, color1); //  START ROW +0
  SetLights(x - 3, y + 1, 7, color1); //  START ROW +1
  SetLED(x - 3, y + 2, color1); //  START ROW +2
  SetLED(x + 3, y + 2, color1);
  SetLED(x - 4, y + 3, color1); //  START ROW +3
  SetLED(x + 4, y + 3, color1);
}

void SpaceInvaderDanceThrough(byte color1 = 7, byte color2 = 7)
{
  Serial.println("SpaceInvaderDanceThrough()");
  int frame = GetFrame(40);
  
  if (frame & 1)  DrawSpaceInvader01(frame - 6, 14, color1, color2);
  else            DrawSpaceInvader02(frame - 6, 14, color1, color2);
}

void ThreeSpaceInvaderDanceThrough(byte color1a = 7, byte color1b = 7, byte color2a = 7, byte color2b = 7, byte color3a = 7, byte color3b = 7)
{
  Serial.println("ThreeSpaceInvaderDanceThrough()");
  int frame = GetFrame(40);
  
  if (frame & 1)
  {
    DrawSpaceInvader01(PANEL_WIDTH - frame + 6, 4, color1a, color1b);
    DrawSpaceInvader01(frame - 6, 14, color2a, color2b);
    DrawSpaceInvader01(PANEL_WIDTH - frame + 6, 24, color3a, color3b);
  }
  else
  {
    DrawSpaceInvader02(PANEL_WIDTH - frame + 6, 4, color1a, color1b);
    DrawSpaceInvader02(frame - 6, 14, color2a, color2b);
    DrawSpaceInvader02(PANEL_WIDTH - frame + 6, 24, color3a, color3b);
  }
}

void DrawMegaManRun01(int x, int y, byte outline, byte body1, byte body2, byte skin, byte eyes)
{
/*
 *              OOO
 *            OOObbO
 *           OBBBObbO
 *          OBBBBBOOOO
 *          OBBBBBObbBO
 *         ObBBBBBBOOBO
 *         ObBBSEEEBBEO
 *       OOObBSEEOOSOEO
 *     OObbbOBSEEOOSOEO
 *    OBBBbbbOSSEEESESO
 *   OBBBBbbObbSOOOOSO   // <-- 0
 *   OBBBOOObbbbBSSSOOO
 *   OBBBO ObbbbbOOOBBBO
 *    OOO OBbOBbbBBBBBBO
 *   OBBOOOBBbOBbBBBBBBO
 *  OBBBBObBBBBOOOOOOOO
 * OBBBBBbbbBBBBbbbO
 * OBBOBBbbbOOObbbBBO
 *  OOOOBBbO   OBBBBO
 *      OOO    OBBBOOO
 *            OBBBBBBBO
 *            OOOOOOOOO
 */
 
  SetLights(x + 0, y - 10, 3, outline); // START ROW -10
  SetLights(x - 2, y - 9, 3, outline); // START ROW -9
  SetLights(x + 1, y - 9, 2, body2);
  SetLED(x + 3, y - 9, outline);
  SetLED(x - 3, y - 8, outline); // START ROW -8
  SetLights(x - 2, y - 8, 3, body1);
  SetLED(x + 1, y - 8, outline);
  SetLights(x + 2, y - 8, 2, body2);
  SetLED(x + 4, y - 8, outline);
  SetLED(x - 4, y - 7, outline); // START ROW -7
  SetLights(x - 3, y - 7, 5, body1);
  SetLights(x + 2, y - 7, 4, outline);
  SetLED(x - 4, y - 6, outline); // START ROW -6
  SetLights(x - 3, y - 6, 5, body1);
  SetLED(x + 2, y - 6, outline);
  SetLights(x + 3, y - 6, 2, body2);
  SetLED(x + 5, y - 6, body1);
  SetLED(x + 6, y - 6, outline);
  SetLED(x - 5, y - 5, outline); // START ROW -5
  SetLED(x - 4, y - 5, body2);
  SetLights(x - 3, y - 5, 6, body1);
  SetLights(x + 3, y - 5, 2, outline);
  SetLED(x + 5, y - 5, body1);
  SetLED(x + 6, y - 5, outline);
  SetLED(x - 5, y - 4, outline); // START ROW -4
  SetLED(x - 4, y - 4, body2);
  SetLights(x - 3, y - 4, 2, body1);
  SetLED(x - 1, y - 4, skin);
  SetLights(x + 0, y - 4, 3, eyes);
  SetLights(x + 3, y - 4, 2, body1);
  SetLED(x + 5, y - 4, eyes);
  SetLED(x + 6, y - 4, outline);
  SetLights(x - 7, y - 3, 3, outline); // START ROW -3
  SetLED(x - 4, y - 3, body2);
  SetLED(x - 3, y - 3, body1);
  SetLED(x - 2, y - 3, skin);
  SetLights(x - 1, y - 3, 2, eyes);
  SetLights(x + 1, y - 3, 2, outline);
  SetLED(x + 3, y - 3, skin);
  SetLED(x + 4, y - 3, outline);
  SetLED(x + 5, y - 3, eyes);
  SetLED(x + 6, y - 3, outline);
  SetLights(x - 9, y - 2, 2, outline); // START ROW -2
  SetLights(x - 7, y - 2, 3, body2);
  SetLED(x - 4, y - 2, outline);
  SetLED(x - 3, y - 2, body1);
  SetLED(x - 2, y - 2, skin);
  SetLights(x - 1, y - 2, 2, eyes);
  SetLights(x + 1, y - 2, 2, outline);
  SetLED(x + 3, y - 2, skin);
  SetLED(x + 4, y - 2, outline);
  SetLED(x + 5, y - 2, eyes);
  SetLED(x + 6, y - 2, outline);
  SetLED(x - 10, y - 1, outline); // START ROW -1
  SetLights(x - 9, y - 1, 3, body1);
  SetLights(x - 6, y - 1, 3, body2);
  SetLED(x - 3, y - 1, outline);
  SetLights(x - 2, y - 1, 2, skin);
  SetLights(x + 0, y - 1, 3, eyes);
  SetLED(x + 3, y - 1, skin);
  SetLED(x + 4, y - 1, eyes);
  SetLED(x + 5, y - 1, skin);
  SetLED(x + 6, y - 1, outline);
  SetLED(x - 11, y + 0, outline); // START ROW +0
  SetLights(x - 10, y + 0, 4, body1);
  SetLights(x - 6, y + 0, 2, body2);
  SetLED(x - 4, y + 0, outline);
  SetLights(x - 3, y + 0, 2, body2);
  SetLED(x - 1, y + 0, skin);
  SetLights(x + 0, y + 0, 4, outline);
  SetLED(x + 4, y + 0, skin);
  SetLED(x + 5, y + 0, outline);
  SetLED(x - 11, y + 1, outline); // START ROW +1
  SetLights(x - 10, y + 1, 3, body1);
  SetLights(x - 7, y + 1, 3, outline);
  SetLights(x - 4, y + 1, 4, body2);
  SetLED(x + 0, y + 1, body1);
  SetLights(x + 1, y + 1, 3, skin);
  SetLights(x + 4, y + 1, 3, outline);
  SetLED(x - 11, y + 2, outline); // START ROW +2
  SetLights(x - 10, y + 2, 3, body1);
  SetLED(x - 7, y + 2, outline);
  SetLED(x - 5, y + 2, outline);
  SetLights(x - 4, y + 2, 5, body2);
  SetLights(x + 1, y + 2, 3, outline);
  SetLights(x + 4, y + 2, 3, body1);
  SetLED(x + 7, y + 2, outline);
  SetLights(x - 10, y + 3, 3, outline); // START ROW +3
  SetLED(x - 6, y + 3, outline);
  SetLED(x - 5, y + 3, body1);
  SetLED(x - 4, y + 3, body2);
  SetLED(x - 3, y + 3, outline);
  SetLED(x - 2, y + 3, body1);
  SetLights(x - 1, y + 3, 2, body2);
  SetLights(x + 1, y + 3, 6, body1);
  SetLED(x + 7, y + 3, outline);
  SetLED(x - 11, y + 4, outline); // START ROW +4
  SetLights(x - 10, y + 4, 2, body1);
  SetLights(x - 8, y + 4, 3, outline);
  SetLights(x - 5, y + 4, 2, body1);
  SetLED(x - 3, y + 4, body2);
  SetLED(x - 2, y + 4, outline);
  SetLED(x - 1, y + 4, body1);
  SetLED(x + 0, y + 4, body2);
  SetLights(x + 1, y + 4, 6, body1);
  SetLED(x + 7, y + 4, outline);
  SetLED(x - 12, y + 5, outline); // START ROW +5
  SetLights(x - 11, y + 5, 4, body1);
  SetLED(x - 7, y + 5, outline);
  SetLED(x - 6, y + 5, body2);
  SetLights(x - 5, y + 5, 4, body1);
  SetLights(x - 1, y + 5, 8, outline);
  SetLED(x - 13, y + 6, outline); // START ROW +6
  SetLights(x - 12, y + 6, 5, body1);
  SetLights(x - 7, y + 6, 3, body2);
  SetLights(x - 4, y + 6, 4, body1);
  SetLights(x + 0, y + 6, 3, body2);
  SetLED(x + 3, y + 6, outline);
  SetLED(x - 13, y + 7, outline); // START ROW +7
  SetLights(x - 12, y + 7, 2, body1);
  SetLED(x - 10, y + 7, outline);
  SetLights(x - 9, y + 7, 2, body1);
  SetLights(x - 7, y + 7, 3, body2);
  SetLights(x - 4, y + 7, 3, outline);
  SetLights(x - 1, y + 7, 3, body2);
  SetLights(x + 2, y + 7, 2, body1);
  SetLED(x + 4, y + 7, outline);
  SetLights(x - 12, y + 8, 4, outline); // START ROW +8
  SetLights(x - 8, y + 8, 2, body1);
  SetLED(x - 6, y + 8, body2);
  SetLED(x - 5, y + 8, outline);
  SetLED(x - 1, y + 8, outline);
  SetLights(x + 0, y + 8, 4, body1);
  SetLED(x + 4, y + 8, outline);
  SetLights(x - 8, y + 9, 3, outline); // START ROW +9
  SetLED(x - 1, y + 9, outline);
  SetLights(x + 0, y + 9, 3, body1);
  SetLights(x + 3, y + 9, 3, outline);
  SetLED(x - 2, y + 10, outline); // START ROW +10
  SetLights(x - 1, y + 10, 7, body1);
  SetLED(x + 6, y + 10, outline);
  SetLights(x - 2, y + 11, 9, outline); // START ROW +11
}

void DrawMegaManRun02(int x, int y, byte outline, byte body1, byte body2, byte skin, byte eyes)
{
  SetLights(x + 1, y - 11, 3, outline); // START ROW -11
  SetLights(x - 1, y - 10, 3, outline); // START ROW -10
  SetLights(x + 2, y - 10, 2, body2);
  SetLED(x + 4, y - 10, outline);
  SetLED(x - 2, y - 9, outline); // START ROW -9
  SetLights(x - 1, y - 9, 3, body1);
  SetLED(x + 2, y - 9, outline);
  SetLights(x + 3, y - 9, 2, body2);
  SetLED(x + 5, y - 9, outline);
  SetLED(x - 3, y - 8, outline); // START ROW -8
  SetLights(x - 2, y - 8, 5, body1);
  SetLights(x + 3, y - 8, 4, outline);
  SetLED(x - 3, y - 7, outline); // START ROW -7
  SetLights(x - 2, y - 7, 5, body1);
  SetLED(x + 3, y - 7, outline);
  SetLights(x + 4, y - 7, 2, body2);
  SetLED(x + 6, y - 7, body1);
  SetLED(x + 7, y - 7, outline);
  SetLED(x - 4, y - 6, outline); // START ROW -6
  SetLED(x - 3, y - 6, body2);
  SetLights(x - 2, y - 6, 6, body1);
  SetLights(x + 4, y - 6, 2, outline);
  SetLED(x + 6, y - 6, body1);
  SetLED(x + 7, y - 6, outline);
  SetLED(x - 4, y - 5, outline); // START ROW -5
  SetLED(x - 3, y - 5, body2);
  SetLights(x - 2, y - 5, 2, body1);
  SetLED(x + 0, y - 5, skin);
  SetLights(x + 1, y - 5, 3, eyes);
  SetLights(x + 4, y - 5, 2, body1);
  SetLED(x + 6, y - 5, eyes);
  SetLED(x + 7, y - 5, outline);
  SetLights(x - 5, y - 4, 2, outline); // START ROW -4
  SetLED(x - 3, y - 4, body2);
  SetLED(x - 2, y - 4, body1);
  SetLED(x - 1, y - 4, skin);
  SetLights(x + 0, y - 4, 2, eyes);
  SetLights(x + 2, y - 4, 2, outline);
  SetLED(x + 4, y - 4, skin);
  SetLED(x + 5, y - 4, outline);
  SetLED(x + 6, y - 4, eyes);
  SetLED(x + 7, y - 4, outline);
  SetLED(x - 6, y - 3, outline); // START ROW -3
  SetLights(x - 5, y - 3, 2, body2);
  SetLED(x - 3, y - 3, outline);
  SetLED(x - 2, y - 3, body1);
  SetLED(x - 1, y - 3, skin);
  SetLights(x + 0, y - 3, 2, eyes);
  SetLights(x + 2, y - 3, 2, outline);
  SetLED(x + 4, y - 3, skin);
  SetLED(x + 5, y - 3, outline);
  SetLED(x + 6, y - 3, eyes);
  SetLED(x + 7, y - 3, outline);
  SetLED(x - 7, y - 2, outline); // START ROW -2
  SetLights(x - 6, y - 2, 3, body2);
  SetLED(x - 3, y - 2, outline);
  SetLED(x - 2, y - 2, body1);
  SetLights(x - 1, y - 2, 2, skin);
  SetLights(x + 1, y - 2, 3, eyes);
  SetLED(x + 4, y - 2, skin);
  SetLED(x + 5, y - 2, eyes);
  SetLED(x + 6, y - 2, skin);
  SetLED(x + 7, y - 2, outline);
  SetLED(x - 8, y - 1, outline); // START ROW -1
  SetLights(x - 7, y - 1, 5, body2);
  SetLED(x - 2, y - 1, outline);
  SetLED(x - 1, y - 1, body1);
  SetLED(x + 0, y - 1, skin);
  SetLights(x + 1, y - 1, 4, outline);
  SetLED(x + 5, y - 1, skin);
  SetLED(x + 6, y - 1, outline);
  SetLED(x - 8, y + 0, outline); // START ROW +0
  SetLights(x - 7, y + 0, 3, body2);
  SetLED(x - 4, y + 0, outline);
  SetLights(x - 3, y + 0, 2, body2);
  SetLED(x - 1, y + 0, outline);
  SetLights(x + 0, y + 0, 5, skin);
  SetLED(x + 5, y + 0, outline);
  SetLED(x - 7, y + 1, outline); // START ROW +1
  SetLights(x - 6, y + 1, 3, body1);
  SetLED(x - 3, y + 1, outline);
  SetLights(x - 2, y + 1, 2, body2);
  SetLights(x + 0, y + 1, 6, outline);
  SetLED(x - 7, y + 2, outline); // START ROW +2
  SetLights(x - 6, y + 2, 4, body1);
  SetLights(x - 2, y + 2, 2, outline);
  SetLights(x + 0, y + 2, 4, body2);
  SetLED(x + 4, y + 2, outline);
  SetLED(x + 5, y + 2, body1);
  SetLED(x + 6, y + 2, outline);
  SetLED(x - 6, y + 3, outline); // START ROW +3
  SetLights(x - 5, y + 3, 2, body1);
  SetLED(x - 3, y + 3, outline);
  SetLights(x - 2, y + 3, 2, body1);
  SetLED(x + 0, y + 3, outline);
  SetLights(x + 1, y + 3, 2, body2);
  SetLED(x + 3, y + 3, outline);
  SetLights(x + 4, y + 3, 2, body1);
  SetLED(x + 6, y + 3, outline);
  SetLED(x - 5, y + 4, outline); // START ROW +4
  SetLights(x - 4, y + 4, 4, body1);
  SetLED(x + 0, y + 4, outline);
  SetLED(x + 1, y + 4, body1);
  SetLights(x + 2, y + 4, 4, outline);
  SetLED(x - 4, y + 5, outline); // START ROW +5
  SetLights(x - 3, y + 5, 2, body1);
  SetLED(x - 1, y + 5, outline);
  SetLED(x + 0, y + 5, body1);
  SetLED(x + 1, y + 5, outline);
  SetLED(x + 2, y + 5, body2);
  SetLED(x + 3, y + 5, outline);
  SetLights(x - 3, y + 6, 2, outline); // START ROW +6
  SetLights(x - 1, y + 6, 2, body2);
  SetLED(x + 1, y + 6, outline);
  SetLED(x + 2, y + 6, body2);
  SetLED(x + 3, y + 6, outline);
  SetLights(x - 4, y + 7, 2, outline); // START ROW +7
  SetLights(x - 2, y + 7, 3, body2);
  SetLights(x + 1, y + 7, 2, outline);
  SetLED(x - 5, y + 8, outline); // START ROW +8
  SetLights(x - 4, y + 8, 5, body1);
  SetLED(x + 1, y + 8, outline);
  SetLED(x - 6, y + 9, outline); // START ROW +9
  SetLights(x - 5, y + 9, 5, body1);
  SetLED(x + 0, y + 9, outline);
  SetLED(x - 6, y + 10, outline); // START ROW +10
  SetLights(x - 5, y + 10, 3, body1);
  SetLights(x - 2, y + 10, 3, outline);
  SetLED(x - 5, y + 11, outline); // START ROW +11
  SetLights(x - 4, y + 11, 5, body1);
  SetLED(x + 1, y + 11, outline);
  SetLights(x - 4, y + 12, 6, outline); // START ROW +12
}

void DrawMegaManRun03(int x, int y, byte outline, byte body1, byte body2, byte skin, byte eyes)
{
  SetLights(x + 1, y - 10, 3, outline); // START ROW -10
  SetLights(x - 1, y - 9, 3, outline); // START ROW -9
  SetLights(x + 2, y - 9, 2, body2);
  SetLED(x + 4, y - 9, outline);
  SetLED(x - 2, y - 8, outline); // START ROW -8
  SetLights(x - 1, y - 8, 3, body1);
  SetLED(x + 2, y - 8, outline);
  SetLights(x + 3, y - 8, 2, body2);
  SetLED(x + 5, y - 8, outline);
  SetLED(x - 3, y - 7, outline); // START ROW -7
  SetLights(x - 2, y - 7, 5, body1);
  SetLights(x + 3, y - 7, 4, outline);
  SetLED(x - 3, y - 6, outline); // START ROW -6
  SetLights(x - 2, y - 6, 5, body1);
  SetLED(x + 3, y - 6, outline);
  SetLights(x + 4, y - 6, 2, body2);
  SetLED(x + 6, y - 6, eyes);
  SetLED(x + 7, y - 6, outline);
  SetLights(x - 6, y - 5, 3, outline); // START ROW -5
  SetLED(x - 3, y - 5, body2);
  SetLights(x - 2, y - 5, 6, body1);
  SetLights(x + 4, y - 5, 2, outline);
  SetLED(x + 6, y - 5, eyes);
  SetLED(x + 7, y - 5, outline);
  SetLights(x - 8, y - 4, 2, outline); // START ROW -4
  SetLights(x - 6, y - 4, 2, body2);
  SetLED(x - 4, y - 4, outline);
  SetLED(x - 3, y - 4, body2);
  SetLights(x - 2, y - 4, 2, body1);
  SetLED(x + 0, y - 4, skin);
  SetLights(x + 1, y - 4, 3, eyes);
  SetLights(x + 4, y - 4, 2, body1);
  SetLED(x + 6, y - 4, eyes);
  SetLED(x + 7, y - 4, outline);
  SetLED(x - 9, y - 3, outline); // START ROW -3
  SetLED(x - 8, y - 3, body1);
  SetLED(x - 7, y - 3, outline);
  SetLights(x - 6, y - 3, 2, body2);
  SetLED(x - 4, y - 3, outline);
  SetLED(x - 3, y - 3, body2);
  SetLED(x - 2, y - 3, body1);
  SetLED(x - 1, y - 3, skin);
  SetLights(x + 0, y - 3, 2, eyes);
  SetLights(x + 2, y - 3, 2, outline);
  SetLED(x + 4, y - 3, skin);
  SetLED(x + 5, y - 3, outline);
  SetLED(x + 6, y - 3, eyes);
  SetLED(x + 7, y - 3, outline);
  SetLED(x - 10, y - 2, outline); // START ROW -2
  SetLights(x - 9, y - 2, 3, body1);
  SetLED(x - 6, y - 2, outline);
  SetLights(x - 5, y - 2, 2, body2);
  SetLED(x - 3, y - 2, outline);
  SetLED(x - 2, y - 2, body1);
  SetLED(x - 1, y - 2, skin);
  SetLights(x + 0, y - 2, 2, eyes);
  SetLights(x + 2, y - 2, 2, outline);
  SetLED(x + 4, y - 2, skin);
  SetLED(x + 5, y - 2, outline);
  SetLED(x + 6, y - 2, eyes);
  SetLED(x + 7, y - 2, outline);
  SetLights(x + 9, y - 2, 3, outline);
  SetLED(x - 11, y - 1, outline); // START ROW -1
  SetLights(x - 10, y - 1, 4, body1);
  SetLED(x - 6, y - 1, outline);
  SetLights(x - 5, y - 1, 2, body2);
  SetLED(x - 3, y - 1, outline);
  SetLED(x - 2, y - 1, body1);
  SetLights(x - 1, y - 1, 2, skin);
  SetLights(x + 1, y - 1, 3, eyes);
  SetLED(x + 4, y - 1, skin);
  SetLED(x + 5, y - 1, eyes);
  SetLED(x + 6, y - 1, skin);
  SetLights(x + 7, y - 1, 2, outline);
  SetLights(x + 9, y - 1, 3, body1);
  SetLED(x + 12, y - 1, outline);
  SetLED(x - 11, y + 0, outline); // START ROW +0
  SetLights(x - 10, y + 0, 3, body1);
  SetLED(x - 7, y + 0, outline);
  SetLED(x - 5, y + 0, outline);
  SetLights(x - 4, y + 0, 2, body2);
  SetLED(x - 2, y + 0, outline);
  SetLED(x - 1, y + 0, body1);
  SetLED(x + 0, y + 0, skin);
  SetLights(x + 1, y + 0, 4, outline);
  SetLED(x + 5, y + 0, skin);
  SetLED(x + 6, y + 0, outline);
  SetLED(x + 7, y + 0, eyes);
  SetLED(x + 8, y + 0, outline);
  SetLights(x + 9, y + 0, 3, body1);
  SetLED(x + 12, y + 0, outline);
  SetLED(x - 11, y + 1, outline); // START ROW +1
  SetLights(x - 10, y + 1, 3, body1);
  SetLED(x - 7, y + 1, outline);
  SetLED(x - 5, y + 1, outline);
  SetLights(x - 4, y + 1, 3, body2);
  SetLED(x - 1, y + 1, outline);
  SetLights(x + 0, y + 1, 5, skin);
  SetLED(x + 5, y + 1, outline);
  SetLED(x + 6, y + 1, body2);
  SetLights(x + 7, y + 1, 2, outline);
  SetLights(x + 9, y + 1, 3, body1);
  SetLED(x + 12, y + 1, outline);
  SetLights(x - 10, y + 2, 3, outline); // START ROW +2
  SetLED(x - 5, y + 2, outline);
  SetLED(x - 4, y + 2, body1);
  SetLights(x - 3, y + 2, 3, body2);
  SetLights(x + 0, y + 2, 5, outline);
  SetLights(x + 5, y + 2, 3, body2);
  SetLights(x + 8, y + 2, 3, body1);
  SetLED(x + 11, y + 2, outline);
  SetLights(x - 8, y + 3, 2, outline); // START ROW +3
  SetLED(x - 5, y + 3, outline);
  SetLights(x - 4, y + 3, 3, body1);
  SetLights(x - 1, y + 3, 5, body2);
  SetLED(x + 4, y + 3, outline);
  SetLights(x + 5, y + 3, 2, body2);
  SetLights(x + 7, y + 3, 3, body1);
  SetLED(x + 10, y + 3, outline);
  SetLED(x - 9, y + 4, outline); // START ROW +4
  SetLights(x - 8, y + 4, 2, body1);
  SetLights(x - 6, y + 4, 2, outline);
  SetLights(x - 4, y + 4, 6, body1);
  SetLED(x + 2, y + 4, body2);
  SetLED(x + 3, y + 4, outline);
  SetLED(x + 5, y + 4, outline);
  SetLED(x + 6, y + 4, body2);
  SetLights(x + 7, y + 4, 2, body1);
  SetLED(x + 9, y + 4, outline);
  SetLED(x - 10, y + 5, outline); // START ROW +5
  SetLights(x - 9, y + 5, 4, body1);
  SetLights(x - 5, y + 5, 2, outline);
  SetLights(x - 3, y + 5, 2, body1);
  SetLights(x - 1, y + 5, 3, body2);
  SetLED(x + 2, y + 5, outline);
  SetLights(x + 6, y + 5, 3, outline);
  SetLED(x - 11, y + 6, outline); // START ROW +6
  SetLights(x - 10, y + 6, 5, body1);
  SetLED(x - 5, y + 6, body2);
  SetLights(x - 4, y + 6, 2, outline);
  SetLED(x - 2, y + 6, body1);
  SetLights(x - 1, y + 6, 4, body2);
  SetLED(x + 3, y + 6, outline);
  SetLED(x - 11, y + 7, outline); // START ROW +7
  SetLights(x - 10, y + 7, 2, body1);
  SetLED(x - 8, y + 7, outline);
  SetLights(x - 7, y + 7, 2, body1);
  SetLights(x - 5, y + 7, 3, body2);
  SetLights(x - 2, y + 7, 2, outline);
  SetLights(x + 0, y + 7, 3, body2);
  SetLED(x + 3, y + 7, body1);
  SetLED(x + 4, y + 7, outline);
  SetLights(x - 10, y + 8, 4, outline); // START ROW +8
  SetLED(x - 6, y + 8, body1);
  SetLights(x - 5, y + 8, 2, body2);
  SetLED(x - 3, y + 8, outline);
  SetLED(x - 1, y + 8, outline);
  SetLights(x + 0, y + 8, 4, body1);
  SetLED(x + 4, y + 8, outline);
  SetLights(x - 6, y + 9, 3, outline); // START ROW +9
  SetLED(x - 1, y + 9, outline);
  SetLights(x + 0, y + 9, 3, body1);
  SetLights(x + 3, y + 9, 3, outline);
  SetLED(x - 2, y + 10, outline); // START ROW +10
  SetLights(x - 1, y + 10, 7, body1);
  SetLED(x + 6, y + 10, outline);
  SetLights(x - 2, y + 11, 9, outline); // START ROW +11
}

void MegaManRunThrough()
{
  Serial.println("MegaManRunThrough()");
  int frame = GetFrame(52, 90);

  SetStrip(CRGB(GetColor(8, 0), GetColor(8, 1), GetColor(8, 2)));
  
  if (frame % 8 == 0)       DrawMegaManRun01(frame - 14, 14, 0, 9, 10, 11, 7);
  else if (frame % 8 == 1)  DrawMegaManRun01(frame - 14, 14, 0, 9, 10, 11, 7);
  else if (frame % 8 == 2)  DrawMegaManRun02(frame - 14, 14, 0, 9, 10, 11, 7);
  else if (frame % 8 == 3)  DrawMegaManRun02(frame - 14, 14, 0, 9, 10, 11, 7);
  else if (frame % 8 == 4)  DrawMegaManRun03(frame - 14, 14, 0, 9, 10, 11, 7);
  else if (frame % 8 == 5)  DrawMegaManRun03(frame - 14, 14, 0, 9, 10, 11, 7);
  else if (frame % 8 == 6)  DrawMegaManRun02(frame - 14, 14, 0, 9, 10, 11, 7);
  else                      DrawMegaManRun02(frame - 14, 14, 0, 9, 10, 11, 7);
}

void DrawMarioWarp01(int x, int y, byte outline = 0, byte skintone = 12, byte outfit = 34)
{
  SetLights(x - 4, y - 13, 3, outline); // START ROW -13
  SetLights(x + 5, y - 13, 2, outline);
  SetLED(x - 4, y - 12, outline); // START ROW -12
  SetLights(x - 3, y - 12, 2, skintone);
  SetLED(x - 1, y - 12, outline);
  SetLights(x + 1, y - 12, 4, outline);
  SetLED(x + 5, y - 12, skintone);
  SetLED(x + 6, y - 12, outline);
  SetLED(x - 4, y - 11, outline); // START ROW -11
  SetLights(x - 3, y - 11, 3, skintone);
  SetLED(x + 0, y - 11, outline);
  SetLights(x + 1, y - 11, 4, outfit);
  SetLED(x + 5, y - 11, skintone);
  SetLED(x + 6, y - 11, outline);
  SetLED(x - 4, y - 10, outline); // START ROW -10
  SetLights(x - 3, y - 10, 3, skintone);
  SetLights(x + 0, y - 10, 5, outfit);
  SetLED(x + 5, y - 10, outline);
  SetLED(x - 4, y - 9, outline); // START ROW -9
  SetLights(x - 3, y - 9, 2, skintone);
  SetLights(x - 1, y - 9, 3, outfit);
  SetLights(x + 2, y - 9, 6, outline);
  SetLED(x - 4, y - 8, outline); // START ROW -8
  SetLights(x - 3, y - 8, 3, outfit);
  SetLights(x + 0, y - 8, 9, outline);
  SetLED(x - 4, y - 7, outline); // START ROW -7
  SetLED(x - 3, y - 7, outfit);
  SetLights(x - 2, y - 7, 2, outline);
  SetLights(x + 0, y - 7, 6, skintone);
  SetLights(x + 6, y - 7, 2, outline);
  SetLights(x - 5, y - 6, 4, outline); // START ROW -6
  SetLights(x - 1, y - 6, 3, skintone);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone);
  SetLED(x + 6, y - 6, outline);
  SetLED(x - 5, y - 5, outline); // START ROW -5
  SetLED(x - 4, y - 5, skintone);
  SetLights(x - 3, y - 5, 2, outline);
  SetLights(x - 1, y - 5, 3, skintone);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone);
  SetLED(x + 6, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLights(x - 5, y - 4, 2, skintone);
  SetLights(x - 3, y - 4, 3, outline);
  SetLights(x + 0, y - 4, 7, skintone);
  SetLED(x + 7, y - 4, outline);
  SetLED(x - 6, y - 3, outline); // START ROW -3
  SetLights(x - 5, y - 3, 3, skintone);
  SetLED(x - 2, y - 3, outline);
  SetLights(x - 1, y - 3, 2, skintone);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 5, skintone);
  SetLED(x + 7, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLights(x - 4, y - 2, 4, skintone);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone);
  SetLights(x + 6, y - 2, 3, outline);
  SetLights(x - 4, y - 1, 2, outline); // START ROW -1
  SetLights(x - 2, y - 1, 4, skintone);
  SetLights(x + 2, y - 1, 5, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone);
  SetLED(x + 5, y + 0, outline);
  SetLights(x - 5, y + 1, 2, outline); // START ROW +1
  SetLights(x - 3, y + 1, 2, outfit);
  SetLights(x - 1, y + 1, 10, outline);
  SetLights(x - 8, y + 2, 4, outline); // START ROW +2
  SetLights(x - 4, y + 2, 4, outfit);
  SetLights(x + 0, y + 2, 2, outline);
  SetLights(x + 2, y + 2, 2, outfit);
  SetLights(x + 4, y + 2, 2, outline);
  SetLights(x + 6, y + 2, 3, skintone);
  SetLED(x + 9, y + 2, outline);
  SetLED(x - 9, y + 3, outline); // START ROW +3
  SetLights(x - 8, y + 3, 4, skintone);
  SetLED(x - 4, y + 3, outline);
  SetLights(x - 3, y + 3, 4, outfit);
  SetLights(x + 1, y + 3, 2, outline);
  SetLights(x + 3, y + 3, 2, outfit);
  SetLights(x + 5, y + 3, 2, outline);
  SetLights(x + 7, y + 3, 2, skintone);
  SetLED(x + 9, y + 3, outline);
  SetLED(x - 9, y + 4, outline); // START ROW +4
  SetLights(x - 8, y + 4, 5, skintone);
  SetLED(x - 3, y + 4, outline);
  SetLights(x - 2, y + 4, 3, outfit);
  SetLights(x + 1, y + 4, 2, outline);
  SetLights(x + 3, y + 4, 3, outfit);
  SetLights(x + 6, y + 4, 3, outline);
  SetLights(x - 13, y + 5, 8, outline); // START ROW +5
  SetLights(x - 5, y + 5, 2, skintone);
  SetLED(x - 3, y + 5, outline);
  SetLED(x - 2, y + 5, outfit);
  SetLights(x - 1, y + 5, 9, outline);
  SetLights(x - 14, y + 6, 2, outline); // START ROW +6
  SetLED(x - 12, y + 6, outfit);
  SetLights(x - 11, y + 6, 2, skintone);
  SetLights(x - 9, y + 6, 2, outfit);
  SetLED(x - 7, y + 6, skintone);
  SetLights(x - 6, y + 6, 8, outline);
  SetLights(x + 2, y + 6, 2, skintone);
  SetLights(x + 4, y + 6, 2, outline);
  SetLED(x + 6, y + 6, skintone);
  SetLED(x + 7, y + 6, outline);
  SetLED(x - 14, y + 7, outline); // START ROW +7
  SetLights(x - 13, y + 7, 2, outfit);
  SetLights(x - 11, y + 7, 2, skintone);
  SetLights(x - 9, y + 7, 2, outfit);
  SetLights(x - 7, y + 7, 9, outline);
  SetLights(x + 2, y + 7, 2, skintone);
  SetLights(x + 4, y + 7, 2, outline);
  SetLED(x + 6, y + 7, skintone);
  SetLED(x + 7, y + 7, outline);
  SetLED(x - 14, y + 8, outline); // START ROW +8
  SetLights(x - 13, y + 8, 2, outfit);
  SetLights(x - 11, y + 8, 2, skintone);
  SetLED(x - 9, y + 8, outfit);
  SetLED(x - 8, y + 8, outline);
  SetLights(x - 7, y + 8, 2, outfit);
  SetLights(x - 5, y + 8, 12, outline);
  SetLights(x - 14, y + 9, 2, outline); // START ROW +9
  SetLED(x - 12, y + 9, outfit);
  SetLights(x - 11, y + 9, 2, skintone);
  SetLED(x - 9, y + 9, outfit);
  SetLED(x - 8, y + 9, outline);
  SetLED(x - 7, y + 9, outfit);
  SetLights(x - 6, y + 9, 13, outline);
  SetLights(x - 13, y + 10, 5, outline); // START ROW +10
  SetLights(x - 8, y + 10, 2, outfit);
  SetLights(x - 6, y + 10, 12, outline);
  SetLED(x - 9, y + 11, outline); // START ROW +11
  SetLights(x - 8, y + 11, 3, outfit);
  SetLights(x - 5, y + 11, 9, outline);
  SetLED(x - 9, y + 12, outline); // START ROW +12
  SetLights(x - 8, y + 12, 2, outfit);
  SetLED(x - 6, y + 12, outline);
  SetLights(x - 5, y + 12, 2, outfit);
  SetLights(x - 3, y + 12, 4, outline);
  SetLights(x - 8, y + 13, 2, outline); // START ROW +13
  SetLights(x - 5, y + 13, 2, outline);
}

void DrawMarioWarp02(int x, int y, byte outline = 0, byte skintone1 = 15, byte raccoon1 = 17, byte skintone2 = 14, byte red3 = 27, byte raccoon2 = 18, byte red2 = 26, byte red1 = 25, byte skintone3 = 21, byte overalls2 = 23, byte overalls1 = 22, byte raccoon3 = 7, byte raccoon4 = 24, byte raccoon5 = 19, byte goldshoes = 20)
{
  SetLights(x - 4, y - 13, 3, outline); // START ROW -13
  SetLights(x + 5, y - 13, 2, outline);
  SetLED(x - 4, y - 12, outline); // START ROW -12
  SetLED(x - 3, y - 12, skintone1);
  SetLED(x - 2, y - 12, raccoon1);
  SetLED(x - 1, y - 12, outline);
  SetLights(x + 1, y - 12, 4, outline);
  SetLED(x + 5, y - 12, raccoon1);
  SetLED(x + 6, y - 12, outline);
  SetLED(x - 4, y - 11, outline); // START ROW -11
  SetLED(x - 3, y - 11, skintone2);
  SetLED(x - 2, y - 11, skintone1);
  SetLED(x - 1, y - 11, raccoon1);
  SetLED(x + 0, y - 11, outline);
  SetLights(x + 1, y - 11, 3, red3);
  SetLED(x + 4, y - 11, outline);
  SetLED(x + 5, y - 11, raccoon2);
  SetLED(x + 6, y - 11, outline);
  SetLED(x - 4, y - 10, outline); // START ROW -10
  SetLED(x - 3, y - 10, skintone2);
  SetLED(x - 2, y - 10, skintone1);
  SetLED(x - 1, y - 10, raccoon2);
  SetLights(x + 0, y - 10, 3, red2);
  SetLights(x + 3, y - 10, 2, red3);
  SetLED(x + 5, y - 10, outline);
  SetLED(x - 4, y - 9, outline); // START ROW -9
  SetLights(x - 3, y - 9, 2, skintone1);
  SetLights(x - 1, y - 9, 2, red2);
  SetLights(x + 1, y - 9, 7, outline);
  SetLED(x - 4, y - 8, outline); // START ROW -8
  SetLights(x - 3, y - 8, 3, red1);
  SetLights(x + 0, y - 8, 9, outline);
  SetLED(x - 4, y - 7, outline); // START ROW -7
  SetLED(x - 3, y - 7, red1);
  SetLights(x - 2, y - 7, 2, outline);
  SetLights(x + 0, y - 7, 6, skintone1);
  SetLights(x + 6, y - 7, 2, outline);
  SetLights(x - 5, y - 6, 4, outline); // START ROW -6
  SetLED(x - 1, y - 6, skintone1);
  SetLights(x + 0, y - 6, 2, skintone2);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone2);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone2);
  SetLED(x + 6, y - 6, outline);
  SetLED(x - 5, y - 5, outline); // START ROW -5
  SetLED(x - 4, y - 5, skintone2);
  SetLights(x - 3, y - 5, 2, outline);
  SetLED(x - 1, y - 5, skintone1);
  SetLights(x + 0, y - 5, 2, skintone2);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone2);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone2);
  SetLED(x + 6, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLights(x - 5, y - 4, 2, skintone2);
  SetLights(x - 3, y - 4, 3, outline);
  SetLED(x + 0, y - 4, skintone1);
  SetLights(x + 1, y - 4, 3, skintone2);
  SetLights(x + 4, y - 4, 3, skintone3);
  SetLED(x + 7, y - 4, outline);
  SetLED(x - 6, y - 3, outline); // START ROW -3
  SetLED(x - 5, y - 3, skintone1);
  SetLED(x - 4, y - 3, skintone2);
  SetLED(x - 3, y - 3, skintone1);
  SetLED(x - 2, y - 3, outline);
  SetLED(x - 1, y - 3, skintone1);
  SetLED(x + 0, y - 3, skintone2);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 2, skintone1);
  SetLights(x + 4, y - 3, 3, skintone2);
  SetLED(x + 7, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLights(x - 4, y - 2, 3, skintone1);
  SetLED(x - 1, y - 2, skintone2);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone1);
  SetLights(x + 6, y - 2, 3, outline);
  SetLights(x - 4, y - 1, 2, outline); // START ROW -1
  SetLights(x - 2, y - 1, 2, skintone1);
  SetLights(x + 0, y - 1, 2, skintone2);
  SetLights(x + 2, y - 1, 5, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone1);
  SetLED(x + 5, y + 0, outline);
  SetLights(x - 5, y + 1, 2, outline); // START ROW +1
  SetLights(x - 3, y + 1, 2, red2);
  SetLights(x - 1, y + 1, 10, outline);
  SetLights(x - 8, y + 2, 4, outline); // START ROW +2
  SetLED(x - 4, y + 2, red2);
  SetLights(x - 3, y + 2, 2, red3);
  SetLED(x - 1, y + 2, red2);
  SetLED(x + 0, y + 2, outline);
  SetLED(x + 1, y + 2, overalls2);
  SetLED(x + 2, y + 2, outline);
  SetLED(x + 3, y + 2, red2);
  SetLights(x + 4, y + 2, 2, outline);
  SetLED(x + 6, y + 2, skintone1);
  SetLights(x + 7, y + 2, 2, skintone2);
  SetLED(x + 9, y + 2, outline);
  SetLED(x - 9, y + 3, outline); // START ROW +3
  SetLights(x - 8, y + 3, 2, skintone3);
  SetLights(x - 6, y + 3, 2, skintone2);
  SetLED(x - 4, y + 3, outline);
  SetLights(x - 3, y + 3, 3, red2);
  SetLED(x + 0, y + 3, red1);
  SetLED(x + 1, y + 3, outline);
  SetLED(x + 2, y + 3, overalls2);
  SetLED(x + 3, y + 3, outline);
  SetLED(x + 4, y + 3, red2);
  SetLights(x + 5, y + 3, 2, outline);
  SetLights(x + 7, y + 3, 2, skintone1);
  SetLED(x + 9, y + 3, outline);
  SetLED(x - 9, y + 4, outline); // START ROW +4
  SetLights(x - 8, y + 4, 3, skintone1);
  SetLights(x - 5, y + 4, 2, skintone2);
  SetLED(x - 3, y + 4, outline);
  SetLights(x - 2, y + 4, 3, red1);
  SetLED(x + 1, y + 4, outline);
  SetLED(x + 2, y + 4, overalls2);
  SetLED(x + 3, y + 4, outline);
  SetLights(x + 4, y + 4, 2, red1);
  SetLights(x + 6, y + 4, 3, outline);
  SetLights(x - 13, y + 5, 8, outline); // START ROW +5
  SetLights(x - 5, y + 5, 2, skintone1);
  SetLED(x - 3, y + 5, outline);
  SetLED(x - 2, y + 5, red1);
  SetLights(x - 1, y + 5, 2, outline);
  SetLED(x + 1, y + 5, overalls1);
  SetLED(x + 2, y + 5, overalls2);
  SetLights(x + 3, y + 5, 3, outline);
  SetLED(x + 6, y + 5, overalls2);
  SetLED(x + 7, y + 5, outline);
  SetLights(x - 14, y + 6, 2, outline); // START ROW +6
  SetLED(x - 12, y + 6, raccoon1);
  SetLights(x - 11, y + 6, 2, skintone3);
  SetLights(x - 9, y + 6, 2, raccoon1);
  SetLED(x - 7, y + 6, skintone2);
  SetLights(x - 6, y + 6, 5, outline);
  SetLights(x - 1, y + 6, 2, overalls1);
  SetLED(x + 1, y + 6, overalls2);
  SetLights(x + 2, y + 6, 2, raccoon3);
  SetLights(x + 4, y + 6, 2, raccoon4);
  SetLED(x + 6, y + 6, raccoon3);
  SetLED(x + 7, y + 6, outline);
  SetLED(x - 14, y + 7, outline); // START ROW +7
  SetLights(x - 13, y + 7, 2, raccoon1);
  SetLights(x - 11, y + 7, 2, skintone3);
  SetLights(x - 9, y + 7, 2, raccoon1);
  SetLights(x - 7, y + 7, 3, outline);
  SetLights(x - 4, y + 7, 3, overalls1);
  SetLights(x - 1, y + 7, 3, overalls2);
  SetLights(x + 2, y + 7, 2, raccoon3);
  SetLED(x + 4, y + 7, raccoon4);
  SetLED(x + 5, y + 7, overalls2);
  SetLED(x + 6, y + 7, raccoon3);
  SetLED(x + 7, y + 7, outline);
  SetLED(x - 14, y + 8, outline); // START ROW +8
  SetLights(x - 13, y + 8, 2, raccoon2);
  SetLights(x - 11, y + 8, 2, skintone2);
  SetLED(x - 9, y + 8, raccoon2);
  SetLED(x - 8, y + 8, outline);
  SetLights(x - 7, y + 8, 2, raccoon1);
  SetLED(x - 5, y + 8, outline);
  SetLights(x - 4, y + 8, 3, overalls1);
  SetLights(x - 1, y + 8, 7, overalls2);
  SetLED(x + 6, y + 8, outline);
  SetLights(x - 14, y + 9, 2, outline); // START ROW +9
  SetLED(x - 12, y + 9, raccoon2);
  SetLights(x - 11, y + 9, 2, skintone2);
  SetLED(x - 9, y + 9, raccoon2);
  SetLED(x - 8, y + 9, outline);
  SetLED(x - 7, y + 9, raccoon1);
  SetLights(x - 6, y + 9, 2, outline);
  SetLights(x - 4, y + 9, 5, overalls1);
  SetLights(x + 1, y + 9, 3, overalls2);
  SetLights(x + 4, y + 9, 2, overalls1);
  SetLED(x + 6, y + 9, outline);
  SetLights(x - 13, y + 10, 5, outline); // START ROW +10
  SetLED(x - 8, y + 10, raccoon1);
  SetLED(x - 7, y + 10, raccoon5);
  SetLED(x - 6, y + 10, outline);
  SetLights(x - 5, y + 10, 4, overalls1);
  SetLights(x - 1, y + 10, 2, outline);
  SetLights(x + 1, y + 10, 3, overalls1);
  SetLights(x + 4, y + 10, 2, outline);
  SetLED(x - 9, y + 11, outline); // START ROW +11
  SetLED(x - 8, y + 11, raccoon1);
  SetLED(x - 7, y + 11, raccoon5);
  SetLED(x - 6, y + 11, goldshoes);
  SetLights(x - 5, y + 11, 4, outline);
  SetLights(x - 1, y + 11, 2, overalls1);
  SetLights(x + 1, y + 11, 3, outline);
  SetLED(x - 9, y + 12, outline); // START ROW +12
  SetLED(x - 8, y + 12, raccoon1);
  SetLED(x - 7, y + 12, goldshoes);
  SetLED(x - 6, y + 12, outline);
  SetLED(x - 5, y + 12, raccoon1);
  SetLED(x - 4, y + 12, raccoon5);
  SetLights(x - 3, y + 12, 4, outline);
  SetLights(x - 8, y + 13, 2, outline); // START ROW +13
  SetLights(x - 5, y + 13, 2, outline);
}

void DrawMarioWarp03(int x, int y, byte outline = 0, byte skintone = 12, byte raccoon = 28)
{
  SetLights(x - 4, y - 13, 2, outline); // START ROW -13
  SetLights(x + 0, y - 13, 4, outline);
  SetLED(x + 5, y - 13, outline);
  SetLED(x - 5, y - 12, outline); // START ROW -12
  SetLights(x - 4, y - 12, 2, skintone);
  SetLights(x - 2, y - 12, 2, outline);
  SetLights(x + 0, y - 12, 4, raccoon);
  SetLED(x + 4, y - 12, outline);
  SetLED(x + 5, y - 12, skintone);
  SetLED(x + 6, y - 12, outline);
  SetLED(x - 5, y - 11, outline); // START ROW -11
  SetLights(x - 4, y - 11, 3, skintone);
  SetLights(x - 1, y - 11, 6, raccoon);
  SetLED(x + 5, y - 11, skintone);
  SetLED(x + 6, y - 11, outline);
  SetLED(x - 5, y - 10, outline); // START ROW -10
  SetLights(x - 4, y - 10, 3, skintone);
  SetLights(x - 1, y - 10, 7, raccoon);
  SetLED(x + 6, y - 10, outline);
  SetLED(x - 5, y - 9, outline); // START ROW -9
  SetLights(x - 4, y - 9, 2, skintone);
  SetLights(x - 2, y - 9, 9, raccoon);
  SetLED(x + 7, y - 9, outline);
  SetLED(x - 6, y - 8, outline); // START ROW -8
  SetLights(x - 5, y - 8, 12, raccoon);
  SetLED(x + 7, y - 8, outline);
  SetLED(x - 6, y - 7, outline); // START ROW -7
  SetLights(x - 5, y - 7, 6, raccoon);
  SetLights(x + 1, y - 7, 5, outline);
  SetLED(x + 6, y - 7, raccoon);
  SetLED(x + 7, y - 7, outline);
  SetLED(x - 6, y - 6, outline); // START ROW -6
  SetLights(x - 5, y - 6, 5, raccoon);
  SetLED(x + 0, y - 6, outline);
  SetLED(x + 1, y - 6, skintone);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone);
  SetLights(x + 6, y - 6, 2, outline);
  SetLED(x - 6, y - 5, outline); // START ROW -5
  SetLights(x - 5, y - 5, 4, raccoon);
  SetLED(x - 1, y - 5, outline);
  SetLights(x + 0, y - 5, 2, skintone);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone);
  SetLED(x + 6, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLights(x - 5, y - 4, 4, raccoon);
  SetLED(x - 1, y - 4, outline);
  SetLights(x + 0, y - 4, 7, skintone);
  SetLED(x + 7, y - 4, outline);
  SetLED(x - 5, y - 3, outline); // START ROW -3
  SetLights(x - 4, y - 3, 3, raccoon);
  SetLights(x - 1, y - 3, 2, skintone);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 5, skintone);
  SetLED(x + 7, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLights(x - 4, y - 2, 3, raccoon);
  SetLED(x - 1, y - 2, skintone);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone);
  SetLights(x + 6, y - 2, 3, outline);
  SetLED(x - 4, y - 1, outline); // START ROW -1
  SetLights(x - 3, y - 1, 3, raccoon);
  SetLights(x + 0, y - 1, 2, skintone);
  SetLights(x + 2, y - 1, 5, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone);
  SetLED(x + 5, y + 0, outline);
  SetLights(x - 5, y + 1, 2, outline); // START ROW +1
  SetLights(x - 3, y + 1, 3, raccoon);
  SetLights(x + 0, y + 1, 9, outline);
  SetLights(x - 8, y + 2, 4, outline); // START ROW +2
  SetLights(x - 4, y + 2, 4, raccoon);
  SetLED(x + 0, y + 2, outline);
  SetLights(x + 1, y + 2, 4, raccoon);
  SetLED(x + 5, y + 2, outline);
  SetLights(x + 6, y + 2, 3, skintone);
  SetLED(x + 9, y + 2, outline);
  SetLED(x - 9, y + 3, outline); // START ROW +3
  SetLights(x - 8, y + 3, 5, skintone);
  SetLights(x - 3, y + 3, 3, raccoon);
  SetLED(x + 0, y + 3, outline);
  SetLights(x + 1, y + 3, 5, raccoon);
  SetLED(x + 6, y + 3, outline);
  SetLights(x + 7, y + 3, 2, skintone);
  SetLED(x + 9, y + 3, outline);
  SetLED(x - 9, y + 4, outline); // START ROW +4
  SetLights(x - 8, y + 4, 6, skintone);
  SetLED(x - 2, y + 4, raccoon);
  SetLED(x - 1, y + 4, outline);
  SetLights(x + 0, y + 4, 3, raccoon);
  SetLights(x + 3, y + 4, 3, skintone);
  SetLED(x + 6, y + 4, raccoon);
  SetLights(x + 7, y + 4, 2, outline);
  SetLights(x - 13, y + 5, 8, outline); // START ROW +5
  SetLights(x - 5, y + 5, 2, skintone);
  SetLights(x - 3, y + 5, 2, outline);
  SetLights(x - 1, y + 5, 2, raccoon);
  SetLights(x + 1, y + 5, 6, skintone);
  SetLED(x + 7, y + 5, outline);
  SetLights(x - 14, y + 6, 2, outline); // START ROW +6
  SetLED(x - 12, y + 6, raccoon);
  SetLights(x - 11, y + 6, 2, skintone);
  SetLights(x - 9, y + 6, 2, raccoon);
  SetLED(x - 7, y + 6, skintone);
  SetLights(x - 6, y + 6, 3, outline);
  SetLights(x - 3, y + 6, 3, raccoon);
  SetLights(x + 0, y + 6, 7, skintone);
  SetLED(x + 7, y + 6, outline);
  SetLED(x - 14, y + 7, outline); // START ROW +7
  SetLights(x - 13, y + 7, 2, raccoon);
  SetLights(x - 11, y + 7, 2, skintone);
  SetLights(x - 9, y + 7, 2, raccoon);
  SetLights(x - 7, y + 7, 2, outline);
  SetLights(x - 5, y + 7, 5, raccoon);
  SetLights(x + 0, y + 7, 7, skintone);
  SetLED(x + 7, y + 7, outline);
  SetLED(x - 14, y + 8, outline); // START ROW +8
  SetLights(x - 13, y + 8, 2, raccoon);
  SetLights(x - 11, y + 8, 2, skintone);
  SetLED(x - 9, y + 8, raccoon);
  SetLED(x - 8, y + 8, outline);
  SetLED(x - 7, y + 8, skintone);
  SetLights(x - 6, y + 8, 5, raccoon);
  SetLights(x - 1, y + 8, 7, skintone);
  SetLED(x + 6, y + 8, outline);
  SetLights(x - 14, y + 9, 2, outline); // START ROW +9
  SetLED(x - 12, y + 9, raccoon);
  SetLights(x - 11, y + 9, 2, skintone);
  SetLED(x - 9, y + 9, raccoon);
  SetLED(x - 8, y + 9, outline);
  SetLED(x - 7, y + 9, skintone);
  SetLights(x - 6, y + 9, 5, raccoon);
  SetLights(x - 1, y + 9, 7, skintone);
  SetLED(x + 6, y + 9, outline);
  SetLights(x - 13, y + 10, 5, outline); // START ROW +10
  SetLED(x - 8, y + 10, skintone);
  SetLights(x - 7, y + 10, 4, raccoon);
  SetLED(x - 3, y + 10, outline);
  SetLights(x - 2, y + 10, 2, raccoon);
  SetLights(x + 0, y + 10, 4, skintone);
  SetLights(x + 4, y + 10, 2, outline);
  SetLED(x - 9, y + 11, outline); // START ROW +11
  SetLED(x - 8, y + 11, skintone);
  SetLights(x - 7, y + 11, 2, raccoon);
  SetLights(x - 5, y + 11, 2, outline);
  SetLights(x - 3, y + 11, 4, raccoon);
  SetLights(x + 1, y + 11, 3, outline);
  SetLED(x - 9, y + 12, outline); // START ROW +12
  SetLights(x - 8, y + 12, 2, skintone);
  SetLED(x - 6, y + 12, outline);
  SetLights(x - 5, y + 12, 2, skintone);
  SetLights(x - 3, y + 12, 4, outline);
  SetLights(x - 8, y + 13, 2, outline); // START ROW +13
  SetLights(x - 5, y + 13, 2, outline);
}

void DrawMarioWarp04(int x, int y, byte outline = 0, byte skintone2 = 15, byte tanuki1 = 30, byte skintone1 = 14, byte tanuki2 = 31, byte tanuki3 = 32, byte skintone3 = 21, byte belly = 16, byte tanuki4 = 17, byte tanuki5 = 19, byte goldshoes = 20)
{
  SetLights(x - 4, y - 13, 2, outline); // START ROW -13
  SetLights(x + 0, y - 13, 4, outline);
  SetLED(x + 5, y - 13, outline);
  SetLED(x - 5, y - 12, outline); // START ROW -12
  SetLED(x - 4, y - 12, skintone2);
  SetLED(x - 3, y - 12, tanuki1);
  SetLights(x - 2, y - 12, 2, outline);
  SetLights(x + 0, y - 12, 4, tanuki1);
  SetLED(x + 4, y - 12, outline);
  SetLED(x + 5, y - 12, skintone2);
  SetLED(x + 6, y - 12, outline);
  SetLED(x - 5, y - 11, outline); // START ROW -11
  SetLED(x - 4, y - 11, skintone1);
  SetLED(x - 3, y - 11, skintone2);
  SetLights(x - 2, y - 11, 2, tanuki1);
  SetLights(x + 0, y - 11, 4, tanuki2);
  SetLED(x + 4, y - 11, tanuki1);
  SetLED(x + 5, y - 11, skintone2);
  SetLED(x + 6, y - 11, outline);
  SetLED(x - 5, y - 10, outline); // START ROW -10
  SetLED(x - 4, y - 10, skintone1);
  SetLED(x - 3, y - 10, skintone2);
  SetLights(x - 2, y - 10, 2, tanuki1);
  SetLights(x + 0, y - 10, 2, tanuki2);
  SetLights(x + 2, y - 10, 2, tanuki3);
  SetLED(x + 4, y - 10, tanuki2);
  SetLED(x + 5, y - 10, tanuki1);
  SetLED(x + 6, y - 10, outline);
  SetLED(x - 5, y - 9, outline); // START ROW -9
  SetLED(x - 4, y - 9, skintone1);
  SetLED(x - 3, y - 9, skintone2);
  SetLED(x - 2, y - 9, tanuki1);
  SetLights(x - 1, y - 9, 2, tanuki2);
  SetLights(x + 1, y - 9, 4, tanuki3);
  SetLED(x + 5, y - 9, tanuki2);
  SetLED(x + 6, y - 9, tanuki1);
  SetLED(x + 7, y - 9, outline);
  SetLED(x - 6, y - 8, outline); // START ROW -8
  SetLights(x - 5, y - 8, 3, tanuki1);
  SetLights(x - 2, y - 8, 8, tanuki2);
  SetLED(x + 6, y - 8, tanuki1);
  SetLED(x + 7, y - 8, outline);
  SetLED(x - 6, y - 7, outline); // START ROW -7
  SetLights(x - 5, y - 7, 2, tanuki1);
  SetLights(x - 3, y - 7, 3, tanuki2);
  SetLED(x + 0, y - 7, tanuki1);
  SetLights(x + 1, y - 7, 5, outline);
  SetLED(x + 6, y - 7, tanuki1);
  SetLED(x + 7, y - 7, outline);
  SetLED(x - 6, y - 6, outline); // START ROW -6
  SetLights(x - 5, y - 6, 2, tanuki1);
  SetLights(x - 3, y - 6, 2, tanuki2);
  SetLED(x - 1, y - 6, tanuki1);
  SetLED(x + 0, y - 6, outline);
  SetLED(x + 1, y - 6, skintone2);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone2);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone2);
  SetLights(x + 6, y - 6, 2, outline);
  SetLED(x - 6, y - 5, outline); // START ROW -5
  SetLights(x - 5, y - 5, 2, tanuki1);
  SetLED(x - 3, y - 5, tanuki2);
  SetLED(x - 2, y - 5, tanuki1);
  SetLED(x - 1, y - 5, outline);
  SetLED(x + 0, y - 5, skintone2);
  SetLED(x + 1, y - 5, skintone1);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone1);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone1);
  SetLED(x + 6, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLights(x - 5, y - 4, 2, tanuki1);
  SetLED(x - 3, y - 4, tanuki2);
  SetLED(x - 2, y - 4, tanuki1);
  SetLED(x - 1, y - 4, outline);
  SetLED(x + 0, y - 4, skintone2);
  SetLights(x + 1, y - 4, 3, skintone1);
  SetLights(x + 4, y - 4, 3, skintone3);
  SetLED(x + 7, y - 4, outline);
  SetLED(x - 5, y - 3, outline); // START ROW -3
  SetLights(x - 4, y - 3, 3, tanuki1);
  SetLights(x - 1, y - 3, 2, skintone2);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 2, skintone2);
  SetLights(x + 4, y - 3, 3, skintone1);
  SetLED(x + 7, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLights(x - 4, y - 2, 3, tanuki1);
  SetLED(x - 1, y - 2, skintone2);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone2);
  SetLights(x + 6, y - 2, 3, outline);
  SetLED(x - 4, y - 1, outline); // START ROW -1
  SetLights(x - 3, y - 1, 3, tanuki1);
  SetLED(x + 0, y - 1, skintone2);
  SetLED(x + 1, y - 1, skintone1);
  SetLights(x + 2, y - 1, 5, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone2);
  SetLED(x + 5, y + 0, outline);
  SetLights(x - 5, y + 1, 2, outline); // START ROW +1
  SetLED(x - 3, y + 1, tanuki2);
  SetLights(x - 2, y + 1, 2, tanuki1);
  SetLights(x + 0, y + 1, 9, outline);
  SetLights(x - 8, y + 2, 4, outline); // START ROW +2
  SetLights(x - 4, y + 2, 3, tanuki2);
  SetLED(x - 1, y + 2, tanuki1);
  SetLED(x + 0, y + 2, outline);
  SetLights(x + 1, y + 2, 4, tanuki1);
  SetLED(x + 5, y + 2, outline);
  SetLED(x + 6, y + 2, skintone2);
  SetLights(x + 7, y + 2, 2, skintone1);
  SetLED(x + 9, y + 2, outline);
  SetLED(x - 9, y + 3, outline); // START ROW +3
  SetLights(x - 8, y + 3, 2, skintone3);
  SetLights(x - 6, y + 3, 3, skintone1);
  SetLights(x - 3, y + 3, 3, tanuki1);
  SetLED(x + 0, y + 3, outline);
  SetLED(x + 1, y + 3, tanuki1);
  SetLights(x + 2, y + 3, 3, tanuki2);
  SetLED(x + 5, y + 3, tanuki1);
  SetLED(x + 6, y + 3, outline);
  SetLights(x + 7, y + 3, 2, skintone2);
  SetLED(x + 9, y + 3, outline);
  SetLED(x - 9, y + 4, outline); // START ROW +4
  SetLights(x - 8, y + 4, 2, skintone1);
  SetLights(x - 6, y + 4, 2, skintone2);
  SetLED(x - 4, y + 4, skintone1);
  SetLED(x - 3, y + 4, skintone2);
  SetLED(x - 2, y + 4, tanuki1);
  SetLED(x - 1, y + 4, outline);
  SetLED(x + 0, y + 4, tanuki1);
  SetLights(x + 1, y + 4, 2, tanuki2);
  SetLights(x + 3, y + 4, 2, belly);
  SetLED(x + 5, y + 4, skintone3);
  SetLED(x + 6, y + 4, tanuki1);
  SetLights(x + 7, y + 4, 2, outline);
  SetLights(x - 13, y + 5, 8, outline); // START ROW +5
  SetLights(x - 5, y + 5, 2, skintone2);
  SetLights(x - 3, y + 5, 2, outline);
  SetLED(x - 1, y + 5, tanuki1);
  SetLED(x + 0, y + 5, tanuki2);
  SetLights(x + 1, y + 5, 5, belly);
  SetLED(x + 6, y + 5, skintone3);
  SetLED(x + 7, y + 5, outline);
  SetLights(x - 14, y + 6, 2, outline); // START ROW +6
  SetLED(x - 12, y + 6, tanuki2);
  SetLights(x - 11, y + 6, 2, skintone3);
  SetLights(x - 9, y + 6, 2, tanuki2);
  SetLED(x - 7, y + 6, tanuki4);
  SetLights(x - 6, y + 6, 3, outline);
  SetLights(x - 3, y + 6, 2, tanuki1);
  SetLED(x - 1, y + 6, tanuki2);
  SetLights(x + 0, y + 6, 6, belly);
  SetLED(x + 6, y + 6, skintone3);
  SetLED(x + 7, y + 6, outline);
  SetLED(x - 14, y + 7, outline); // START ROW +7
  SetLights(x - 13, y + 7, 2, tanuki2);
  SetLights(x - 11, y + 7, 2, skintone3);
  SetLights(x - 9, y + 7, 2, tanuki2);
  SetLights(x - 7, y + 7, 2, outline);
  SetLights(x - 5, y + 7, 2, tanuki1);
  SetLights(x - 3, y + 7, 3, tanuki2);
  SetLights(x + 0, y + 7, 6, belly);
  SetLED(x + 6, y + 7, skintone3);
  SetLED(x + 7, y + 7, outline);
  SetLED(x - 14, y + 8, outline); // START ROW +8
  SetLights(x - 13, y + 8, 2, tanuki1);
  SetLights(x - 11, y + 8, 2, skintone1);
  SetLED(x - 9, y + 8, tanuki1);
  SetLED(x - 8, y + 8, outline);
  SetLED(x - 7, y + 8, tanuki5);
  SetLED(x - 6, y + 8, tanuki1);
  SetLights(x - 5, y + 8, 3, tanuki2);
  SetLED(x - 2, y + 8, tanuki1);
  SetLED(x - 1, y + 8, skintone3);
  SetLights(x + 0, y + 8, 5, belly);
  SetLED(x + 5, y + 8, skintone3);
  SetLED(x + 6, y + 8, outline);
  SetLights(x - 14, y + 9, 2, outline); // START ROW +9
  SetLED(x - 12, y + 9, tanuki1);
  SetLights(x - 11, y + 9, 2, skintone1);
  SetLED(x - 9, y + 9, tanuki1);
  SetLED(x - 8, y + 9, outline);
  SetLED(x - 7, y + 9, tanuki5);
  SetLights(x - 6, y + 9, 2, tanuki1);
  SetLED(x - 4, y + 9, tanuki2);
  SetLights(x - 3, y + 9, 2, tanuki1);
  SetLights(x - 1, y + 9, 2, skintone3);
  SetLights(x + 1, y + 9, 3, belly);
  SetLights(x + 4, y + 9, 2, skintone3);
  SetLED(x + 6, y + 9, outline);
  SetLights(x - 13, y + 10, 5, outline); // START ROW +10
  SetLED(x - 8, y + 10, tanuki5);
  SetLights(x - 7, y + 10, 4, tanuki1);
  SetLED(x - 3, y + 10, outline);
  SetLights(x - 2, y + 10, 2, tanuki1);
  SetLights(x + 0, y + 10, 4, skintone3);
  SetLights(x + 4, y + 10, 2, outline);
  SetLED(x - 9, y + 11, outline); // START ROW +11
  SetLED(x - 8, y + 11, tanuki5);
  SetLights(x - 7, y + 11, 2, tanuki1);
  SetLights(x - 5, y + 11, 2, outline);
  SetLights(x - 3, y + 11, 4, tanuki1);
  SetLights(x + 1, y + 11, 3, outline);
  SetLED(x - 9, y + 12, outline); // START ROW +12
  SetLED(x - 8, y + 12, goldshoes);
  SetLED(x - 7, y + 12, tanuki5);
  SetLED(x - 6, y + 12, outline);
  SetLights(x - 5, y + 12, 2, tanuki5);
  SetLights(x - 3, y + 12, 4, outline);
  SetLights(x - 8, y + 13, 2, outline); // START ROW +13
  SetLights(x - 5, y + 13, 2, outline);
}

void DrawMarioWarp05(int x, int y, byte outline = 0, byte skintone = 12, byte redsuit = 34)
{
  SetLights(x + 1, y - 12, 4, outline); // START ROW -12
  SetLights(x + 6, y - 12, 3, outline);
  SetLights(x - 1, y - 11, 2, outline); // START ROW -11
  SetLights(x + 1, y - 11, 3, redsuit);
  SetLights(x + 4, y - 11, 2, outline);
  SetLights(x + 6, y - 11, 3, skintone);
  SetLED(x + 9, y - 11, outline);
  SetLED(x - 2, y - 10, outline); // START ROW -10
  SetLights(x - 1, y - 10, 6, redsuit);
  SetLED(x + 5, y - 10, outline);
  SetLights(x + 6, y - 10, 3, skintone);
  SetLED(x + 9, y - 10, outline);
  SetLED(x - 3, y - 9, outline); // START ROW -9
  SetLights(x - 2, y - 9, 3, redsuit);
  SetLights(x + 1, y - 9, 6, outline);
  SetLights(x + 7, y - 9, 2, skintone);
  SetLED(x + 9, y - 9, outline);
  SetLED(x - 4, y - 8, outline); // START ROW -8
  SetLights(x - 3, y - 8, 3, redsuit);
  SetLights(x + 0, y - 8, 8, outline);
  SetLED(x + 8, y - 8, skintone);
  SetLED(x + 9, y - 8, outline);
  SetLED(x - 4, y - 7, outline); // START ROW -7
  SetLED(x - 3, y - 7, redsuit);
  SetLights(x - 2, y - 7, 2, outline);
  SetLights(x + 0, y - 7, 6, skintone);
  SetLights(x + 6, y - 7, 3, outline);
  SetLights(x - 5, y - 6, 4, outline); // START ROW -6
  SetLights(x - 1, y - 6, 3, skintone);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone);
  SetLED(x + 6, y - 6, outline);
  SetLights(x + 7, y - 6, 2, redsuit);
  SetLED(x + 9, y - 6, outline);
  SetLED(x - 5, y - 5, outline); // START ROW -5
  SetLED(x - 4, y - 5, skintone);
  SetLights(x - 3, y - 5, 2, outline);
  SetLights(x - 1, y - 5, 3, skintone);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone);
  SetLED(x + 6, y - 5, outline);
  SetLights(x + 7, y - 5, 2, redsuit);
  SetLED(x + 9, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLights(x - 5, y - 4, 2, skintone);
  SetLights(x - 3, y - 4, 3, outline);
  SetLights(x + 0, y - 4, 7, skintone);
  SetLED(x + 7, y - 4, outline);
  SetLED(x + 8, y - 4, redsuit);
  SetLED(x + 9, y - 4, outline);
  SetLED(x - 6, y - 3, outline); // START ROW -3
  SetLights(x - 5, y - 3, 3, skintone);
  SetLED(x - 2, y - 3, outline);
  SetLights(x - 1, y - 3, 2, skintone);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 5, skintone);
  SetLED(x + 7, y - 3, outline);
  SetLED(x + 8, y - 3, redsuit);
  SetLED(x + 9, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLights(x - 4, y - 2, 4, skintone);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone);
  SetLights(x + 6, y - 2, 3, outline);
  SetLights(x - 4, y - 1, 2, outline); // START ROW -1
  SetLights(x - 2, y - 1, 4, skintone);
  SetLights(x + 2, y - 1, 5, outline);
  SetLED(x + 7, y - 1, redsuit);
  SetLED(x + 8, y - 1, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone);
  SetLED(x + 5, y + 0, outline);
  SetLights(x + 6, y + 0, 2, redsuit);
  SetLED(x + 8, y + 0, outline);
  SetLED(x - 4, y + 1, outline); // START ROW +1
  SetLights(x - 3, y + 1, 3, redsuit);
  SetLights(x + 0, y + 1, 6, outline);
  SetLED(x + 6, y + 1, redsuit);
  SetLED(x + 7, y + 1, outline);
  SetLED(x - 5, y + 2, outline); // START ROW +2
  SetLights(x - 4, y + 2, 5, redsuit);
  SetLights(x + 1, y + 2, 2, outline);
  SetLights(x + 3, y + 2, 2, redsuit);
  SetLights(x + 5, y + 2, 2, outline);
  SetLED(x - 6, y + 3, outline); // START ROW +3
  SetLights(x - 5, y + 3, 3, redsuit);
  SetLights(x - 2, y + 3, 3, outline);
  SetLED(x + 1, y + 3, redsuit);
  SetLED(x + 2, y + 3, outline);
  SetLights(x + 3, y + 3, 3, redsuit);
  SetLights(x + 6, y + 3, 2, outline);
  SetLED(x - 6, y + 4, outline); // START ROW +4
  SetLights(x - 5, y + 4, 2, redsuit);
  SetLED(x - 3, y + 4, outline);
  SetLights(x - 2, y + 4, 3, skintone);
  SetLights(x + 1, y + 4, 3, outline);
  SetLights(x + 4, y + 4, 3, redsuit);
  SetLED(x + 7, y + 4, outline);
  SetLED(x - 6, y + 5, outline); // START ROW +5
  SetLED(x - 5, y + 5, redsuit);
  SetLED(x - 4, y + 5, outline);
  SetLights(x - 3, y + 5, 5, skintone);
  SetLights(x + 2, y + 5, 3, outline);
  SetLights(x + 5, y + 5, 2, redsuit);
  SetLights(x + 7, y + 5, 2, outline);
  SetLights(x - 5, y + 6, 2, outline); // START ROW +6
  SetLights(x - 3, y + 6, 5, skintone);
  SetLED(x + 2, y + 6, outline);
  SetLights(x + 3, y + 6, 2, skintone);
  SetLights(x + 5, y + 6, 2, outline);
  SetLED(x + 7, y + 6, skintone);
  SetLED(x + 8, y + 6, outline);
  SetLights(x - 4, y + 7, 2, outline); // START ROW +7
  SetLights(x - 2, y + 7, 3, skintone);
  SetLights(x + 1, y + 7, 2, outline);
  SetLights(x + 3, y + 7, 2, skintone);
  SetLights(x + 5, y + 7, 2, outline);
  SetLights(x + 7, y + 7, 2, redsuit);
  SetLED(x + 9, y + 7, outline);
  SetLED(x - 5, y + 8, outline); // START ROW +8
  SetLights(x - 4, y + 8, 2, redsuit);
  SetLights(x - 2, y + 8, 8, outline);
  SetLights(x + 6, y + 8, 3, redsuit);
  SetLED(x + 9, y + 8, outline);
  SetLights(x - 6, y + 9, 2, outline); // START ROW +9
  SetLED(x - 4, y + 9, redsuit);
  SetLights(x - 3, y + 9, 9, outline);
  SetLights(x + 6, y + 9, 3, redsuit);
  SetLED(x + 9, y + 9, outline);
  SetLED(x - 6, y + 10, outline); // START ROW +10
  SetLights(x - 5, y + 10, 2, redsuit);
  SetLights(x - 3, y + 10, 9, outline);
  SetLights(x + 6, y + 10, 3, redsuit);
  SetLED(x + 9, y + 10, outline);
  SetLED(x - 6, y + 11, outline); // START ROW +11
  SetLights(x - 5, y + 11, 2, redsuit);
  SetLights(x - 3, y + 11, 5, outline);
  SetLED(x + 5, y + 11, outline);
  SetLights(x + 6, y + 11, 2, redsuit);
  SetLights(x + 8, y + 11, 2, outline);
  SetLED(x - 6, y + 12, outline); // START ROW +12
  SetLights(x - 5, y + 12, 2, redsuit);
  SetLights(x - 3, y + 12, 2, outline);
  SetLights(x + 6, y + 12, 3, outline);
  SetLights(x - 5, y + 13, 2, outline); // START ROW +13
}

void DrawMarioWarp06(int x, int y, byte outline = 0, byte red3 = 27, byte skintone1 = 14, byte red1 = 25, byte red2 = 26, byte skintone2 = 15, byte skintone3 = 21, byte overalls2 = 23, byte overalls3 = 24, byte button = 16, byte goldshoes = 20, byte shoes1 = 19, byte shoes2 = 17, byte overalls1 = 22)
{
  SetLights(x + 1, y - 12, 4, outline); // START ROW -12
  SetLights(x + 6, y - 12, 3, outline);
  SetLights(x - 1, y - 11, 2, outline); // START ROW -11
  SetLights(x + 1, y - 11, 3, red3);
  SetLights(x + 4, y - 11, 2, outline);
  SetLights(x + 6, y - 11, 3, skintone1);
  SetLED(x + 9, y - 11, outline);
  SetLED(x - 2, y - 10, outline); // START ROW -10
  SetLED(x - 1, y - 10, red1);
  SetLights(x + 0, y - 10, 3, red2);
  SetLights(x + 3, y - 10, 2, red3);
  SetLED(x + 5, y - 10, outline);
  SetLED(x + 6, y - 10, skintone2);
  SetLights(x + 7, y - 10, 2, skintone1);
  SetLED(x + 9, y - 10, outline);
  SetLED(x - 3, y - 9, outline); // START ROW -9
  SetLED(x - 2, y - 9, red1);
  SetLights(x - 1, y - 9, 2, red2);
  SetLights(x + 1, y - 9, 6, outline);
  SetLights(x + 7, y - 9, 2, skintone2);
  SetLED(x + 9, y - 9, outline);
  SetLED(x - 4, y - 8, outline); // START ROW -8
  SetLights(x - 3, y - 8, 3, red1);
  SetLights(x + 0, y - 8, 8, outline);
  SetLED(x + 8, y - 8, skintone2);
  SetLED(x + 9, y - 8, outline);
  SetLED(x - 4, y - 7, outline); // START ROW -7
  SetLED(x - 3, y - 7, red1);
  SetLights(x - 2, y - 7, 2, outline);
  SetLights(x + 0, y - 7, 6, skintone2);
  SetLights(x + 6, y - 7, 3, outline);
  SetLights(x - 5, y - 6, 4, outline); // START ROW -6
  SetLED(x - 1, y - 6, skintone2);
  SetLights(x + 0, y - 6, 2, skintone1);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone1);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone1);
  SetLED(x + 6, y - 6, outline);
  SetLED(x + 7, y - 6, red2);
  SetLED(x + 8, y - 6, red1);
  SetLED(x + 9, y - 6, outline);
  SetLED(x - 5, y - 5, outline); // START ROW -5
  SetLED(x - 4, y - 5, skintone1);
  SetLights(x - 3, y - 5, 2, outline);
  SetLED(x - 1, y - 5, skintone2);
  SetLights(x + 0, y - 5, 2, skintone1);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone1);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone1);
  SetLED(x + 6, y - 5, outline);
  SetLED(x + 7, y - 5, red2);
  SetLED(x + 8, y - 5, red1);
  SetLED(x + 9, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLights(x - 5, y - 4, 2, skintone1);
  SetLights(x - 3, y - 4, 3, outline);
  SetLED(x + 0, y - 4, skintone2);
  SetLights(x + 1, y - 4, 3, skintone1);
  SetLights(x + 4, y - 4, 3, skintone3);
  SetLED(x + 7, y - 4, outline);
  SetLED(x + 8, y - 4, red1);
  SetLED(x + 9, y - 4, outline);
  SetLED(x - 6, y - 3, outline); // START ROW -3
  SetLED(x - 5, y - 3, skintone2);
  SetLED(x - 4, y - 3, skintone1);
  SetLED(x - 3, y - 3, skintone2);
  SetLED(x - 2, y - 3, outline);
  SetLED(x - 1, y - 3, skintone2);
  SetLED(x + 0, y - 3, skintone1);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 2, skintone2);
  SetLights(x + 4, y - 3, 3, skintone1);
  SetLED(x + 7, y - 3, outline);
  SetLED(x + 8, y - 3, red1);
  SetLED(x + 9, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLights(x - 4, y - 2, 3, skintone2);
  SetLED(x - 1, y - 2, skintone1);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone2);
  SetLights(x + 6, y - 2, 3, outline);
  SetLights(x - 4, y - 1, 2, outline); // START ROW -1
  SetLights(x - 2, y - 1, 2, skintone2);
  SetLights(x + 0, y - 1, 2, skintone1);
  SetLights(x + 2, y - 1, 5, outline);
  SetLED(x + 7, y - 1, red1);
  SetLED(x + 8, y - 1, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone2);
  SetLED(x + 5, y + 0, outline);
  SetLED(x + 6, y + 0, red2);
  SetLED(x + 7, y + 0, red1);
  SetLED(x + 8, y + 0, outline);
  SetLED(x - 4, y + 1, outline); // START ROW +1
  SetLights(x - 3, y + 1, 3, red2);
  SetLights(x + 0, y + 1, 6, outline);
  SetLED(x + 6, y + 1, red1);
  SetLED(x + 7, y + 1, outline);
  SetLED(x - 5, y + 2, outline); // START ROW +2
  SetLED(x - 4, y + 2, red1);
  SetLED(x - 3, y + 2, red2);
  SetLights(x - 2, y + 2, 2, red3);
  SetLED(x + 0, y + 2, red2);
  SetLights(x + 1, y + 2, 2, outline);
  SetLED(x + 3, y + 2, red2);
  SetLED(x + 4, y + 2, red3);
  SetLights(x + 5, y + 2, 2, outline);
  SetLED(x - 6, y + 3, outline); // START ROW +3
  SetLED(x - 5, y + 3, red1);
  SetLED(x - 4, y + 3, red2);
  SetLED(x - 3, y + 3, red3);
  SetLights(x - 2, y + 3, 3, outline);
  SetLED(x + 1, y + 3, overalls2);
  SetLED(x + 2, y + 3, outline);
  SetLED(x + 3, y + 3, red1);
  SetLED(x + 4, y + 3, red2);
  SetLED(x + 5, y + 3, red3);
  SetLights(x + 6, y + 3, 2, outline);
  SetLED(x - 6, y + 4, outline); // START ROW +4
  SetLights(x - 5, y + 4, 2, red1);
  SetLED(x - 3, y + 4, outline);
  SetLights(x - 2, y + 4, 3, skintone1);
  SetLED(x + 1, y + 4, outline);
  SetLED(x + 2, y + 4, overalls2);
  SetLED(x + 3, y + 4, outline);
  SetLED(x + 4, y + 4, red1);
  SetLights(x + 5, y + 4, 2, red2);
  SetLED(x + 7, y + 4, outline);
  SetLED(x - 6, y + 5, outline); // START ROW +5
  SetLED(x - 5, y + 5, red1);
  SetLED(x - 4, y + 5, outline);
  SetLED(x - 3, y + 5, skintone2);
  SetLED(x - 2, y + 5, skintone1);
  SetLights(x - 1, y + 5, 2, skintone3);
  SetLED(x + 1, y + 5, skintone1);
  SetLED(x + 2, y + 5, outline);
  SetLED(x + 3, y + 5, overalls3);
  SetLights(x + 4, y + 5, 5, outline);
  SetLights(x - 5, y + 6, 2, outline); // START ROW +6
  SetLights(x - 3, y + 6, 2, skintone2);
  SetLights(x - 1, y + 6, 3, skintone1);
  SetLED(x + 2, y + 6, outline);
  SetLights(x + 3, y + 6, 2, button);
  SetLights(x + 5, y + 6, 2, overalls3);
  SetLights(x + 7, y + 6, 2, outline);
  SetLights(x - 4, y + 7, 2, outline); // START ROW +7
  SetLights(x - 2, y + 7, 3, skintone2);
  SetLED(x + 1, y + 7, outline);
  SetLED(x + 2, y + 7, overalls2);
  SetLights(x + 3, y + 7, 2, button);
  SetLED(x + 5, y + 7, overalls3);
  SetLED(x + 6, y + 7, outline);
  SetLED(x + 7, y + 7, goldshoes);
  SetLED(x + 8, y + 7, shoes1);
  SetLED(x + 9, y + 7, outline);
  SetLED(x - 5, y + 8, outline); // START ROW +8
  SetLights(x - 4, y + 8, 2, shoes2);
  SetLights(x - 2, y + 8, 3, outline);
  SetLights(x + 1, y + 8, 4, overalls2);
  SetLED(x + 5, y + 8, outline);
  SetLED(x + 6, y + 8, goldshoes);
  SetLED(x + 7, y + 8, shoes1);
  SetLED(x + 8, y + 8, shoes2);
  SetLED(x + 9, y + 8, outline);
  SetLights(x - 6, y + 9, 2, outline); // START ROW +9
  SetLED(x - 4, y + 9, shoes2);
  SetLED(x - 3, y + 9, outline);
  SetLED(x - 2, y + 9, overalls1);
  SetLights(x - 1, y + 9, 3, overalls2);
  SetLights(x + 2, y + 9, 3, overalls1);
  SetLED(x + 5, y + 9, outline);
  SetLED(x + 6, y + 9, shoes1);
  SetLights(x + 7, y + 9, 2, shoes2);
  SetLED(x + 9, y + 9, outline);
  SetLED(x - 6, y + 10, outline); // START ROW +10
  SetLED(x - 5, y + 10, shoes2);
  SetLED(x - 4, y + 10, shoes1);
  SetLED(x - 3, y + 10, outline);
  SetLights(x - 2, y + 10, 4, overalls1);
  SetLights(x + 2, y + 10, 4, outline);
  SetLED(x + 6, y + 10, shoes1);
  SetLights(x + 7, y + 10, 2, shoes2);
  SetLED(x + 9, y + 10, outline);
  SetLED(x - 6, y + 11, outline); // START ROW +11
  SetLED(x - 5, y + 11, shoes2);
  SetLED(x - 4, y + 11, shoes1);
  SetLED(x - 3, y + 11, outline);
  SetLED(x - 2, y + 11, overalls1);
  SetLights(x - 1, y + 11, 3, outline);
  SetLED(x + 5, y + 11, outline);
  SetLED(x + 6, y + 11, shoes1);
  SetLED(x + 7, y + 11, shoes2);
  SetLights(x + 8, y + 11, 2, outline);
  SetLED(x - 6, y + 12, outline); // START ROW +12
  SetLED(x - 5, y + 12, shoes2);
  SetLED(x - 4, y + 12, shoes1);
  SetLights(x - 3, y + 12, 2, outline);
  SetLights(x + 6, y + 12, 3, outline);
  SetLights(x - 5, y + 13, 2, outline); // START ROW +13
}

void DrawMarioWarp07(int x, int y, byte outline = 34, byte skintone = 12, byte suit = 35)
{
  SetLights(x + 1, y - 12, 4, outline); // START ROW -12
  SetLights(x + 6, y - 12, 3, outline);
  SetLights(x - 1, y - 11, 2, outline); // START ROW -11
  SetLights(x + 1, y - 11, 3, suit);
  SetLights(x + 4, y - 11, 2, outline);
  SetLights(x + 6, y - 11, 3, skintone);
  SetLED(x + 9, y - 11, outline);
  SetLED(x - 2, y - 10, outline); // START ROW -10
  SetLights(x - 1, y - 10, 6, suit);
  SetLED(x + 5, y - 10, outline);
  SetLights(x + 6, y - 10, 3, skintone);
  SetLED(x + 9, y - 10, outline);
  SetLED(x - 3, y - 9, outline); // START ROW -9
  SetLights(x - 2, y - 9, 3, suit);
  SetLights(x + 1, y - 9, 6, outline);
  SetLights(x + 7, y - 9, 2, skintone);
  SetLED(x + 9, y - 9, outline);
  SetLED(x - 4, y - 8, outline); // START ROW -8
  SetLights(x - 3, y - 8, 3, suit);
  SetLights(x + 0, y - 8, 8, outline);
  SetLED(x + 8, y - 8, skintone);
  SetLED(x + 9, y - 8, outline);
  SetLED(x - 4, y - 7, outline); // START ROW -7
  SetLED(x - 3, y - 7, suit);
  SetLights(x - 2, y - 7, 2, outline);
  SetLights(x + 0, y - 7, 6, skintone);
  SetLights(x + 6, y - 7, 3, outline);
  SetLights(x - 5, y - 6, 4, outline); // START ROW -6
  SetLights(x - 1, y - 6, 3, skintone);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone);
  SetLED(x + 6, y - 6, outline);
  SetLights(x + 7, y - 6, 2, suit);
  SetLED(x + 9, y - 6, outline);
  SetLED(x - 5, y - 5, outline); // START ROW -5
  SetLED(x - 4, y - 5, skintone);
  SetLights(x - 3, y - 5, 2, outline);
  SetLights(x - 1, y - 5, 3, skintone);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone);
  SetLED(x + 6, y - 5, outline);
  SetLights(x + 7, y - 5, 2, suit);
  SetLED(x + 9, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLights(x - 5, y - 4, 2, skintone);
  SetLights(x - 3, y - 4, 3, outline);
  SetLights(x + 0, y - 4, 7, skintone);
  SetLED(x + 7, y - 4, outline);
  SetLED(x + 8, y - 4, suit);
  SetLED(x + 9, y - 4, outline);
  SetLED(x - 6, y - 3, outline); // START ROW -3
  SetLights(x - 5, y - 3, 3, skintone);
  SetLED(x - 2, y - 3, outline);
  SetLights(x - 1, y - 3, 2, skintone);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 5, skintone);
  SetLED(x + 7, y - 3, outline);
  SetLED(x + 8, y - 3, suit);
  SetLED(x + 9, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLights(x - 4, y - 2, 4, skintone);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone);
  SetLights(x + 6, y - 2, 3, outline);
  SetLights(x - 4, y - 1, 2, outline); // START ROW -1
  SetLights(x - 2, y - 1, 4, skintone);
  SetLights(x + 2, y - 1, 5, outline);
  SetLED(x + 7, y - 1, suit);
  SetLED(x + 8, y - 1, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone);
  SetLED(x + 5, y + 0, outline);
  SetLights(x + 6, y + 0, 2, suit);
  SetLED(x + 8, y + 0, outline);
  SetLED(x - 4, y + 1, outline); // START ROW +1
  SetLights(x - 3, y + 1, 3, suit);
  SetLights(x + 0, y + 1, 6, outline);
  SetLED(x + 6, y + 1, suit);
  SetLED(x + 7, y + 1, outline);
  SetLED(x - 5, y + 2, outline); // START ROW +2
  SetLights(x - 4, y + 2, 5, suit);
  SetLights(x + 1, y + 2, 2, outline);
  SetLights(x + 3, y + 2, 2, suit);
  SetLights(x + 5, y + 2, 2, outline);
  SetLED(x - 6, y + 3, outline); // START ROW +3
  SetLights(x - 5, y + 3, 3, suit);
  SetLights(x - 2, y + 3, 3, outline);
  SetLED(x + 1, y + 3, suit);
  SetLED(x + 2, y + 3, outline);
  SetLights(x + 3, y + 3, 3, suit);
  SetLights(x + 6, y + 3, 2, outline);
  SetLED(x - 6, y + 4, outline); // START ROW +4
  SetLights(x - 5, y + 4, 2, suit);
  SetLED(x - 3, y + 4, outline);
  SetLights(x - 2, y + 4, 3, skintone);
  SetLights(x + 1, y + 4, 3, outline);
  SetLights(x + 4, y + 4, 3, suit);
  SetLED(x + 7, y + 4, outline);
  SetLED(x - 6, y + 5, outline); // START ROW +5
  SetLED(x - 5, y + 5, suit);
  SetLED(x - 4, y + 5, outline);
  SetLights(x - 3, y + 5, 5, skintone);
  SetLights(x + 2, y + 5, 3, outline);
  SetLights(x + 5, y + 5, 2, suit);
  SetLights(x + 7, y + 5, 2, outline);
  SetLights(x - 5, y + 6, 2, outline); // START ROW +6
  SetLights(x - 3, y + 6, 5, skintone);
  SetLED(x + 2, y + 6, outline);
  SetLights(x + 3, y + 6, 2, skintone);
  SetLights(x + 5, y + 6, 2, outline);
  SetLED(x + 7, y + 6, skintone);
  SetLED(x + 8, y + 6, outline);
  SetLights(x - 4, y + 7, 2, outline); // START ROW +7
  SetLights(x - 2, y + 7, 3, skintone);
  SetLights(x + 1, y + 7, 2, outline);
  SetLights(x + 3, y + 7, 2, skintone);
  SetLights(x + 5, y + 7, 2, outline);
  SetLights(x + 7, y + 7, 2, suit);
  SetLED(x + 9, y + 7, outline);
  SetLED(x - 5, y + 8, outline); // START ROW +8
  SetLights(x - 4, y + 8, 2, suit);
  SetLights(x - 2, y + 8, 8, outline);
  SetLights(x + 6, y + 8, 3, suit);
  SetLED(x + 9, y + 8, outline);
  SetLights(x - 6, y + 9, 2, outline); // START ROW +9
  SetLED(x - 4, y + 9, suit);
  SetLights(x - 3, y + 9, 9, outline);
  SetLights(x + 6, y + 9, 3, suit);
  SetLED(x + 9, y + 9, outline);
  SetLED(x - 6, y + 10, outline); // START ROW +10
  SetLights(x - 5, y + 10, 2, suit);
  SetLights(x - 3, y + 10, 9, outline);
  SetLights(x + 6, y + 10, 3, suit);
  SetLED(x + 9, y + 10, outline);
  SetLED(x - 6, y + 11, outline); // START ROW +11
  SetLights(x - 5, y + 11, 2, suit);
  SetLights(x - 3, y + 11, 5, outline);
  SetLED(x + 5, y + 11, outline);
  SetLights(x + 6, y + 11, 2, suit);
  SetLights(x + 8, y + 11, 2, outline);
  SetLED(x - 6, y + 12, outline); // START ROW +12
  SetLights(x - 5, y + 12, 2, suit);
  SetLights(x - 3, y + 12, 2, outline);
  SetLights(x + 6, y + 12, 3, outline);
  SetLights(x - 5, y + 13, 2, outline); // START ROW +13
}

void DrawMarioWarp08(int x, int y, byte outline = 0, byte hat3 = 16, byte skintone1 = 14, byte hat1 = 39, byte hat2 = 40, byte skintone2 = 15, byte skintone3 = 21, byte overalls2 = 37, byte overalls3 = 38, byte goldshoes = 20, byte shoes1 = 19, byte shoes2 = 17, byte overalls1 = 36)
{
  SetLights(x + 1, y - 12, 4, outline); // START ROW -12
  SetLights(x + 6, y - 12, 3, outline);
  SetLights(x - 1, y - 11, 2, outline); // START ROW -11
  SetLights(x + 1, y - 11, 3, hat3);
  SetLights(x + 4, y - 11, 2, outline);
  SetLights(x + 6, y - 11, 3, skintone1);
  SetLED(x + 9, y - 11, outline);
  SetLED(x - 2, y - 10, outline); // START ROW -10
  SetLED(x - 1, y - 10, hat1);
  SetLights(x + 0, y - 10, 3, hat2);
  SetLights(x + 3, y - 10, 2, hat3);
  SetLED(x + 5, y - 10, outline);
  SetLED(x + 6, y - 10, skintone2);
  SetLights(x + 7, y - 10, 2, skintone1);
  SetLED(x + 9, y - 10, outline);
  SetLED(x - 3, y - 9, outline); // START ROW -9
  SetLED(x - 2, y - 9, hat1);
  SetLights(x - 1, y - 9, 2, hat2);
  SetLights(x + 1, y - 9, 6, outline);
  SetLights(x + 7, y - 9, 2, skintone2);
  SetLED(x + 9, y - 9, outline);
  SetLED(x - 4, y - 8, outline); // START ROW -8
  SetLights(x - 3, y - 8, 3, hat1);
  SetLights(x + 0, y - 8, 8, outline);
  SetLED(x + 8, y - 8, skintone2);
  SetLED(x + 9, y - 8, outline);
  SetLED(x - 4, y - 7, outline); // START ROW -7
  SetLED(x - 3, y - 7, hat1);
  SetLights(x - 2, y - 7, 2, outline);
  SetLights(x + 0, y - 7, 6, skintone2);
  SetLights(x + 6, y - 7, 3, outline);
  SetLights(x - 5, y - 6, 4, outline); // START ROW -6
  SetLED(x - 1, y - 6, skintone2);
  SetLights(x + 0, y - 6, 2, skintone1);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone1);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone1);
  SetLED(x + 6, y - 6, outline);
  SetLED(x + 7, y - 6, hat2);
  SetLED(x + 8, y - 6, hat1);
  SetLED(x + 9, y - 6, outline);
  SetLED(x - 5, y - 5, outline); // START ROW -5
  SetLED(x - 4, y - 5, skintone1);
  SetLights(x - 3, y - 5, 2, outline);
  SetLED(x - 1, y - 5, skintone2);
  SetLights(x + 0, y - 5, 2, skintone1);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone1);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone1);
  SetLED(x + 6, y - 5, outline);
  SetLED(x + 7, y - 5, hat2);
  SetLED(x + 8, y - 5, hat1);
  SetLED(x + 9, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLights(x - 5, y - 4, 2, skintone1);
  SetLights(x - 3, y - 4, 3, outline);
  SetLED(x + 0, y - 4, skintone2);
  SetLights(x + 1, y - 4, 3, skintone1);
  SetLights(x + 4, y - 4, 3, skintone3);
  SetLED(x + 7, y - 4, outline);
  SetLED(x + 8, y - 4, hat1);
  SetLED(x + 9, y - 4, outline);
  SetLED(x - 6, y - 3, outline); // START ROW -3
  SetLED(x - 5, y - 3, skintone2);
  SetLED(x - 4, y - 3, skintone1);
  SetLED(x - 3, y - 3, skintone2);
  SetLED(x - 2, y - 3, outline);
  SetLED(x - 1, y - 3, skintone2);
  SetLED(x + 0, y - 3, skintone1);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 2, skintone2);
  SetLights(x + 4, y - 3, 3, skintone1);
  SetLED(x + 7, y - 3, outline);
  SetLED(x + 8, y - 3, hat1);
  SetLED(x + 9, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLights(x - 4, y - 2, 3, skintone2);
  SetLED(x - 1, y - 2, skintone1);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone2);
  SetLights(x + 6, y - 2, 3, outline);
  SetLights(x - 4, y - 1, 2, outline); // START ROW -1
  SetLights(x - 2, y - 1, 2, skintone2);
  SetLights(x + 0, y - 1, 2, skintone1);
  SetLights(x + 2, y - 1, 5, outline);
  SetLED(x + 7, y - 1, hat1);
  SetLED(x + 8, y - 1, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone2);
  SetLED(x + 5, y + 0, outline);
  SetLED(x + 6, y + 0, hat2);
  SetLED(x + 7, y + 0, hat1);
  SetLED(x + 8, y + 0, outline);
  SetLED(x - 4, y + 1, outline); // START ROW +1
  SetLights(x - 3, y + 1, 3, hat2);
  SetLights(x + 0, y + 1, 6, outline);
  SetLED(x + 6, y + 1, hat1);
  SetLED(x + 7, y + 1, outline);
  SetLED(x - 5, y + 2, outline); // START ROW +2
  SetLED(x - 4, y + 2, hat1);
  SetLED(x - 3, y + 2, hat2);
  SetLights(x - 2, y + 2, 2, hat3);
  SetLED(x + 0, y + 2, hat2);
  SetLights(x + 1, y + 2, 2, outline);
  SetLED(x + 3, y + 2, hat2);
  SetLED(x + 4, y + 2, hat3);
  SetLights(x + 5, y + 2, 2, outline);
  SetLED(x - 6, y + 3, outline); // START ROW +3
  SetLED(x - 5, y + 3, hat1);
  SetLED(x - 4, y + 3, hat2);
  SetLED(x - 3, y + 3, hat3);
  SetLights(x - 2, y + 3, 3, outline);
  SetLED(x + 1, y + 3, overalls2);
  SetLED(x + 2, y + 3, outline);
  SetLED(x + 3, y + 3, hat1);
  SetLED(x + 4, y + 3, hat2);
  SetLED(x + 5, y + 3, hat3);
  SetLights(x + 6, y + 3, 2, outline);
  SetLED(x - 6, y + 4, outline); // START ROW +4
  SetLights(x - 5, y + 4, 2, hat1);
  SetLED(x - 3, y + 4, outline);
  SetLights(x - 2, y + 4, 3, skintone1);
  SetLED(x + 1, y + 4, outline);
  SetLED(x + 2, y + 4, overalls2);
  SetLED(x + 3, y + 4, outline);
  SetLED(x + 4, y + 4, hat1);
  SetLights(x + 5, y + 4, 2, hat2);
  SetLED(x + 7, y + 4, outline);
  SetLED(x - 6, y + 5, outline); // START ROW +5
  SetLED(x - 5, y + 5, hat1);
  SetLED(x - 4, y + 5, outline);
  SetLED(x - 3, y + 5, skintone2);
  SetLED(x - 2, y + 5, skintone1);
  SetLights(x - 1, y + 5, 2, skintone3);
  SetLED(x + 1, y + 5, skintone1);
  SetLED(x + 2, y + 5, outline);
  SetLED(x + 3, y + 5, overalls3);
  SetLights(x + 4, y + 5, 5, outline);
  SetLights(x - 5, y + 6, 2, outline); // START ROW +6
  SetLights(x - 3, y + 6, 2, skintone2);
  SetLights(x - 1, y + 6, 3, skintone1);
  SetLED(x + 2, y + 6, outline);
  SetLights(x + 3, y + 6, 2, hat3);
  SetLights(x + 5, y + 6, 2, overalls3);
  SetLights(x + 7, y + 6, 2, outline);
  SetLights(x - 4, y + 7, 2, outline); // START ROW +7
  SetLights(x - 2, y + 7, 3, skintone2);
  SetLED(x + 1, y + 7, outline);
  SetLED(x + 2, y + 7, overalls2);
  SetLights(x + 3, y + 7, 2, hat3);
  SetLED(x + 5, y + 7, overalls3);
  SetLED(x + 6, y + 7, outline);
  SetLED(x + 7, y + 7, goldshoes);
  SetLED(x + 8, y + 7, shoes1);
  SetLED(x + 9, y + 7, outline);
  SetLED(x - 5, y + 8, outline); // START ROW +8
  SetLights(x - 4, y + 8, 2, shoes2);
  SetLights(x - 2, y + 8, 3, outline);
  SetLights(x + 1, y + 8, 4, overalls2);
  SetLED(x + 5, y + 8, outline);
  SetLED(x + 6, y + 8, goldshoes);
  SetLED(x + 7, y + 8, shoes1);
  SetLED(x + 8, y + 8, shoes2);
  SetLED(x + 9, y + 8, outline);
  SetLights(x - 6, y + 9, 2, outline); // START ROW +9
  SetLED(x - 4, y + 9, shoes2);
  SetLED(x - 3, y + 9, outline);
  SetLED(x - 2, y + 9, overalls1);
  SetLights(x - 1, y + 9, 3, overalls2);
  SetLights(x + 2, y + 9, 3, overalls1);
  SetLED(x + 5, y + 9, outline);
  SetLED(x + 6, y + 9, shoes1);
  SetLights(x + 7, y + 9, 2, shoes2);
  SetLED(x + 9, y + 9, outline);
  SetLED(x - 6, y + 10, outline); // START ROW +10
  SetLED(x - 5, y + 10, shoes2);
  SetLED(x - 4, y + 10, shoes1);
  SetLED(x - 3, y + 10, outline);
  SetLights(x - 2, y + 10, 4, overalls1);
  SetLights(x + 2, y + 10, 4, outline);
  SetLED(x + 6, y + 10, shoes1);
  SetLights(x + 7, y + 10, 2, shoes2);
  SetLED(x + 9, y + 10, outline);
  SetLED(x - 6, y + 11, outline); // START ROW +11
  SetLED(x - 5, y + 11, shoes2);
  SetLED(x - 4, y + 11, shoes1);
  SetLED(x - 3, y + 11, outline);
  SetLED(x - 2, y + 11, overalls1);
  SetLights(x - 1, y + 11, 3, outline);
  SetLED(x + 5, y + 11, outline);
  SetLED(x + 6, y + 11, shoes1);
  SetLED(x + 7, y + 11, shoes2);
  SetLights(x + 8, y + 11, 2, outline);
  SetLED(x - 6, y + 12, outline); // START ROW +12
  SetLED(x - 5, y + 12, shoes2);
  SetLED(x - 4, y + 12, shoes1);
  SetLights(x - 3, y + 12, 2, outline);
  SetLights(x + 6, y + 12, 3, outline);
  SetLights(x - 5, y + 13, 2, outline); // START ROW +13
}

void DrawMarioWarp09(int x, int y, byte outline = 34, byte skintone = 35, byte suit = 7)
{
  SetLights(x - 1, y - 13, 5, outline); // START ROW -13
  SetLights(x + 6, y - 13, 3, outline);
  SetLights(x - 2, y - 12, 7, outline); // START ROW -12
  SetLights(x + 6, y - 12, 3, skintone);
  SetLED(x + 9, y - 12, outline);
  SetLED(x - 3, y - 11, outline); // START ROW -11
  SetLED(x - 2, y - 11, suit);
  SetLights(x - 1, y - 11, 8, outline);
  SetLights(x + 7, y - 11, 2, skintone);
  SetLED(x + 9, y - 11, outline);
  SetLights(x - 3, y - 10, 3, outline); // START ROW -10
  SetLights(x + 0, y - 10, 7, suit);
  SetLED(x + 7, y - 10, outline);
  SetLED(x + 8, y - 10, skintone);
  SetLED(x + 9, y - 10, outline);
  SetLights(x - 4, y - 9, 2, outline); // START ROW -9
  SetLights(x - 2, y - 9, 9, suit);
  SetLED(x + 7, y - 9, outline);
  SetLED(x + 8, y - 9, skintone);
  SetLED(x + 9, y - 9, outline);
  SetLED(x - 4, y - 8, outline); // START ROW -8
  SetLights(x - 3, y - 8, 4, suit);
  SetLights(x + 1, y - 8, 5, outline);
  SetLED(x + 6, y - 8, suit);
  SetLights(x + 7, y - 8, 3, outline);
  SetLights(x - 4, y - 7, 2, outline); // START ROW -7
  SetLED(x - 2, y - 7, suit);
  SetLights(x - 1, y - 7, 2, outline);
  SetLights(x + 1, y - 7, 5, skintone);
  SetLED(x + 6, y - 7, outline);
  SetLights(x + 7, y - 7, 2, suit);
  SetLED(x + 9, y - 7, outline);
  SetLED(x - 5, y - 6, outline); // START ROW -6
  SetLights(x - 4, y - 6, 2, suit);
  SetLED(x - 2, y - 6, outline);
  SetLights(x - 1, y - 6, 3, skintone);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone);
  SetLED(x + 6, y - 6, outline);
  SetLights(x + 7, y - 6, 2, suit);
  SetLED(x + 9, y - 6, outline);
  SetLED(x - 5, y - 5, suit); // START ROW -5
  SetLED(x - 4, y - 5, outline);
  SetLights(x - 3, y - 5, 2, suit);
  SetLED(x - 1, y - 5, outline);
  SetLights(x + 0, y - 5, 2, skintone);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone);
  SetLED(x + 6, y - 5, outline);
  SetLights(x + 7, y - 5, 2, suit);
  SetLED(x + 9, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLED(x - 5, y - 4, suit);
  SetLED(x - 4, y - 4, outline);
  SetLights(x - 3, y - 4, 2, suit);
  SetLED(x - 1, y - 4, outline);
  SetLights(x + 0, y - 4, 7, skintone);
  SetLED(x + 7, y - 4, outline);
  SetLED(x + 8, y - 4, suit);
  SetLED(x + 9, y - 4, outline);
  SetLED(x - 6, y - 3, outline); // START ROW -3
  SetLED(x - 5, y - 3, suit);
  SetLED(x - 4, y - 3, outline);
  SetLights(x - 3, y - 3, 2, suit);
  SetLED(x - 1, y - 3, outline);
  SetLED(x + 0, y - 3, skintone);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 5, skintone);
  SetLED(x + 7, y - 3, outline);
  SetLED(x + 8, y - 3, suit);
  SetLED(x + 9, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLights(x - 4, y - 2, 2, suit);
  SetLED(x - 2, y - 2, outline);
  SetLED(x - 1, y - 2, skintone);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone);
  SetLights(x + 6, y - 2, 3, outline);
  SetLights(x - 4, y - 1, 2, outline); // START ROW -1
  SetLights(x - 2, y - 1, 4, skintone);
  SetLights(x + 2, y - 1, 5, outline);
  SetLED(x + 7, y - 1, suit);
  SetLED(x + 8, y - 1, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone);
  SetLED(x + 5, y + 0, outline);
  SetLights(x + 6, y + 0, 2, suit);
  SetLED(x + 8, y + 0, outline);
  SetLED(x - 4, y + 1, outline); // START ROW +1
  SetLights(x - 3, y + 1, 3, suit);
  SetLights(x + 0, y + 1, 6, outline);
  SetLED(x + 6, y + 1, suit);
  SetLED(x + 7, y + 1, outline);
  SetLED(x - 5, y + 2, outline); // START ROW +2
  SetLights(x - 4, y + 2, 5, suit);
  SetLights(x + 1, y + 2, 2, outline);
  SetLights(x + 3, y + 2, 2, suit);
  SetLights(x + 5, y + 2, 2, outline);
  SetLED(x - 6, y + 3, outline); // START ROW +3
  SetLights(x - 5, y + 3, 3, suit);
  SetLights(x - 2, y + 3, 3, outline);
  SetLED(x + 1, y + 3, suit);
  SetLED(x + 2, y + 3, outline);
  SetLights(x + 3, y + 3, 3, suit);
  SetLights(x + 6, y + 3, 2, outline);
  SetLED(x - 6, y + 4, outline); // START ROW +4
  SetLights(x - 5, y + 4, 2, suit);
  SetLED(x - 3, y + 4, outline);
  SetLights(x - 2, y + 4, 3, skintone);
  SetLights(x + 1, y + 4, 3, outline);
  SetLights(x + 4, y + 4, 3, suit);
  SetLED(x + 7, y + 4, outline);
  SetLED(x - 6, y + 5, outline); // START ROW +5
  SetLED(x - 5, y + 5, suit);
  SetLED(x - 4, y + 5, outline);
  SetLights(x - 3, y + 5, 5, skintone);
  SetLights(x + 2, y + 5, 3, outline);
  SetLights(x + 5, y + 5, 2, suit);
  SetLights(x + 7, y + 5, 2, outline);
  SetLights(x - 5, y + 6, 2, outline); // START ROW +6
  SetLights(x - 3, y + 6, 5, skintone);
  SetLED(x + 2, y + 6, outline);
  SetLights(x + 3, y + 6, 2, skintone);
  SetLights(x + 5, y + 6, 2, outline);
  SetLED(x + 7, y + 6, skintone);
  SetLED(x + 8, y + 6, outline);
  SetLights(x - 4, y + 7, 2, outline); // START ROW +7
  SetLights(x - 2, y + 7, 3, skintone);
  SetLights(x + 1, y + 7, 2, outline);
  SetLights(x + 3, y + 7, 2, skintone);
  SetLights(x + 5, y + 7, 2, outline);
  SetLights(x + 7, y + 7, 2, suit);
  SetLED(x + 9, y + 7, outline);
  SetLED(x - 5, y + 8, outline); // START ROW +8
  SetLights(x - 4, y + 8, 2, suit);
  SetLights(x - 2, y + 8, 8, outline);
  SetLights(x + 6, y + 8, 3, suit);
  SetLED(x + 9, y + 8, outline);
  SetLights(x - 6, y + 9, 2, outline); // START ROW +9
  SetLED(x - 4, y + 9, suit);
  SetLights(x - 3, y + 9, 9, outline);
  SetLights(x + 6, y + 9, 3, suit);
  SetLED(x + 9, y + 9, outline);
  SetLED(x - 6, y + 10, outline); // START ROW +10
  SetLights(x - 5, y + 10, 2, suit);
  SetLights(x - 3, y + 10, 9, outline);
  SetLights(x + 6, y + 10, 3, suit);
  SetLED(x + 9, y + 10, outline);
  SetLED(x - 6, y + 11, outline); // START ROW +11
  SetLights(x - 5, y + 11, 2, suit);
  SetLights(x - 3, y + 11, 5, outline);
  SetLED(x + 5, y + 11, outline);
  SetLights(x + 6, y + 11, 2, suit);
  SetLights(x + 8, y + 11, 2, outline);
  SetLED(x - 6, y + 12, outline); // START ROW +12
  SetLights(x - 5, y + 12, 2, suit);
  SetLights(x - 3, y + 12, 2, outline);
  SetLights(x + 6, y + 12, 3, outline);
  SetLights(x - 5, y + 13, 2, outline); // START ROW +13
}

void DrawMarioWarp10(int x, int y, byte outline = 0, byte hardhat1 = 41, byte hardhat2 = 42, byte hardhat3 = 43, byte skintone1 = 15, byte skintone2 = 14, byte skintone3 = 21, byte yellowhat2 = 44, byte yellowhat1 = 45, byte brownsuit1 = 46, byte brownsuit2 = 47, byte button = 16, byte brownsuit3 = 19)
{
  SetLights(x - 1, y - 13, 5, outline); // START ROW -13
  SetLights(x + 6, y - 13, 3, outline);
  SetLED(x - 2, y - 12, outline); // START ROW -12
  SetLights(x - 1, y - 12, 2, hardhat1);
  SetLED(x + 1, y - 12, hardhat2);
  SetLED(x + 2, y - 12, hardhat3);
  SetLED(x + 3, y - 12, hardhat1);
  SetLights(x + 4, y - 12, 2, outline);
  SetLED(x + 6, y - 12, skintone1);
  SetLED(x + 7, y - 12, skintone2);
  SetLED(x + 8, y - 12, skintone3);
  SetLED(x + 9, y - 12, outline);
  SetLED(x - 3, y - 11, outline); // START ROW -11
  SetLED(x - 2, y - 11, hardhat1);
  SetLED(x - 1, y - 11, hardhat2);
  SetLights(x + 0, y - 11, 7, outline);
  SetLED(x + 7, y - 11, skintone1);
  SetLED(x + 8, y - 11, skintone2);
  SetLED(x + 9, y - 11, outline);
  SetLights(x - 3, y - 10, 3, outline); // START ROW -10
  SetLights(x + 0, y - 10, 7, yellowhat2);
  SetLED(x + 7, y - 10, outline);
  SetLED(x + 8, y - 10, skintone1);
  SetLED(x + 9, y - 10, outline);
  SetLights(x - 4, y - 9, 2, outline); // START ROW -9
  SetLED(x - 2, y - 9, yellowhat1);
  SetLED(x - 1, y - 9, yellowhat2);
  SetLights(x + 0, y - 9, 6, yellowhat1);
  SetLED(x + 6, y - 9, yellowhat2);
  SetLED(x + 7, y - 9, outline);
  SetLED(x + 8, y - 9, skintone1);
  SetLED(x + 9, y - 9, outline);
  SetLED(x - 4, y - 8, outline); // START ROW -8
  SetLED(x - 3, y - 8, yellowhat1);
  SetLED(x - 2, y - 8, yellowhat2);
  SetLights(x - 1, y - 8, 2, yellowhat1);
  SetLights(x + 1, y - 8, 5, outline);
  SetLED(x + 6, y - 8, yellowhat1);
  SetLights(x + 7, y - 8, 3, outline);
  SetLights(x - 4, y - 7, 2, outline); // START ROW -7
  SetLED(x - 2, y - 7, yellowhat1);
  SetLights(x - 1, y - 7, 2, outline);
  SetLights(x + 1, y - 7, 5, skintone1);
  SetLED(x + 6, y - 7, outline);
  SetLED(x + 7, y - 7, brownsuit1);
  SetLED(x + 8, y - 7, brownsuit2);
  SetLED(x + 9, y - 7, outline);
  SetLED(x - 5, y - 6, outline); // START ROW -6
  SetLED(x - 4, y - 6, yellowhat2);
  SetLED(x - 3, y - 6, yellowhat1);
  SetLED(x - 2, y - 6, outline);
  SetLights(x - 1, y - 6, 2, skintone1);
  SetLED(x + 1, y - 6, skintone2);
  SetLED(x + 2, y - 6, outline);
  SetLED(x + 3, y - 6, skintone2);
  SetLED(x + 4, y - 6, outline);
  SetLED(x + 5, y - 6, skintone2);
  SetLED(x + 6, y - 6, outline);
  SetLED(x + 7, y - 6, brownsuit1);
  SetLED(x + 8, y - 6, brownsuit2);
  SetLED(x + 9, y - 6, outline);
  SetLED(x - 6, y - 5, outline); // START ROW -5
  SetLED(x - 5, y - 5, yellowhat1);
  SetLED(x - 4, y - 5, outline);
  SetLED(x - 3, y - 5, yellowhat2);
  SetLED(x - 2, y - 5, yellowhat1);
  SetLED(x - 1, y - 5, outline);
  SetLED(x + 0, y - 5, skintone1);
  SetLED(x + 1, y - 5, skintone2);
  SetLED(x + 2, y - 5, outline);
  SetLED(x + 3, y - 5, skintone2);
  SetLED(x + 4, y - 5, outline);
  SetLED(x + 5, y - 5, skintone2);
  SetLED(x + 6, y - 5, outline);
  SetLED(x + 7, y - 5, brownsuit1);
  SetLED(x + 8, y - 5, brownsuit2);
  SetLED(x + 9, y - 5, outline);
  SetLED(x - 6, y - 4, outline); // START ROW -4
  SetLED(x - 5, y - 4, yellowhat1);
  SetLED(x - 4, y - 4, outline);
  SetLED(x - 3, y - 4, yellowhat2);
  SetLED(x - 2, y - 4, yellowhat1);
  SetLED(x - 1, y - 4, outline);
  SetLED(x + 0, y - 4, skintone1);
  SetLights(x + 1, y - 4, 3, skintone2);
  SetLights(x + 4, y - 4, 3, skintone3);
  SetLED(x + 7, y - 4, outline);
  SetLED(x + 8, y - 4, brownsuit1);
  SetLED(x + 9, y - 4, outline);
  SetLED(x - 6, y - 3, outline); // START ROW -3
  SetLED(x - 5, y - 3, yellowhat1);
  SetLED(x - 4, y - 3, outline);
  SetLED(x - 3, y - 3, yellowhat2);
  SetLED(x - 2, y - 3, yellowhat1);
  SetLED(x - 1, y - 3, outline);
  SetLED(x + 0, y - 3, skintone1);
  SetLED(x + 1, y - 3, outline);
  SetLights(x + 2, y - 3, 2, skintone1);
  SetLights(x + 4, y - 3, 3, skintone2);
  SetLED(x + 7, y - 3, outline);
  SetLED(x + 8, y - 3, brownsuit1);
  SetLED(x + 9, y - 3, outline);
  SetLED(x - 5, y - 2, outline); // START ROW -2
  SetLED(x - 4, y - 2, yellowhat2);
  SetLED(x - 3, y - 2, yellowhat1);
  SetLED(x - 2, y - 2, outline);
  SetLED(x - 1, y - 2, skintone1);
  SetLights(x + 0, y - 2, 4, outline);
  SetLights(x + 4, y - 2, 2, skintone1);
  SetLights(x + 6, y - 2, 3, outline);
  SetLights(x - 4, y - 1, 2, outline); // START ROW -1
  SetLights(x - 2, y - 1, 2, skintone1);
  SetLights(x + 0, y - 1, 2, skintone2);
  SetLights(x + 2, y - 1, 5, outline);
  SetLED(x + 7, y - 1, brownsuit2);
  SetLED(x + 8, y - 1, outline);
  SetLights(x - 3, y + 0, 3, outline); // START ROW +0
  SetLights(x + 0, y + 0, 5, skintone1);
  SetLED(x + 5, y + 0, outline);
  SetLights(x + 6, y + 0, 2, brownsuit1);
  SetLED(x + 8, y + 0, outline);
  SetLED(x - 4, y + 1, outline); // START ROW +1
  SetLights(x - 3, y + 1, 3, brownsuit2);
  SetLights(x + 0, y + 1, 6, outline);
  SetLED(x + 6, y + 1, brownsuit1);
  SetLED(x + 7, y + 1, outline);
  SetLED(x - 5, y + 2, outline); // START ROW +2
  SetLED(x - 4, y + 2, brownsuit1);
  SetLED(x - 3, y + 2, brownsuit2);
  SetLights(x - 2, y + 2, 2, yellowhat1);
  SetLED(x + 0, y + 2, brownsuit2);
  SetLights(x + 1, y + 2, 2, outline);
  SetLED(x + 3, y + 2, brownsuit2);
  SetLED(x + 4, y + 2, yellowhat1);
  SetLights(x + 5, y + 2, 2, outline);
  SetLED(x - 6, y + 3, outline); // START ROW +3
  SetLED(x - 5, y + 3, brownsuit1);
  SetLED(x - 4, y + 3, brownsuit2);
  SetLED(x - 3, y + 3, yellowhat1);
  SetLights(x - 2, y + 3, 3, outline);
  SetLED(x + 1, y + 3, hardhat2);
  SetLED(x + 2, y + 3, outline);
  SetLED(x + 3, y + 3, brownsuit1);
  SetLED(x + 4, y + 3, brownsuit2);
  SetLED(x + 5, y + 3, yellowhat1);
  SetLights(x + 6, y + 3, 2, outline);
  SetLED(x - 6, y + 4, outline); // START ROW +4
  SetLights(x - 5, y + 4, 2, brownsuit1);
  SetLED(x - 3, y + 4, outline);
  SetLights(x - 2, y + 4, 3, skintone2);
  SetLED(x + 1, y + 4, outline);
  SetLED(x + 2, y + 4, hardhat2);
  SetLED(x + 3, y + 4, outline);
  SetLED(x + 4, y + 4, brownsuit1);
  SetLights(x + 5, y + 4, 2, brownsuit2);
  SetLED(x + 7, y + 4, outline);
  SetLED(x - 6, y + 5, outline); // START ROW +5
  SetLED(x - 5, y + 5, brownsuit1);
  SetLED(x - 4, y + 5, outline);
  SetLED(x - 3, y + 5, skintone1);
  SetLED(x - 2, y + 5, skintone2);
  SetLights(x - 1, y + 5, 2, skintone3);
  SetLED(x + 1, y + 5, skintone2);
  SetLED(x + 2, y + 5, outline);
  SetLED(x + 3, y + 5, hardhat3);
  SetLights(x + 4, y + 5, 5, outline);
  SetLights(x - 5, y + 6, 2, outline); // START ROW +6
  SetLights(x - 3, y + 6, 2, skintone1);
  SetLights(x - 1, y + 6, 3, skintone2);
  SetLED(x + 2, y + 6, outline);
  SetLights(x + 3, y + 6, 2, button);
  SetLights(x + 5, y + 6, 2, hardhat3);
  SetLED(x + 7, y + 6, button);
  SetLED(x + 8, y + 6, outline);
  SetLights(x - 4, y + 7, 2, outline); // START ROW +7
  SetLights(x - 2, y + 7, 3, skintone1);
  SetLED(x + 1, y + 7, outline);
  SetLED(x + 2, y + 7, hardhat2);
  SetLights(x + 3, y + 7, 2, button);
  SetLED(x + 5, y + 7, hardhat3);
  SetLED(x + 6, y + 7, outline);
  SetLED(x + 7, y + 7, yellowhat1);
  SetLED(x + 8, y + 7, brownsuit3);
  SetLED(x + 9, y + 7, outline);
  SetLED(x - 5, y + 8, outline); // START ROW +8
  SetLights(x - 4, y + 8, 2, brownsuit1);
  SetLights(x - 2, y + 8, 3, outline);
  SetLights(x + 1, y + 8, 4, hardhat2);
  SetLED(x + 5, y + 8, outline);
  SetLED(x + 6, y + 8, yellowhat1);
  SetLED(x + 7, y + 8, brownsuit3);
  SetLED(x + 8, y + 8, brownsuit1);
  SetLED(x + 9, y + 8, outline);
  SetLights(x - 6, y + 9, 2, outline); // START ROW +9
  SetLED(x - 4, y + 9, brownsuit1);
  SetLED(x - 3, y + 9, outline);
  SetLED(x - 2, y + 9, hardhat1);
  SetLights(x - 1, y + 9, 3, hardhat2);
  SetLights(x + 2, y + 9, 3, hardhat1);
  SetLED(x + 5, y + 9, outline);
  SetLED(x + 6, y + 9, brownsuit3);
  SetLights(x + 7, y + 9, 2, brownsuit1);
  SetLED(x + 9, y + 9, outline);
  SetLED(x - 6, y + 10, outline); // START ROW +10
  SetLED(x - 5, y + 10, brownsuit1);
  SetLED(x - 4, y + 10, brownsuit3);
  SetLED(x - 3, y + 10, outline);
  SetLights(x - 2, y + 10, 4, hardhat1);
  SetLights(x + 2, y + 10, 4, outline);
  SetLED(x + 6, y + 10, brownsuit3);
  SetLights(x + 7, y + 10, 2, brownsuit1);
  SetLED(x + 9, y + 10, outline);
  SetLED(x - 6, y + 11, outline); // START ROW +11
  SetLED(x - 5, y + 11, brownsuit1);
  SetLED(x - 4, y + 11, brownsuit3);
  SetLED(x - 3, y + 11, outline);
  SetLED(x - 2, y + 11, hardhat1);
  SetLights(x - 1, y + 11, 3, outline);
  SetLED(x + 5, y + 11, outline);
  SetLED(x + 6, y + 11, brownsuit3);
  SetLED(x + 7, y + 11, brownsuit1);
  SetLights(x + 8, y + 11, 2, outline);
  SetLED(x - 6, y + 12, outline); // START ROW +12
  SetLED(x - 5, y + 12, brownsuit1);
  SetLED(x - 4, y + 12, brownsuit3);
  SetLights(x - 3, y + 12, 2, outline);
  SetLights(x + 6, y + 12, 3, outline);
  SetLights(x - 5, y + 13, 2, outline); // START ROW +13
}

void MarioWarpThrough()
{
  Serial.println("MarioWarpThrough()");
  int frame = GetFrame(56, 200);
  int frameIndex = (frame % 40) / 4;
  static int framePrint = -1;

  SetStrip(CRGB(GetColor(8, 0), GetColor(8, 1), GetColor(8, 2)));
  
  if      (frameIndex == 0) DrawMarioWarp09(frame - 14, 14);
  else if (frameIndex == 1) DrawMarioWarp10(frame - 14, 14);
  else if (frameIndex == 2) DrawMarioWarp01(frame - 14, 14);
  else if (frameIndex == 3) DrawMarioWarp02(frame - 14, 14);
  else if (frameIndex == 4) DrawMarioWarp07(frame - 14, 14);
  else if (frameIndex == 5) DrawMarioWarp08(frame - 14, 14);
  else if (frameIndex == 6) DrawMarioWarp05(frame - 14, 14);
  else if (frameIndex == 7) DrawMarioWarp06(frame - 14, 14);
  else if (frameIndex == 8) DrawMarioWarp03(frame - 14, 14);
  else                      DrawMarioWarp04(frame - 14, 14);
}

void DetermineNextTetrisShape(byte& nextShape, byte& nextColor)
{
  RenderTetrisShape(nextShape, 0, 0, 22, 6);
  nextShape = random(4);
  nextColor = GetRandomVibrantColorIndex();
}

void IterateCurrentTetrisShape(byte& currentShape, byte& currentColor, byte &nextShape, byte& nextColor)
{
  currentShape = nextShape;
  currentColor = nextColor;
  DetermineNextTetrisShape(nextShape, nextColor);
}

void RenderTetrisShape(byte shapeIndex, byte color, byte facing, byte xPos, byte yPos)
{
  switch (shapeIndex)
  {
    case 0: //  SQUARE
      SetLights(xPos, yPos, 2, color); SetLights(xPos, yPos + 1, 2, color);
      break;
    case 1: //  LINE
      switch (facing)
      {
        case 0: //  000 Degrees
        case 2: //  180 Degrees
          SetLights(xPos - 1, yPos + 0, 4, color);
          break;
        case 1: //  090 Degrees
        case 3: //  270 Degrees
          SetLED(xPos + 0, yPos - 1, color);
          SetLED(xPos + 0, yPos + 0, color);
          SetLED(xPos + 0, yPos + 1, color);
          SetLED(xPos + 0, yPos + 2, color);
          break;
      }
      break;
    case 2: //  L-BLOCK
      switch (facing)
      {
        case 0:
          SetLights(xPos - 1, yPos + 0, 3, color);
          SetLED(xPos - 1, yPos + 1, color);
          break;
        case 1:
          SetLights(xPos - 1, yPos - 1, 2, color);
          SetLED(xPos + 0, yPos + 0, color);
          SetLED(xPos + 0, yPos + 1, color);
          break;
        case 2:
          SetLights(xPos - 1, yPos + 0, 3, color);
          SetLED(xPos + 1, yPos - 1, color);
          break;
        case 3:
          SetLED(xPos + 0, yPos - 1, color);
          SetLED(xPos + 0, yPos + 0, color);
          SetLights(xPos + 0, yPos + 1, 2, color);
          break;
      }
      break;
    case 3: //  REVERSE L-BLOCK
      switch (facing)
      {
        case 0:
          SetLights(xPos - 1, yPos + 0, 3, color);
          SetLED(xPos + 1, yPos + 1, color);
          break;
        case 1:
          SetLED(xPos + 0, yPos - 1, color);
          SetLED(xPos + 0, yPos + 0, color);
          SetLights(xPos - 1, yPos + 1, 2, color);
          break;
        case 2:
          SetLED(xPos - 1, yPos - 1, color);
          SetLights(xPos - 1, yPos + 0, 3, color);
          break;
        case 3:
          SetLights(xPos + 0, yPos - 1, 2, color);
          SetLED(xPos + 0, yPos + 0, color);
          SetLED(xPos + 0, yPos + 1, color);
          break;
      }
      break;
  }
}

bool CheckTetrisSpace(byte xPos, byte yPos, byte shape, byte facing)
{
  switch (shape)
  {
    case 0: //  SQUARE
      return (IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 1, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 1) && IsPixelBlack(xPos + 1, yPos + 1));
      break;
    case 1: //  LINE
      switch (facing)
      {
        case 0: //  000 Degrees
        case 2: //  180 Degrees
          return (IsPixelBlack(xPos - 1, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 1, yPos + 0) && IsPixelBlack(xPos + 2, yPos + 0));
          break;
        case 1: //  090 Degrees
        case 3: //  270 Degrees
          return (IsPixelBlack(xPos + 0, yPos - 1) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 1) && IsPixelBlack(xPos + 0, yPos + 2));
          break;
      }
      break;
    case 2: //  L-BLOCK
      switch (facing)
      {
        case 0:
          return (IsPixelBlack(xPos - 1, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos - 1, yPos + 1));
          break;
        case 1:
          return (IsPixelBlack(xPos - 1, yPos - 1) && IsPixelBlack(xPos + 0, yPos - 1) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 1));
          break;
        case 2:
          return (IsPixelBlack(xPos - 1, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 1, yPos + 0) && IsPixelBlack(xPos + 1, yPos - 1));
          break;
        case 3:
          return (IsPixelBlack(xPos + 0, yPos - 1) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 1) && IsPixelBlack(xPos + 1, yPos + 1));
          break;
      }
      break;
    case 3: //  REVERSE L-BLOCK
      switch (facing)
      {
        case 0:
          return (IsPixelBlack(xPos - 1, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 1, yPos + 1));
          break;
        case 1:
          return (IsPixelBlack(xPos + 0, yPos - 1) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos - 1, yPos + 1) && IsPixelBlack(xPos + 0, yPos + 1));
          break;
        case 2:
          return (IsPixelBlack(xPos - 1, yPos - 1) && IsPixelBlack(xPos - 1, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 1, yPos + 0));
          break;
        case 3:
          return (IsPixelBlack(xPos + 0, yPos - 1) && IsPixelBlack(xPos + 1, yPos - 1) && IsPixelBlack(xPos + 0, yPos + 0) && IsPixelBlack(xPos + 0, yPos + 1));
          break;
      }
      break;
  }
  
  return true;
}

void DropTetrisRow(byte rowIndex)
{
  for (int i = 0; i < rowIndex; ++i)
    for (int j = 0; j < TETRIS_BOARD_WIDTH; ++j)
      SetLED(TETRIS_BOARD_START_X + j, TETRIS_BOARD_START_Y + rowIndex - i, GetPixel(TETRIS_BOARD_START_X + j, TETRIS_BOARD_START_Y + rowIndex - i - 1));
}

void CheckForTetrisRowCleared()
{
  for (int i = 0; i < TETRIS_BOARD_HEIGHT; ++i)
  {
    for (int j = 0; j < TETRIS_BOARD_WIDTH; ++j)
    {
      if (IsPixelBlack(TETRIS_BOARD_START_X + j, TETRIS_BOARD_START_Y + i)) break;
      if (j == TETRIS_BOARD_WIDTH - 1)
      {
        DropTetrisRow(i);
      }
    }
  }
}

void ShowTetrisFailureAnimation()
{
  for (int i = 0; i < TETRIS_BOARD_HEIGHT; ++i)
    for (int j = 0; j < TETRIS_BOARD_WIDTH; ++j)
      if (!IsPixelBlack(TETRIS_BOARD_START_X + j, TETRIS_BOARD_START_Y + i))
        SetLED(TETRIS_BOARD_START_X + j, TETRIS_BOARD_START_Y + i, 7);

  FastLED.show();
  delay(1000);
  UpdateMillisOffset();
  ResetTetris();
}

void Tetris()
{
  static byte currentShape = 255;
  static byte currentColor = 0;
  static byte currentFacing = 0;
  
  static byte nextShape = 255;
  static byte nextColor = 0;
  
  static byte xPos = 4;
  static byte yPos = 1;

  if (nextShape == 255) DetermineNextTetrisShape(nextShape, nextColor);  
  if (currentShape == 255)
  {
    currentFacing = 0;
    IterateCurrentTetrisShape(currentShape, currentColor, nextShape, nextColor);
    xPos = 4;
    yPos = 1;
  }

  RenderTetrisShape(nextShape, nextColor, 0, 22, 6);
  RenderTetrisShape(currentShape, currentColor, currentFacing, TETRIS_BOARD_START_X + xPos, TETRIS_BOARD_START_Y + yPos);

  if (IsNextFrameReady())
  {
    RenderTetrisShape(currentShape, 0, currentFacing, TETRIS_BOARD_START_X + xPos, TETRIS_BOARD_START_Y + yPos);

    //  Move left or right if the controller is polled with LEFT or RIGHT input
    if (bitRead(NESRegister, LEFT_BUTTON) == 0)
    {
      if (CheckTetrisSpace(TETRIS_BOARD_START_X + xPos - 1, TETRIS_BOARD_START_Y + yPos, currentShape, currentFacing)) xPos -= 1;
    }
    if (bitRead(NESRegister, RIGHT_BUTTON) == 0)
    {
      if (CheckTetrisSpace(TETRIS_BOARD_START_X + xPos + 1, TETRIS_BOARD_START_Y + yPos, currentShape, currentFacing)) xPos += 1;
    }
    if (bitRead(NESRegister, UP_BUTTON) == 0)
    {
      int newFacing = currentFacing + 1;
      if (newFacing == 4) newFacing = 0;
      if (CheckTetrisSpace(TETRIS_BOARD_START_X + xPos, TETRIS_BOARD_START_Y + yPos + 1, currentShape, newFacing)) currentFacing = newFacing;
    }
    if (bitRead(NESRegister, DOWN_BUTTON) == 0)
    {
      if (CheckTetrisSpace(TETRIS_BOARD_START_X + xPos, TETRIS_BOARD_START_Y + yPos + 1, currentShape, currentFacing)) yPos += 2;
    }

    //  Move down if possible. If not, leave the shape and move on the next shape
    if (CheckTetrisSpace(TETRIS_BOARD_START_X + xPos, TETRIS_BOARD_START_Y + yPos + 1, currentShape, currentFacing)) yPos += 1;
    else
    {
      RenderTetrisShape(currentShape, currentColor, currentFacing, TETRIS_BOARD_START_X + xPos, TETRIS_BOARD_START_Y + yPos);
      CheckForTetrisRowCleared();

      IterateCurrentTetrisShape(currentShape, currentColor, nextShape, nextColor);
      xPos = 4;
      yPos = 1;
      currentFacing = 0;
      if (!CheckTetrisSpace(TETRIS_BOARD_START_X + xPos, TETRIS_BOARD_START_Y + yPos, currentShape, currentFacing)) ShowTetrisFailureAnimation();
    }
    nextFrameMillis += 250;
  }
}

inline void IteratePatternIndex(byte overrideIndex = 255)
{
    ClearStrip();
    if (overrideIndex == 255) ++patternIndex;
    else patternIndex = overrideIndex;
    UpdateMillisOffset();
    if (patternIndex == 0) ResetTetris();
}

byte readNesController() 
{  
  // Pre-load a variable with all 1's which assumes all buttons are not
  // pressed. But while we cycle through the bits, if we detect a LOW, which is
  // a 0, we clear that bit. In the end, we find all the buttons states at once.
  NESRegister = 255;
    
  // Quickly pulse the NES_LATCH_PIN pin so that the register grab what it see on
  // its parallel data pins.
  digitalWrite(NES_LATCH_PIN, HIGH);
  digitalWrite(NES_LATCH_PIN, LOW);
 
  // Upon latching, the first bit is available to look at, which is the state
  // of the A button. We see if it is low, and if it is, we clear out variable's
  // first bit to indicate this is so.
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, A_BUTTON);
    
  // Clock the next bit which is the B button and determine its state just like
  // we did above.
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, B_BUTTON);
  
  // Now do this for the rest of them!
  
  // Select button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, SELECT_BUTTON);

  // Start button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, START_BUTTON);

  // Up button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, UP_BUTTON);
    
  // Down button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, DOWN_BUTTON);

  // Left button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, LEFT_BUTTON);  
    
  // Right button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, RIGHT_BUTTON);
}

void setup()
{
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // connect internal pull-up
  
  //  Setup the LED strip and color all LEDs black
  if (GHOST_CART) FastLED.addLeds<WS2801, LED_DATA_PIN, LED_CLOCK_PIN, RGB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
  else            FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  ClearStrip();
  FastLED.show();
  
  //  Seed the random number generator
  randomSeed(analogRead(0));
  
  //  Set the button timer to the current time
  buttonTimer = millis();

  randomSeed(analogRead(0));

  //  NES CONTROLLER CODE
  // Set appropriate pins to inputs
  pinMode(NES_DATA_PIN, INPUT);
  
  // Set appropriate pins to outputs
  pinMode(NES_CLOCK_PIN, OUTPUT);
  pinMode(NES_LATCH_PIN, OUTPUT);
  
  // Set initial states
  digitalWrite(NES_CLOCK_PIN, LOW);
  digitalWrite(NES_LATCH_PIN, LOW);
  //  NES CONTROLLER CODE

  Serial.begin(9600);
  while (!Serial) { ; }
  Serial.println("Program START!");
  
  IteratePatternIndex(0);
}

void loop()
{
  //  Note: Comment this in ONLY if you have a potentiometer attached to POTENTIOMETER_PIN
  /*
  int newDelayTime = map(analogRead(POTENTIOMETER_PIN), 0, 1023, 1, 50);
  if (abs(newDelayTime - DELAY_TIME) >= 2)
  {
    DELAY_TIME = newDelayTime;
    Serial.print("New DELAY_TIME: ");
    Serial.println(DELAY_TIME);
  }
  */

  //  Note: Use this code to grab data from the NES Controller
  /*
  if (bitRead(NESRegister, A_BUTTON) == 0) Serial.println("A");
  if (bitRead(NESRegister, B_BUTTON) == 0) Serial.println("B");
  if (bitRead(NESRegister, START_BUTTON) == 0) Serial.println("START");
  if (bitRead(NESRegister, SELECT_BUTTON) == 0) Serial.println("SELECT");
  if (bitRead(NESRegister, UP_BUTTON) == 0) Serial.println("UP");
  if (bitRead(NESRegister, DOWN_BUTTON) == 0) Serial.println("DOWN");
  if (bitRead(NESRegister, LEFT_BUTTON) == 0) Serial.println("LEFT");  
  if (bitRead(NESRegister, RIGHT_BUTTON) == 0) Serial.println("RIGHT");
   */
  
  readNesController();
  
  unsigned long currentMillis = millis();
  if (buttonTimer < currentMillis)
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      buttonTimer = currentMillis + BUTTON_DELAY;
      IteratePatternIndex();
    }
  }

  if (patternIndex != 0) ClearStrip(); // Don't clear
  switch (patternIndex)
  {
    case 0:   Tetris();                                         break;
    case 1:   RainbowFlow1();                                   break;
    case 2:   RainbowFlow2(1, true);                            break;
    case 3:   ColorFire();                                      break;
    case 4:   GlowFlow(10, 100);                                break;
    case 5:   PacManChompDanceThrough();                        break;
    case 6:   PacManChompDanceThroughPlusGhost();               break;
    case 7:   MsPacManChompDanceThrough();                      break;
    case 8:   SpaceInvaderDanceThrough();                       break;
    case 9:
    {
      byte color1 = random(1, COMMON_COLOR_COUNT);
      byte color2 = random(1, COMMON_COLOR_COUNT);
      byte color3 = random(1, COMMON_COLOR_COUNT);
      ThreeSpaceInvaderDanceThrough(color1, color1, color2, color2, color3, color3); //  ColorBurst Invaders
      break;
    }
    case 10:  ThreeSpaceInvaderDanceThrough();                  break;
    case 11:  ThreeSpaceInvaderDanceThrough(2, 1, 2, 3, 2, 4);  break; // <= Ninja Turtles Invaders
    case 12:  MegaManRunThrough();                              break;
    case 13:  MarioWarpThrough();                               break;
    default:  patternIndex = 0;                                 break;
  }
  FastLED.show();
}
