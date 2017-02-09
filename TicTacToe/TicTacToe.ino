#include <FastLED.h>

#define COLOR_INDICATOR_PIN     5   //  The NeoPixel string data pin
#define COLOR_INDICATOR_R_PIN   A1  //  The NeoPixel string data pin
#define COLOR_INDICATOR_G_PIN   A0  //  The NeoPixel string data pin
#define COLOR_INDICATOR_B_PIN   A2  //  The NeoPixel string data pin
#define LED_STRIP_PIN           3   //  The NeoPixel string data pin
#define SWITCH_POS_BUTTON_PIN   12  //  The button to switch current position
#define PLACE_SYMBOL_BUTTON_PIN 7   //  The button to place your symbol down
#define SWITCH_COLOR_BUTTON_PIN 8   //  The button to change player color 

#define NUM_LEDS        9  //  The number of LEDs we want to alter
#define MAX_LEDS        9  //  The number of LEDs on the full strip
#define BRIGHTNESS      60  //  The number (0 to 200) for the brightness setting)
#define DEBUG_OUTPUT    false

#define BUTTON_DELAY            250

CRGB leds[MAX_LEDS];

char BoardPosition[3][3];
char DigitalBoardPosition[3][3];
bool FirstPlayerTurn;
int Player1Color;
int Player2Color;
int PositionSelected;
int DigitalPosition;
int CurrentX;
int CurrentY;

const int PlayerColorCount = 8;
const CRGB PlayerColors[PlayerColorCount] = 
{
  CRGB::Red,
  CRGB::Aqua,
  CRGB::Green,
  CRGB::Yellow,
  CRGB::Magenta,
  CRGB::White,
  CRGB::Orange,
  CRGB::Purple
};

// Fill the dots one after the other with a color
void fillColor(CRGB color, bool fullStrip = false)
{
#if DEBUG_OUTPUT
  Serial.print(color[0]);
  Serial.print(" - ");
  Serial.print(color[1]);
  Serial.print(" - ");
  Serial.print(color[2]);
  Serial.print("\n");
  delay(5);
#endif

  for( int j = 0; j < (fullStrip ? MAX_LEDS : NUM_LEDS); j++) leds[j] = color;
  FastLED.show(); // display this frame
}

void Blink(CRGB color, int milliWait = 1000)
{
  fillColor(color);
  delay(milliWait);
  fillColor(CRGB::Black);
}

void DebugOutputBoard()
{
  Serial.print("Board: { ");
  Serial.print((int)(BoardPosition[0][0]));
  Serial.print(", ");
  Serial.print((int)(BoardPosition[1][0]));
  Serial.print(", ");
  Serial.print((int)(BoardPosition[2][0]));
  Serial.print(", ");
  Serial.print((int)(BoardPosition[0][1]));
  Serial.print(", ");
  Serial.print((int)(BoardPosition[1][1]));
  Serial.print(", ");
  Serial.print((int)(BoardPosition[2][1]));
  Serial.print(", ");
  Serial.print((int)(BoardPosition[0][2]));
  Serial.print(", ");
  Serial.print((int)(BoardPosition[1][2]));
  Serial.print(", ");
  Serial.print((int)(BoardPosition[2][2]));
  Serial.println(" }");
}

void UpdateCurrentXY()
{
  switch (PositionSelected)
  {
    case 0: CurrentX = 0; CurrentY = 0; DigitalPosition = 6; break;
    case 1: CurrentX = 1; CurrentY = 0; DigitalPosition = 7; break;
    case 2: CurrentX = 2; CurrentY = 0; DigitalPosition = 8; break;
    case 3: CurrentX = 0; CurrentY = 1; DigitalPosition = 5; break;
    case 4: CurrentX = 1; CurrentY = 1; DigitalPosition = 4; break;
    case 5: CurrentX = 2; CurrentY = 1; DigitalPosition = 3; break;
    case 6: CurrentX = 0; CurrentY = 2; DigitalPosition = 0; break;
    case 7: CurrentX = 1; CurrentY = 2; DigitalPosition = 1; break;
    case 8: CurrentX = 2; CurrentY = 2; DigitalPosition = 2; break;
  }
}

int GetDigitalPosition(int pos)
{
  switch (PositionSelected)
  {
    case 0: return 6; break;
    case 1: return 7; break;
    case 2: return 8; break;
    case 3: return 5; break;
    case 4: return 4; break;
    case 5: return 3; break;
    case 6: return 0; break;
    case 7: return 1; break;
    case 8: return 2; break;
  }
}

void SetupGame(void)
{
  fillColor(CRGB::Black, true);
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      BoardPosition[i][j] = 0;
    }
  }

  FirstPlayerTurn = true;
  
  PositionSelected = 0;
  UpdateCurrentXY();
}

void IterateCurrentPosition(void)
{
  ++PositionSelected;
  if (PositionSelected >= 9) PositionSelected = 0;
  UpdateCurrentXY();

  Serial.print("Iterating position to digital position ");
  Serial.println(DigitalPosition);
}

bool CheckForBoardFull()
{
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      if (BoardPosition[i][j] == 0) return false;
    }
  }

  delay(500);
  fillColor(CRGB::White, true);
  delay(500);
  fillColor(CRGB::Black, true);
  delay(500);
  fillColor(CRGB::White, true);
  delay(500);
  fillColor(CRGB::Black, true);
  delay(500);
  fillColor(CRGB::White, true);
  delay(500);
  fillColor(CRGB::Black, true);
  delay(500);

  return true;
}

bool CheckForWinner()
{
  int winner = 0;
  
  for (int i = 1; i < 3; ++i)
  {
    if ((BoardPosition[0][0] == i) && (BoardPosition[1][0] == i) && (BoardPosition[2][0] == i)) { winner = i; break; }
    if ((BoardPosition[0][1] == i) && (BoardPosition[1][1] == i) && (BoardPosition[2][1] == i)) { winner = i; break; }
    if ((BoardPosition[0][2] == i) && (BoardPosition[1][2] == i) && (BoardPosition[2][2] == i)) { winner = i; break; }
    if ((BoardPosition[0][0] == i) && (BoardPosition[0][1] == i) && (BoardPosition[0][2] == i)) { winner = i; break; }
    if ((BoardPosition[1][0] == i) && (BoardPosition[1][1] == i) && (BoardPosition[1][2] == i)) { winner = i; break; }
    if ((BoardPosition[2][0] == i) && (BoardPosition[2][1] == i) && (BoardPosition[2][2] == i)) { winner = i; break; }
    if ((BoardPosition[0][0] == i) && (BoardPosition[1][1] == i) && (BoardPosition[2][2] == i)) { winner = i; break; }
    if ((BoardPosition[2][0] == i) && (BoardPosition[1][1] == i) && (BoardPosition[0][2] == i)) { winner = i; break; }
  }

  if (winner != 0)
  {
    fillColor(PlayerColors[(winner == 1) ? Player1Color : Player2Color], true);
    delay(500);
    fillColor(CRGB::Black, true);
    delay(500);
    fillColor(PlayerColors[(winner == 1) ? Player1Color : Player2Color], true);
    delay(500);
    fillColor(CRGB::Black, true);
    delay(500);
    fillColor(PlayerColors[(winner == 1) ? Player1Color : Player2Color], true);
    delay(500);
    fillColor(CRGB::Black, true);
    delay(500);
    
    return true;
  }

  return false;
}

void PlaceSymbol(void)
{
  if (BoardPosition[CurrentX][CurrentY] != 0)
  {
    fillColor(CRGB::White, true);
    delay(500);
    return;
  }

  //  Set the current position selected to the current player's color
  Serial.print("Placing Symbol ");
  Serial.print(FirstPlayerTurn ? 1 : 2);
  Serial.print(" on position ");
  Serial.print(CurrentX);
  Serial.print(",");
  Serial.println(CurrentY);
  BoardPosition[CurrentX][CurrentY] = (FirstPlayerTurn ? 1 : 2);

  if (CheckForWinner() == true)
  {
    SetupGame();
    return;
  }

  if (CheckForBoardFull() == true)
  {
    SetupGame();
    return;
  }
  
  DebugOutputBoard();
  FirstPlayerTurn = !FirstPlayerTurn;
}

void ChangeColor(void)
{
  int& playerColor = (FirstPlayerTurn ? Player1Color : Player2Color);

  do
  {
    ++playerColor;
    if (playerColor >= PlayerColorCount) playerColor = 0;
  } while (playerColor == (FirstPlayerTurn ? Player2Color : Player1Color));
  
  Serial.print("Changed player color to ");
  Serial.println(playerColor);
}

CRGB GetPositionColor(int row, int col)
{
  if (BoardPosition[row][col] == 0) return CRGB::Black;
  
  return PlayerColors[((BoardPosition[row][col] == 1) ? Player1Color : Player2Color)];
}

bool ButtonCheck(int pin)
{
  static long buttonTimer = 0;
  long currentMillis = millis();
  if (buttonTimer < currentMillis)
  {
    if (digitalRead(pin) == LOW) //  Read the pattern switch button
    {
      Serial.println("Button Pressed!");
      buttonTimer = currentMillis + BUTTON_DELAY;
      return true;
    }
  }
  
  return false;
}

void RenderBoard(void)
{
  //  Set the Color Indicator LED based on the current player's current color
  CRGB playerColor = PlayerColors[(FirstPlayerTurn ? Player1Color : Player2Color)];
  analogWrite(COLOR_INDICATOR_R_PIN, 255 - playerColor.r);
  analogWrite(COLOR_INDICATOR_G_PIN, 255 - playerColor.g);
  analogWrite(COLOR_INDICATOR_B_PIN, 255 - playerColor.b);
  
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      leds[DigitalBoardPosition[i][j]] = GetPositionColor(i, j);

  if ((millis() % 1000) >= 900) leds[DigitalPosition] = CRGB::Blue;
  
  FastLED.show(); // display this frame
}

void setup(){
  
  Serial.begin(9600);
  
  //  Setup the LED strip and color all LEDs black
  FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  fillColor(CRGB::Black, true);
  
  //  Set the button pins to INPUT_PULLUP and reset the button timer
  pinMode(SWITCH_POS_BUTTON_PIN, INPUT_PULLUP); // connect internal pull-up
  pinMode(PLACE_SYMBOL_BUTTON_PIN, INPUT_PULLUP); // connect internal pull-up
  pinMode(SWITCH_COLOR_BUTTON_PIN, INPUT_PULLUP); // connect internal pull-up

  pinMode(COLOR_INDICATOR_PIN, OUTPUT);
  digitalWrite(COLOR_INDICATOR_PIN, HIGH);
  
  pinMode(COLOR_INDICATOR_R_PIN, OUTPUT);
  pinMode(COLOR_INDICATOR_G_PIN, OUTPUT);
  pinMode(COLOR_INDICATOR_B_PIN, OUTPUT);

  DigitalBoardPosition[0][0] = 6;
  DigitalBoardPosition[1][0] = 7;
  DigitalBoardPosition[2][0] = 8;
  DigitalBoardPosition[0][1] = 5;
  DigitalBoardPosition[1][1] = 4;
  DigitalBoardPosition[2][1] = 3;
  DigitalBoardPosition[0][2] = 0;
  DigitalBoardPosition[1][2] = 1;
  DigitalBoardPosition[2][2] = 2;

  Player1Color = 1; // Aqua
  Player2Color = 7; // Purple

  SetupGame();

  Serial.println("Start Program: Tic Tac Toe");
}

void loop()
{
  if (ButtonCheck(SWITCH_POS_BUTTON_PIN)) IterateCurrentPosition();
  if (ButtonCheck(PLACE_SYMBOL_BUTTON_PIN)) PlaceSymbol();
  if (ButtonCheck(SWITCH_COLOR_BUTTON_PIN)) ChangeColor();
  
  RenderBoard();
}

