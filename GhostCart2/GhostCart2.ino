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

#define GHOST_CART              false

int DELAY_TIME = 25;
unsigned long startTime = 0;
unsigned long buttonTimer = 0;

//  Generic values
#define FRAME_MILLIS            333
#define BUTTON_DELAY            400

CRGB leds[MAX_LEDS];
int position = 0;

int LoopedLEDIndex(int index)
{
  index = index % MAX_LEDS;
  while (index < 0) index = (MAX_LEDS + index) % MAX_LEDS;
  return index;
}

struct Color
{
private:
  int R, G, B;
public:
  Color(int r, int g, int b) : R(r), G(g), B(b) {}
  inline bool isZero()    const { return (R == 0 && G == 0 && B == 0);  }
};

//  The basic colors to warp through for GlowFlow
#define COMMON_COLOR_COUNT    19
const byte COMMON_COLORS[COMMON_COLOR_COUNT][3] = 
{
  { 0, 0, 0 },          //  Black
  { 255, 0, 0 },        //  Red
  { 0, 255, 0 },        //  Green
  { 0, 0, 255 },        //  Blue
  { 200, 200, 0 },      //  Yellow
  { 0, 255, 255 },      //  Cyan
  { 255, 0, 255 },      //  Magenta
  { 255, 255, 255 },    //  White
  { 100, 100, 100 },      //  Pale Red (MegaMan BG?)
  { 67, 79, 254 },      //  MegaMan Blue 1
  { 36, 242, 205 },     //  MegaMan Blue 2
  { 251, 236, 154 },    //  MegaMan Skin
////////// UNUSED COLORS BELOW
  { 255, 165, 0 },  //  Orange
  { 034, 139, 034 },  //  Forest Green
  { 0, 250, 154 },  //  Medium Spring Green
  { 032, 178, 170 },  //  Light Sea Green
  { 147, 112, 219 },  //  Medium Purple
  { 0, 100, 0 },    //  Dark Green
  { 070, 130, 180 },  //  Steel Blue
};

inline void SetStrip(CRGB color) { fill_solid(leds, MAX_LEDS, color); }
inline void ClearStrip() { SetStrip(CRGB::Black); }
inline bool IsPositionOnStrip(int pos) { return ((pos >= 0) && (pos < NUM_LEDS)); }
inline bool IsPositionOnStrip(int x, int y) { return ((x >= 0 && x < PANEL_WIDTH) && (y >= 0 && y < PANEL_HEIGHT) && (((y * PANEL_WIDTH + x) >= 0) && ((y * PANEL_WIDTH + x) < NUM_LEDS))); }
inline int PositionTranslate(int pos) { return (((pos / PANEL_WIDTH) & 1) ? (((pos / PANEL_WIDTH + 1) * PANEL_WIDTH) - 1 - (pos % PANEL_WIDTH)) : pos); }
inline int PositionTranslate(int x, int y) { int pos = (y * PANEL_WIDTH + x); return (((pos / PANEL_WIDTH) & 1) ? (((pos / PANEL_WIDTH + 1) * PANEL_WIDTH) - 1 - (pos % PANEL_WIDTH)) : pos); }
inline byte GetColor(byte colorIndex, int part) { return COMMON_COLORS[colorIndex][part]; }
inline int GetFrame(int modulus) { return (((millis() - startTime) / FRAME_MILLIS) % modulus); }
inline int GetFrame(int modulus, int frameMillis) { return (((millis() - startTime) / frameMillis) % modulus); }

inline void SetLED(int pos, byte color)
{
  if (IsPositionOnStrip(PositionTranslate(pos)) == false) return;
  
  leds[PositionTranslate(pos)] = CRGB(GetColor(color, 0), GetColor(color, 1), GetColor(color, 2));
}

inline void SetLED(int x, int y, byte color)
{
  if (IsPositionOnStrip(x, y) == false) return;
  
  leds[PositionTranslate(x, y)] = CRGB(GetColor(color, 0), GetColor(color, 1), GetColor(color, 2));
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

void PacManChompDanceThrough()
{
  Serial.println("PacManChompDanceThrough()");
  int frame = GetFrame(46, 120);
  
  if (frame % 8 == 0)       DrawPacManChomp01(frame - 6, 14, 4);
  else if (frame % 8 == 1)  DrawPacManChomp02(frame - 6, 14, 4);
  else if (frame % 8 == 2)  DrawPacManChomp02(frame - 6, 14, 4);
  else if (frame % 8 == 3)  DrawPacManChomp03(frame - 6, 14, 4);
  else if (frame % 8 == 4)  DrawPacManChomp03(frame - 6, 14, 4);
  else if (frame % 8 == 5)  DrawPacManChomp02(frame - 6, 14, 4);
  else if (frame % 8 == 6)  DrawPacManChomp02(frame - 6, 14, 4);
  else                      DrawPacManChomp01(frame - 6, 14, 4);
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
  else if (frame % 8 == 1)  DrawMsPacManChomp02(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 2)  DrawMsPacManChomp02(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 3)  DrawMsPacManChomp03(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 4)  DrawMsPacManChomp03(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 5)  DrawMsPacManChomp02(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else if (frame % 8 == 6)  DrawMsPacManChomp02(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
  else                      DrawMsPacManChomp01(PANEL_WIDTH - frame + 6, 14, 4, 1, 3);
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
  SetLED(x + 6, y - 3, skin);
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

void MegaManRunThrough()
{
  Serial.println("MegaManRunThrough()");
  int frame = GetFrame(52, 120);

  SetStrip(CRGB(GetColor(8, 0), GetColor(8, 1), GetColor(8, 2)));
  
  if (frame % 3 == 0)       DrawMegaManRun01(PANEL_WIDTH - frame + 14, 14, 0, 9, 10, 11, 7);
  else if (frame % 3 == 1)  DrawMegaManRun01(PANEL_WIDTH - frame + 14, 14, 0, 9, 10, 11, 7);
  else                      DrawMegaManRun01(PANEL_WIDTH - frame + 14, 14, 0, 9, 10, 11, 7);
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

  Serial.begin(9600);
  while (!Serial) { ; }
  Serial.println("Program START!");
}

void loop()
{
  static long int PatternCount = 17;
  static long int patternIndex = 7;

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
  
  unsigned long currentMillis = millis();
  if (buttonTimer < currentMillis)
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      buttonTimer = currentMillis + BUTTON_DELAY;
      ClearStrip();
      ++patternIndex;
    }
  }

  ClearStrip();
  switch (patternIndex)
  {
    case 0:   TestAnimation();                                  break;
    case 1:   PacManChompDanceThrough();                        break;
    case 2:   SpaceInvaderDanceThrough();                       break;
    case 3:
    {
      byte color1 = random(1, 19);
      byte color2 = random(1, 19);
      byte color3 = random(1, 19);
      ThreeSpaceInvaderDanceThrough(color1, color1, color2, color2, color3, color3); //  ColorBurst Invaders
      break;
    }
    case 4:   ThreeSpaceInvaderDanceThrough();                  break;
    case 5:   ThreeSpaceInvaderDanceThrough(2, 1, 2, 3, 2, 4);  break; // <= Ninja Turtles Invaders
    case 6:   MsPacManChompDanceThrough();                      break;
    case 7:   MegaManRunThrough();                              break;
    default:  patternIndex = 0;                                 break;
  }
  FastLED.show();
}
