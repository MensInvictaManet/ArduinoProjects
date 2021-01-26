#include <FastLED.h>
#include <Wire.h>

////////////////////////////////////////
///////////// GAME DATA ////////////////
////////////////////////////////////////

#define DEBUG_OUTPUT      0
#define DEBUG(output)     if (DEBUG_OUTPUT) Serial.println(output);

#define BRIDGE_LEVEL      10
#define STEP_DELAY        132
#define RUN_AWAY_TIMER    40
#define CHASE_TIMER       300
#define REVIVE_COUNTER    60
#define ATTRACT_TIMER     20000

#define MAP_W             24
#define MAP_H             23
#define XXX               0

#define COLOR_PACMAN	    0xFFFF00
#define COLOR_BLINKY      0xFF0000
#define COLOR_PINKY       0xFF00FF
#define COLOR_INKY        0x00DFDF
#define COLOR_CLYDE       0xE06000
#define COLOR_VULNERABLE  0x00008B
#define COLOR_DEADGHOST   0xA9A9A9

static const long int GhostColors[4] = { COLOR_BLINKY, COLOR_PINKY, COLOR_INKY, COLOR_CLYDE };
static unsigned long int attractTimeout = 0;

enum Direction
{
    DIRECTION_NONE = -1,
    DIRECTION_U,
    DIRECTION_D,
    DIRECTION_L,
    DIRECTION_R,
    DIRECTION_UL,
    DIRECTION_UR,
    DIRECTION_DL,
    DIRECTION_DR
};

const int DirectionSpeeds[4][2] = 
{
	{ 0, -1 },	// UP
	{ 1, 0 },	// RIGHT
	{ 0, 1 },	// DOWN
	{ -1, 0 }	// LEFT
};

enum GhostBehavior
{
	BEHAVIOR_CORNER_TL,
	BEHAVIOR_CORNER_TR,
	BEHAVIOR_CORNER_BL,
	BEHAVIOR_CORNER_BR,
	BEHAVIOR_CHASE,
	BEHAVIOR_FRONT,
	BEHAVIOR_RANDOM,
	BEHAVIOR_RUN_AWAY,
	BEHAVIOR_EATEN
};

struct Character
{
	Character(unsigned char x, unsigned char y, signed char dirX, signed char dirY) :
		originX(x),
		originY(y),
		X(x),
		Y(y),
		SpeedX(dirX),
		SpeedY(dirY),
		Behavior(BEHAVIOR_RANDOM),
		BehaviorCounter(REVIVE_COUNTER),
		Active(false)
	{}
	
	void Reset(bool active = false)
	{
		X = originX;
		Y = originY;
		Active = active;
		SpeedX = 1;
		SpeedY = 0;
		Behavior = BEHAVIOR_RANDOM;
		BehaviorCounter = REVIVE_COUNTER;
	}
	
	void TurnTo(Direction dir)
	{
		switch (dir)
		{
			case DIRECTION_U:		SpeedX = 0; 	SpeedY = -1; 	return;
			case DIRECTION_D:		SpeedX = 0; 	SpeedY = 1; 	return;
			case DIRECTION_L:		SpeedX = -1; 	SpeedY = 0; 	return;
			case DIRECTION_R:		SpeedX = 1; 	SpeedY = 0; 	return;
		}
	}

	signed char originX;
	signed char originY;
	signed char X;
	signed char Y;
	
	signed char SpeedX;
	signed char SpeedY;
	
	GhostBehavior Behavior;
	unsigned long BehaviorCounter;
	bool Active;
};

Character Pacman(12, 17, 0, 0);
Character Ghosts[4] = { Character(11, 8, 1, 0), Character(11, 8, 1, 0), Character(11, 8, 1, 0), Character(11, 8, 1, 0) };

const PROGMEM byte LightMap[MAP_W * MAP_H] =
{
	 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42, XXX, XXX, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
	 31, XXX, XXX, XXX,  19, XXX, XXX, XXX, XXX, XXX,  43, XXX, XXX, 109, XXX, XXX, XXX, XXX, XXX, 100, XXX, XXX, XXX, 121,
	 30, XXX, XXX, XXX,  18, XXX, XXX, XXX, XXX, XXX,  44, XXX, XXX, 108, XXX, XXX, XXX, XXX, XXX, 101, XXX, XXX, XXX, 122,
	 29,  28,  14,  15,  20,  16,  17,  50,  49,  48,  47,  46,  45, 107, 106, 105, 104, 103,  95, 102,  99,  98, 123, 124,
	 27, XXX, XXX, XXX,  21, XXX, XXX,  51, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,  94, XXX, XXX,  97, XXX, XXX, XXX, 125,
	 26, XXX, XXX, XXX,  13, XXX, XXX,  52, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,  93, XXX, XXX,  96, XXX, XXX, XXX, 126,
	 25,  24,  23,  22,  12, XXX, XXX,  53,  54,  55,  56, XXX, XXX,  89,  90,  91,  92, XXX, XXX, 131, 130, 129, 128, 127,
	XXX, XXX, XXX, XXX,  11, XXX, XXX, XXX, XXX, XXX,  57, XXX, XXX,  88, XXX, XXX, XXX, XXX, XXX, 132, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX,  10, XXX, XXX,  63,  62,  61,  60,  59,  58,  87,  86,  85,  84, XXX, XXX, 133, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX,   9, XXX, XXX,  64, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,  83, XXX, XXX, 134, XXX, XXX, XXX, XXX,
	  0,   1,   2,   3,   8,   7,   6,  65, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,  82,  81,  80, 139, 138, 135, 137, 136,
	XXX, XXX, XXX, XXX,   5, XXX, XXX,  66, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,  79, XXX, XXX, 140, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX,   4, XXX, XXX,  67,  69,  70,  71,  72,  73,  74,  75,  76,  78, XXX, XXX, 141, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX, 212, XXX, XXX,  68, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,  77, XXX, XXX, 142, XXX, XXX, XXX, XXX,
	216, 215, 214, 213, 211, 208, 207, 206, 205, 204, 203, XXX, XXX, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182,
	217, XXX, XXX, XXX, 210, XXX, XXX, XXX, XXX, XXX, 202, XXX, XXX, 193, XXX, XXX, XXX, XXX, XXX, 143, XXX, XXX, XXX, 181,
	218, XXX, XXX, XXX, 209, XXX, XXX, XXX, XXX, XXX, 201, XXX, XXX, 194, XXX, XXX, XXX, XXX, XXX, 144, XXX, XXX, XXX, 180,
	219, 220, XXX, XXX, 229, 230, 231, 232, 233, 200, 199, 198, 197, 196, 195, 168, 169, 170, 171, 172, XXX, XXX, 178, 179,
	XXX, 221, XXX, XXX, 228, XXX, XXX, 234, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 167, XXX, XXX, 145, XXX, XXX, 177, XXX,
	XXX, 222, XXX, XXX, 227, XXX, XXX, 235, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 166, XXX, XXX, 173, XXX, XXX, 176, XXX,
	255, 223, 224, 225, 226, XXX, XXX, 236, 237, 238, 239, XXX, XXX, 162, 163, 164, 165, XXX, XXX, 146, 174, 147, 175, 148,
	254, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 240, XXX, XXX, 161, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 149,
	253, 252, 251, 250, 249, 238, 247, 246, 245, 244, 243, 242, 241, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150
};

//  Dot Types: 0 (No Dot), 1 (Basic Dot), 2 (Power Pill)
const PROGMEM byte PillColor[5] = { 60, 255, 128, 0, 128 };

#define DEFAULT_DOT_COUNT 100
const PROGMEM uint16_t DotPositions[DEFAULT_DOT_COUNT] =
{
	0, 2, 4, 6, 8, 10, 13, 15, 17, 19, 21, 23,
	
	
	74, 76, 78, 80, 82, 85, 87, 89, 91, 93,
	
	120, 124, 127, 136, 139, 143,
	145, 147, 152, 154, 157, 159, 164, 166,
	172, 187, 
	
	220, 235,
	
	268, 283,
	
	316, 331,
	337, 339, 341, 343, 345, 350, 352, 354, 356, 358,
	360, 364, 370, 373, 379, 383,
	
	412, 414, 416, 418, 421, 423, 425, 427,
	433, 439, 448, 454,
	460, 475,
	481, 483, 487, 489, 494, 496, 500, 502,
	504, 514, 517, 527,
	529, 531, 533, 535, 537, 539, 540, 542, 544, 546, 548, 550
};

#define DEFAULT_PILL_COUNT 4
const PROGMEM uint16_t PillPositions[DEFAULT_PILL_COUNT] = { 48, 71, 408, 431 };

unsigned char GetDotIndexByPosition(int space)
{
	for (int i = 0; i < DEFAULT_DOT_COUNT; ++i)
		if (pgm_read_word_near(DotPositions + i) == space) return i;
		
	for (int i = 0; i < DEFAULT_PILL_COUNT; ++i)
		if (pgm_read_word_near(PillPositions + i) == space) return DEFAULT_DOT_COUNT + i;
		
	return 255;
}

byte DotsRemainingByteFlag[13]; //  98 bits representing the Dots remaining. 4 bits representing the Power Pills. 2 bits unused. 

inline bool IsDotStillAtIndex(int dotIndex)                   { return (DotsRemainingByteFlag[dotIndex / 8] & (1 << (dotIndex % 8))); }
inline void ClearDotAtIndex(int dotIndex)                     { DotsRemainingByteFlag[dotIndex / 8] ^= (1 << (dotIndex % 8)); }
inline void ResetDotIndexMap()                                { for (int i = 0; i < 13; ++i) DotsRemainingByteFlag[i] = 255; }

inline bool IsMapPosition(unsigned char x, unsigned char y) 	{ return ((x == 0 && y == 10) || (pgm_read_byte_near(LightMap + (MAP_W * y) + x) != 0)); }
bool CanMove(const Character& c, int x, int y)					      { return (((c.Y == BRIDGE_LEVEL) && ((c.X + x == -1) || c.X + x == MAP_W)) || ((c.X + x >= 0) && (c.X + x < MAP_W) && (c.Y + y >= 0) && (c.Y + y < MAP_H) && (IsMapPosition(c.X + x, c.Y + y)))); }
inline bool CanContinue(const Character& c)						        { return CanMove(c, c.SpeedX, c.SpeedY); }
inline bool CanTurn(const Character& c)							          { return (CanMove(c, c.SpeedY, c.SpeedX) || CanMove(c, -c.SpeedY, -c.SpeedX)); }
inline void Move(Character& c)									              { c.X += c.SpeedX; c.Y += c.SpeedY; }
inline bool IsAtOrigin(Character& c)							            { return (c.X == c.originX && c.Y == c.originY); }
inline bool PacmanOnSpawn()										                { return ((Pacman.Y == 8) && (Pacman.X >= 7) && (Pacman.X <= 16)) || (Pacman.X == 10 && (Pacman.Y == 7 || Pacman.Y == 6)); }

inline void Left(Character& c, int& speedX, int& speedY)
{
	int directionIndex = -1;
	for (int i = 0; i < 4; ++i)
		if (c.SpeedX == DirectionSpeeds[i][0] && c.SpeedY == DirectionSpeeds[i][1])
			directionIndex = (((i - 1) == -1) ? 3 : (i - 1));
	
	speedX = DirectionSpeeds[directionIndex][0];
	speedY = DirectionSpeeds[directionIndex][1];
}

inline void Right(Character& c, int& speedX, int& speedY)
{
	int directionIndex = -1;
	for (int i = 0; i < 4; ++i)
		if (c.SpeedX == DirectionSpeeds[i][0] && c.SpeedY == DirectionSpeeds[i][1])
			directionIndex = (((i + 1) == 4) ? 0 : (i + 1));
			
	speedX = DirectionSpeeds[directionIndex][0];
	speedY = DirectionSpeeds[directionIndex][1];
}

inline void PacmanDelta(Character& c, int& deltaX, int& deltaY, bool reverse = false, bool normalized = false)
{
	deltaX = Pacman.X - c.X;
	deltaY = Pacman.Y - c.Y;
	if (reverse) { deltaX *= -1; deltaY *= -1; }
	if (normalized) { deltaX = min(max(deltaX, -1), 1); deltaY = min(max(deltaY, -1), 1); }
}

inline void OriginDelta(Character& c, int& deltaX, int& deltaY, bool reverse = false, bool normalized = false)
{
	deltaX = c.originX - c.X;
	deltaY = c.originY - c.Y;
	if (reverse) { deltaX *= -1; deltaY *= -1; }
	if (normalized) { deltaX = min(max(deltaX, -1), 1); deltaY = min(max(deltaY, -1), 1); }
}

void TurnRandom(Character& c)
{
	c.SpeedX ^= c.SpeedY;
	c.SpeedY ^= c.SpeedX;
	c.SpeedX ^= c.SpeedY;
	
	if (c.SpeedX == 0)	c.SpeedY = random(2) ? 1 : -1;
	else				c.SpeedX = random(2) ? 1 : -1;
	
	if (CanContinue(c)) return;
	
	if (c.SpeedX == 0)	c.SpeedY *= -1;
	else				c.SpeedX *= -1;
}

bool GetEatenDirection(Character& c)
{
	if (c.X == 0  && c.Y == 0 )	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 4  && c.Y == 0 )	{ c.SpeedX = 0;		c.SpeedY = 1;	return true; }
	if (c.X == 10 && c.Y == 0 )	{ c.SpeedX = 0;		c.SpeedY = 1;	return true; }
	if (c.X == 13 && c.Y == 0 )	{ c.SpeedX = 0;		c.SpeedY = 1;	return true; }
	if (c.X == 19 && c.Y == 0 )	{ c.SpeedX = 0;		c.SpeedY = 1;	return true; }
	if (c.X == 23 && c.Y == 0 )	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	
	if (c.X == 0  && c.Y == 3 )	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 4  && c.Y == 3 )	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 7 && c.Y == 3 )	{ c.SpeedX = 0;		c.SpeedY = 1;	return true; }
	if (c.X == 10 && c.Y == 3 )	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 13 && c.Y == 3 )	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 16 && c.Y == 3 )	{ c.SpeedX = 0;		c.SpeedY = 1;	return true; }
	if (c.X == 19 && c.Y == 3 )	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 23 && c.Y == 3 )	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	
	if (c.X == 0  && c.Y == 6 )	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 4  && c.Y == 6 )	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 7  && c.Y == 6 )	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 10 && c.Y == 6 )	{ c.SpeedX = 0;		c.SpeedY = 1;	return true; }
	if (c.X == 13 && c.Y == 6 )	{ c.SpeedX = 0;		c.SpeedY = 1;	return true; }
	if (c.X == 16 && c.Y == 6 )	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 19 && c.Y == 6 )	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 23 && c.Y == 6 )	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	
	if (c.X == 7  && c.Y == 8 )	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 10 && c.Y == 8 )	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 13 && c.Y == 8 )	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 16 && c.Y == 8 )	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	
	if (c.X == 4  && c.Y == 10)	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 7  && c.Y == 10)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 16 && c.Y == 10)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 19 && c.Y == 10)	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	
	if (c.X == 7  && c.Y == 12)	{ c.SpeedX = 0;		c.SpeedY =-1;	return true; }
	if (c.X == 16 && c.Y == 12)	{ c.SpeedX = 0;		c.SpeedY =-1;	return true; }
	
	if (c.X == 0  && c.Y == 14)	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 4  && c.Y == 14)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 7  && c.Y == 14)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 10 && c.Y == 14)	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 13 && c.Y == 14)	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 16 && c.Y == 14)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 19 && c.Y == 14)	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 23 && c.Y == 14)	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	
	if (c.X == 0  && c.Y == 17)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 1  && c.Y == 17)	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 4  && c.Y == 17)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 7  && c.Y == 17)	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 10 && c.Y == 17)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 13 && c.Y == 17)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 16 && c.Y == 17)	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 19 && c.Y == 17)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 22 && c.Y == 17)	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 23 && c.Y == 17)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	
	if (c.X == 0  && c.Y == 20)	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 1  && c.Y == 20)	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 4  && c.Y == 20)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 7  && c.Y == 20)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 10 && c.Y == 20)	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 13 && c.Y == 20)	{ c.SpeedX = 1;		c.SpeedY = 0;	return true; }
	if (c.X == 16 && c.Y == 20)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 19 && c.Y == 20)	{ c.SpeedX = 0;		c.SpeedY = -1;	return true; }
	if (c.X == 22 && c.Y == 20)	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	if (c.X == 23 && c.Y == 20)	{ c.SpeedX = -1;	c.SpeedY = 0;	return true; }
	
	return false;
}

int DotsCollected = 0;
int Score = 0;
int Level = 0;
int DotCount = 0;


////////////////////////////////////////
///////////// GAME CODE ////////////////
////////////////////////////////////////

//  NOTE: Add "UP" symbol so people don't think the button is the top.
//  NOTE: Ghost runaway timer... too fast? How does it compare to the original?
//  NOTE: Possibly add an indicator of the ghost status in the box? Eaten or unreleased?
//  NOTE: As soon as the power pill is eaten, they should determine direction (even if they are already edible)
//  NOTE: Pause and display the cause of death before the flashes, and when you win
//  NOTE: Death sequence... fade from pacman to the color of the ghost that ate him? No flash?

#define LED_STRIP_PIN     10	//  The NeoPixel string data pin
#define Button_F          6   //  The pin that controls the fire button signal
#define Button_U          2   //  The pin that controls the UP button signal
#define Button_D          3   //  The pin that controls the DOWN button signal
#define Button_L          4   //  The pin that controls the LEFT button signal
#define Button_R          5   //  The pin that controls the RIGHT button signal
#define Joystick_Ground   7   //  The grounding wire for the controller

#define NUM_LEDS		  256	//  The number of LEDs we want to access
#define BRIGHTNESS  	200	//  The number (0 to 200) for the brightness setting)

CRGB leds[NUM_LEDS];

unsigned long turnMillis = 0;
Direction controllerDirection;
bool ghostRunAwayDelay = false;

char directionTimers[4];
char ghostBlink = 0;
char powerPillBlink = 0;
bool attractMode = true;

//LedControl sevenSeg = LedControl(7,8,9,1);

inline void FillColor(long int color) { memset(leds, color, sizeof(struct CRGB) * NUM_LEDS); }

void setScoreDisplay(long score, int level)
{
  if (score < 0)      return;
  if (score > 99999)  return;

  Serial.println("Score: " + String(score));

 score = min(9999, score);
  //  Send the current score to the scoreboard controller over i2c
  Wire.beginTransmission(0x60);
  Wire.write((const char*)&score, 4);                
  Wire.endTransmission();   
}

void StepForwardAllTheWay()
{
	while (millis() >= turnMillis) StepForward();
}

void StepForward()
{
	turnMillis += (STEP_DELAY - min(max(Level, 0) * 30, STEP_DELAY - 1));
}

void AttractModeTransition()
{
	//  Set all LEDs to white, then render
	FillColor(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	FastLED.show();
	delay(250);
	FillColor(0x151515);
	FastLED.show();
	delay(250);
	StepForwardAllTheWay();
	SetControllerDirection(DIRECTION_NONE);
}

void WinTransition()
{
	//  Set all LEDs to white, then render
	DrawMap();
	FastLED.show();
	delay(750);
	FillColor(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	delay(250);
	FillColor(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	delay(250);
	FillColor(0x151515);
	FastLED.show();
	delay(500);
	StepForwardAllTheWay();
	SetControllerDirection(DIRECTION_NONE);
}

void DeathTransition()
{
	//  Set all LEDs to white, then render
	DrawMap();
	FastLED.show();
	delay(750);
	FillColor(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	delay(250);
	FillColor(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	delay(250);
	FillColor(0x151515);
	FastLED.show();
	delay(500);
	StepForwardAllTheWay();
	SetControllerDirection(DIRECTION_NONE);
}

void EatTransition()
{
	//  Set all LEDs to white, then render
	FillColor(0x151515);
	FastLED.show();
	delay(250);
	StepForwardAllTheWay();
}

void CheckForDeath()
{
	for (int i = 0; i < 4; ++i)
	{
		if (Ghosts[i].Behavior == BEHAVIOR_EATEN) continue;
		
		if (Ghosts[i].Active && Ghosts[i].X == Pacman.X && Ghosts[i].Y == Pacman.Y)
		{
			if (Ghosts[i].Behavior == BEHAVIOR_RUN_AWAY)
			{
				//  Pacman ate a ghost
				Ghosts[i].Behavior = BEHAVIOR_EATEN;
				Ghosts[i].BehaviorCounter = 0;
				EatTransition();
				Score += 5;
				setScoreDisplay(Score, Level);
			}
			else
			{
				//  Pacman died. Reset the game and the level
				DeathTransition();
				ResetMap();
				setScoreDisplay(Score, Level);

        //  If the player hasn't touched the controller within the attract timeout seconds and just died, return to attract mode
        if (!attractMode && (millis() > attractTimeout)) attractMode = !attractMode;
			}
		}
	}
}

void ResetMap()
{
    //  Reset the dots on the map
    DotsCollected = 0;
    Score = 0;
    setScoreDisplay(Score, Level);
    Level = 0;
    ResetDotIndexMap();  //  Set all dots and power pills to be active according to the bit flag
    
    Pacman.X = 12;
    Pacman.Y = 17;
    Pacman.SpeedX = 0;
    Pacman.SpeedY = 0;

    //  Reset all ghosts, and set only the first to active
    for (int i = 0; i < 4; ++i) Ghosts[i].Reset(false);
    
    SetControllerDirection(DIRECTION_NONE);
}

void DetermineGhostsReleased()
{
	if (PacmanOnSpawn())
	{
		DEBUG("Can't spawn while Pacman is in the spawn area.");
		return;
	}
	
	//  Blinky
	if (Ghosts[0].Active == false && (attractMode || (DotsCollected >= 0)))
	{
		Ghosts[0].Reset(true);
		Ghosts[0].SpeedX = (random(2) ? 1 : -1);
		Ghosts[0].Behavior = attractMode ? BEHAVIOR_RANDOM : BEHAVIOR_CHASE;
	}
	
	//  Pinky
	if (Ghosts[1].Active == false && (attractMode || (DotsCollected >= (DEFAULT_DOT_COUNT / 4))))
	{
		Ghosts[1].Reset(true);
		Ghosts[1].SpeedX = (random(2) ? 1 : -1);
		Ghosts[1].Behavior = BEHAVIOR_RANDOM;
	}
	
	//  Inky
	if (Ghosts[2].Active == false && (attractMode || (DotsCollected >= (DEFAULT_DOT_COUNT / 2))))
	{
		Ghosts[2].Reset(true);
		Ghosts[2].SpeedX = (random(2) ? 1 : -1);
		Ghosts[2].Behavior = BEHAVIOR_RANDOM;
	}
	
	// Clyde
	if (Ghosts[3].Active == false && (attractMode || (DotsCollected >= (DEFAULT_DOT_COUNT - (DEFAULT_DOT_COUNT / 4)))))
	{
		Ghosts[3].Reset(true);
		Ghosts[3].SpeedX = (random(2) ? 1 : -1);
		Ghosts[3].Behavior = BEHAVIOR_RANDOM;
	}
}

void SetControllerDirection(Direction dir)
{
	controllerDirection = dir;
	for (char i = 0; i < 4; ++i) directionTimers[i] = 0;
	if (controllerDirection == DIRECTION_NONE) { return; }
	
	
	for (char i = 0; i < 4; ++i) directionTimers[i] = 0;
	switch (dir)
	{
		case DIRECTION_U:
			directionTimers[DIRECTION_U] = 4;
			break;
		case DIRECTION_D:
			directionTimers[DIRECTION_D] = 4;
			break;
		case DIRECTION_L:
			directionTimers[DIRECTION_L] = 4;
			break;
		case DIRECTION_R:
			directionTimers[DIRECTION_R] = 4;
			break;
		case DIRECTION_UL:
			directionTimers[DIRECTION_U] = 4;
			directionTimers[DIRECTION_L] = 4;
			break;
		case DIRECTION_UR:
			directionTimers[DIRECTION_U] = 4;
			directionTimers[DIRECTION_R] = 4;
			break;
		case DIRECTION_DL:
			directionTimers[DIRECTION_D] = 4;
			directionTimers[DIRECTION_L] = 4;
			break;
		case DIRECTION_DR:
			directionTimers[DIRECTION_D] = 4;
			directionTimers[DIRECTION_R] = 4;
			break;
	}
}

void DetermineControllerDirection()
{
	for (char i = 0; i < 4; ++i) if (directionTimers[i] > 0) --directionTimers[i];
	
	controllerDirection = DIRECTION_NONE;

	bool dirU = (digitalRead(Button_U) == 0);
	bool dirD = (digitalRead(Button_D) == 0);
	bool dirL = (digitalRead(Button_L) == 0);
	bool dirR = (digitalRead(Button_R) == 0);

  //  If any button is pressed, reset the attractTimeout so that we don't move back into Attract Mode
  if (dirU || dirD || dirL || dirR) attractTimeout = millis() + ATTRACT_TIMER;

	if 		(dirU && !dirD && !dirL && !dirR)	  SetControllerDirection(DIRECTION_U);
	else if (!dirU && dirD && !dirL && !dirR)	SetControllerDirection(DIRECTION_D);
	else if (!dirU && !dirD && dirL && !dirR)	SetControllerDirection(DIRECTION_L);
	else if (!dirU && !dirD && !dirL && dirR)	SetControllerDirection(DIRECTION_R);
	else if (dirU && !dirD && dirL && !dirR)	SetControllerDirection(DIRECTION_UL);
	else if (dirU && !dirD && !dirL && dirR)	SetControllerDirection(DIRECTION_UR);
	else if (!dirU && dirD && dirL && !dirR)	SetControllerDirection(DIRECTION_DL);
	else if (!dirU && dirD && !dirL && dirR)	SetControllerDirection(DIRECTION_DR);
}

bool CheckDirection(Direction dir)
{
	return (controllerDirection == dir || directionTimers[dir] > 0);
}

void DeterminePacmanDirection()
{
	if 		(CheckDirection(DIRECTION_U) && CanMove(Pacman, 0, -1))	Pacman.TurnTo(DIRECTION_U);
	else if (CheckDirection(DIRECTION_D) && CanMove(Pacman, 0, 1))	Pacman.TurnTo(DIRECTION_D);
	else if (CheckDirection(DIRECTION_L) && CanMove(Pacman, -1, 0))	Pacman.TurnTo(DIRECTION_L);
	else if (CheckDirection(DIRECTION_R) && CanMove(Pacman, 1, 0))	Pacman.TurnTo(DIRECTION_R);
	else if (controllerDirection == DIRECTION_UL)
	{
		if (CanMove(Pacman, 0, -1))	Pacman.TurnTo(DIRECTION_U);
		if (CanMove(Pacman, -1, 0))	Pacman.TurnTo(DIRECTION_L);
	}
	else if (controllerDirection == DIRECTION_UR)
	{
		if (CanMove(Pacman, 0, -1))	Pacman.TurnTo(DIRECTION_U);
		if (CanMove(Pacman, 1, 0))	Pacman.TurnTo(DIRECTION_R);
	}
	else if (controllerDirection == DIRECTION_DL)
	{
		if (CanMove(Pacman, 0, 1))	Pacman.TurnTo(DIRECTION_D);
		if (CanMove(Pacman, -1, 0))	Pacman.TurnTo(DIRECTION_L);
	}
	else if (controllerDirection == DIRECTION_DR)
	{
		if (CanMove(Pacman, 0, 1))	Pacman.TurnTo(DIRECTION_D);
		if (CanMove(Pacman, 1, 0))	Pacman.TurnTo(DIRECTION_R);
	}
}

void DetermineGhostDirections()
{
	for (int i = 0; i < 4; ++i)
	{
		if (Ghosts[i].Active == false) continue;
		
		switch (Ghosts[i].Behavior)
		{
		case BEHAVIOR_CHASE:
			{
				if (++Ghosts[i].BehaviorCounter >= CHASE_TIMER) Ghosts[i].Behavior = BEHAVIOR_RANDOM;
				
				//  Get the direction to run. If we're already heading in that direction, break out
				int dirX, dirY, leftSpeedX, leftSpeedY, rightSpeedX, rightSpeedY, currentDanger, leftDanger, rightDanger;
				PacmanDelta(Ghosts[i], dirX, dirY, true, true);
				Left(Ghosts[i], leftSpeedX, leftSpeedY);
				Right(Ghosts[i], rightSpeedX, rightSpeedY);
				currentDanger = CanContinue(Ghosts[i]) ? (Ghosts[i].SpeedX != 0 && ((dirX > 0 && Ghosts[i].SpeedX > 0) || (dirX < 0 && Ghosts[i].SpeedX < 0))) + (Ghosts[i].SpeedY != 0 && ((dirY > 0 && Ghosts[i].SpeedY > 0) || (dirY < 0 && Ghosts[i].SpeedY < 0))) : 3;
				leftDanger = CanMove(Ghosts[i], leftSpeedX, leftSpeedY) ? ((leftSpeedX != 0 && ((dirX > 0 && leftSpeedX > 0) || (dirX < 0 && leftSpeedX < 0))) + (leftSpeedY != 0 && ((dirY > 0 && leftSpeedY > 0) || (dirY < 0 && leftSpeedY < 0)))) : 3;
				rightDanger = CanMove(Ghosts[i], rightSpeedX, rightSpeedY) ? ((rightSpeedX != 0 && ((dirX > 0 && rightSpeedX > 0) || (dirX < 0 && rightSpeedX < 0))) + (rightSpeedY != 0 && ((dirY > 0 && rightSpeedY > 0) || (dirY < 0 && rightSpeedY < 0)))) : 3;
				
				if (currentDanger <= leftDanger && currentDanger <= rightDanger) { break; }
				else if (leftDanger < rightDanger)
				{
					Ghosts[i].SpeedX = leftSpeedX;
					Ghosts[i].SpeedY = leftSpeedY;
				}
				else if (rightDanger < leftDanger)
				{
					Ghosts[i].SpeedX = rightSpeedX;
					Ghosts[i].SpeedY = rightSpeedY;
				}
				else
				{
					TurnRandom(Ghosts[i]);
				}
			}
			break;
			
			
		case BEHAVIOR_RUN_AWAY:
			{
				if (++Ghosts[i].BehaviorCounter >= RUN_AWAY_TIMER) Ghosts[i].Behavior = BEHAVIOR_RANDOM;
				
				//  Get the direction to run. If we're already heading in that direction, break out
				int dirX, dirY, leftSpeedX, leftSpeedY, rightSpeedX, rightSpeedY, currentDanger, leftDanger, rightDanger;
				PacmanDelta(Ghosts[i], dirX, dirY, false, true);
				Left(Ghosts[i], leftSpeedX, leftSpeedY);
				Right(Ghosts[i], rightSpeedX, rightSpeedY);
				currentDanger = CanContinue(Ghosts[i]) ? (Ghosts[i].SpeedX != 0 && ((dirX > 0 && Ghosts[i].SpeedX > 0) || (dirX < 0 && Ghosts[i].SpeedX < 0))) + (Ghosts[i].SpeedY != 0 && ((dirY > 0 && Ghosts[i].SpeedY > 0) || (dirY < 0 && Ghosts[i].SpeedY < 0))) : 3;
				leftDanger = CanMove(Ghosts[i], leftSpeedX, leftSpeedY) ? ((leftSpeedX != 0 && ((dirX > 0 && leftSpeedX > 0) || (dirX < 0 && leftSpeedX < 0))) + (leftSpeedY != 0 && ((dirY > 0 && leftSpeedY > 0) || (dirY < 0 && leftSpeedY < 0)))) : 3;
				rightDanger = CanMove(Ghosts[i], rightSpeedX, rightSpeedY) ? ((rightSpeedX != 0 && ((dirX > 0 && rightSpeedX > 0) || (dirX < 0 && rightSpeedX < 0))) + (rightSpeedY != 0 && ((dirY > 0 && rightSpeedY > 0) || (dirY < 0 && rightSpeedY < 0)))) : 3;
				
				if (currentDanger <= leftDanger && currentDanger <= rightDanger) { break; }
				else if (leftDanger < rightDanger)
				{
					Ghosts[i].SpeedX = leftSpeedX;
					Ghosts[i].SpeedY = leftSpeedY;
				}
				else if (rightDanger < leftDanger)
				{
					Ghosts[i].SpeedX = rightSpeedX;
					Ghosts[i].SpeedY = rightSpeedY;
				}
				else
				{
					TurnRandom(Ghosts[i]);
				}
			}
			break;
			
			
		case BEHAVIOR_EATEN:
			{
				DEBUG("Behavior: EATEN"); // DEBUG
				if (IsAtOrigin(Ghosts[i]) && ++Ghosts[i].BehaviorCounter >= REVIVE_COUNTER && !PacmanOnSpawn())
				{
					DEBUG("Behavior: RANDOM"); // DEBUG
					Ghosts[i].Behavior = BEHAVIOR_RANDOM;
				}
				
				if (GetEatenDirection(Ghosts[i])) break;
				
				//  Get the direction to run. If we're already heading in that direction, break out
				int dirX, dirY, leftSpeedX, leftSpeedY, rightSpeedX, rightSpeedY, currentDanger, leftDanger, rightDanger;
				OriginDelta(Ghosts[i], dirX, dirY, true, true);
				Left(Ghosts[i], leftSpeedX, leftSpeedY);
				Right(Ghosts[i], rightSpeedX, rightSpeedY);
				currentDanger = (Ghosts[i].SpeedX != 0) && ((dirX > 0 && Ghosts[i].SpeedX > 0) || (dirX < 0 && Ghosts[i].SpeedX < 0)) + (Ghosts[i].SpeedY != 0 && ((dirY > 0 && Ghosts[i].SpeedY > 0) || (dirY < 0 && Ghosts[i].SpeedY < 0)));
				leftDanger = CanMove(Ghosts[i], leftSpeedX, leftSpeedY) ? ((leftSpeedX != 0 && ((dirX > 0 && leftSpeedX > 0) || (dirX < 0 && leftSpeedX < 0))) + (leftSpeedY != 0 && ((dirY > 0 && leftSpeedY > 0) || (dirY < 0 && leftSpeedY < 0)))) : 3;
				rightDanger = CanMove(Ghosts[i], rightSpeedX, rightSpeedY) ? ((rightSpeedX != 0 && ((dirX > 0 && rightSpeedX > 0) || (dirX < 0 && rightSpeedX < 0))) + (rightSpeedY != 0 && ((dirY > 0 && rightSpeedY > 0) || (dirY < 0 && rightSpeedY < 0)))) : 3;
				
				if (currentDanger == 0 && CanContinue(Ghosts[i])) { break; }
				else if (leftDanger < rightDanger)
				{
					Ghosts[i].SpeedX = leftSpeedX;
					Ghosts[i].SpeedY = leftSpeedY;
					break;
				}
				else if (rightDanger < leftDanger)
				{
					Ghosts[i].SpeedX = rightSpeedX;
					Ghosts[i].SpeedY = rightSpeedY;
					break;
				}
			}
			break;
			
			
		case BEHAVIOR_RANDOM:
			{
				bool canTurn = CanTurn(Ghosts[i]);
				if (canTurn && random(4) == 0) TurnRandom(Ghosts[i]);
				else
				{
					bool continuing = CanContinue(Ghosts[i]);
					if (continuing == false) TurnRandom(Ghosts[i]);
				}
			}
			break;
		}
	}
}

void PowerPill()
{
	for (int i = 0; i < 4; ++i)
	{
		if (Ghosts[i].Active == true && Ghosts[i].Behavior != BEHAVIOR_EATEN)
		{
			Ghosts[i].Behavior = BEHAVIOR_RUN_AWAY;
			Ghosts[i].BehaviorCounter = 0;
		}
	}
}

void DetermineCharacterPositions()
{
	//  Move Ghosts
	ghostRunAwayDelay = !ghostRunAwayDelay;
	for (int i = 0; i < 4; ++i)
	{
		if (Ghosts[i].Active == false) continue;
		if (Ghosts[i].Behavior != BEHAVIOR_RUN_AWAY || ghostRunAwayDelay != true)
		{
			if ((Ghosts[i].Behavior == BEHAVIOR_EATEN) && IsAtOrigin(Ghosts[i])) continue; //  Don't move if we're waiting at our start point

			Move(Ghosts[i]);
			if (Ghosts[i].X >= MAP_W) Ghosts[i].X = 0;
			if (Ghosts[i].X < 0) Ghosts[i].X = MAP_W - 1;
		}
	}
	
	//  Move Pacman
	if (!attractMode)
	{
		CheckForDeath();
		if (CanContinue(Pacman)) Move(Pacman);
		if (Pacman.X >= MAP_W) Pacman.X = 0;
		if (Pacman.X < 0) Pacman.X = MAP_W - 1;
		CheckForDeath(); //  We check twice instead of just at the end because if we only check after everyone moves, they could move right past each other, swapping spots.
	
		//  If Pacman is on a dot, take it and up the score
		unsigned char dotIndex = GetDotIndexByPosition((MAP_W * Pacman.Y) + Pacman.X);
		if (dotIndex != 255 && IsDotStillAtIndex(dotIndex))
		{
			if (dotIndex >= DEFAULT_DOT_COUNT) PowerPill();
      ClearDotAtIndex(dotIndex);
			Score++;
			setScoreDisplay(Score, Level);
			if ((++DotsCollected) == (DEFAULT_DOT_COUNT + DEFAULT_PILL_COUNT))
			{
				//  The dots are gone. Reset the game and up the WinTransition
				int level = Level;
				int score = Score;
				WinTransition();
				ResetMap();
				Level = ++level;
				Score = score;
				setScoreDisplay(Score, Level);
			}
		}
	}
}

void DrawMap()
{
	if (++ghostBlink == 4) ghostBlink = 0;
	if (++powerPillBlink == 4) powerPillBlink = 0;
	
	//  Set all LEDs to black
	FillColor(0x000000);
	
	//  Draw all of the dots
	static byte dotColorIndex = 0;
	static byte powerPillFrame = 0;
	static char dotIndex = -1;
	
	for (int i = 0; i < MAP_W * MAP_H; ++i)
	{
		dotColorIndex = 0;
		if ((dotIndex = GetDotIndexByPosition(i)) == -1) continue;
    if (!IsDotStillAtIndex(dotIndex)) continue;
		dotColorIndex = ((dotIndex < DEFAULT_DOT_COUNT) ? 0 : (1 + powerPillBlink));

    byte pillColorTone = pgm_read_byte_near(PillColor + dotColorIndex);
		leds[pgm_read_byte_near(LightMap + i)].setRGB(pillColorTone, pillColorTone, pillColorTone);
	}

/*
 Ghosts[0].X = 7;
 Ghosts[0].Y = 8;
 Ghosts[1].X = 10;
 Ghosts[1].Y = 8;
 Ghosts[2].X = 13;
 Ghosts[2].Y = 8;
 Ghosts[3].X = 16;
 Ghosts[3].Y = 8;
 */
	
	//  Draw all active ghosts
	for (char i = 0; i < 4; ++i)
	{
		if (Ghosts[i].Active == false) continue;
		bool eaten = (Ghosts[i].Behavior == BEHAVIOR_EATEN);
		if (eaten && IsAtOrigin(Ghosts[i])) continue;
		bool runningAway = (Ghosts[i].Behavior == BEHAVIOR_RUN_AWAY);
		bool blinking = (runningAway && Ghosts[i].BehaviorCounter > (RUN_AWAY_TIMER / 2));
		bool blinkWhite = (blinking && (ghostBlink > 1));
		long int color = ((eaten || blinkWhite) ? COLOR_DEADGHOST : (runningAway ? COLOR_VULNERABLE : GhostColors[i]));
		leds[pgm_read_byte_near(LightMap + (MAP_W * Ghosts[i].Y) + Ghosts[i].X)] = color;
	}

	//  Draw Pacman
	if (!attractMode) leds[pgm_read_byte_near(LightMap + (MAP_W * Pacman.Y) + Pacman.X)] = COLOR_PACMAN;

	//  Show the new LED layout
	FastLED.show();
}

void CheckForAttractToggle()
{
    if (attractMode)
    {
        if ((digitalRead(Button_F) == LOW) || (digitalRead(Button_U) == LOW) || (digitalRead(Button_D) == LOW) || (digitalRead(Button_L) == LOW) || (digitalRead(Button_R) == LOW))
        {
            AttractModeTransition();
            attractMode = false;
            ResetMap();
            attractTimeout = millis() + ATTRACT_TIMER;
        }
    }
}

void setup()
{
  Wire.begin();
  
	Serial.begin(115200);
	DEBUG("Pac-Man: START");
	
	randomSeed(analogRead(0));

	// Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN, RGB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	FillColor(0x000000);
	FastLED.show();

	//  Enable the reset button input pin
	pinMode(Button_F, INPUT_PULLUP);
	//digitalWrite(Button_F, HIGH); //Enable the pull-up resistor

	//  Enable the joystick input pins
	pinMode(Button_U, INPUT);
	digitalWrite(Button_U, HIGH); //Enable the pull-up resistor
	pinMode(Button_D, INPUT);
	digitalWrite(Button_D, HIGH); //Enable the pull-up resistor
	pinMode(Button_L, INPUT);
	digitalWrite(Button_L, HIGH); //Enable the pull-up resistor
	pinMode(Button_R, INPUT);
	digitalWrite(Button_R, HIGH); //Enable the pull-up resistor
  pinMode(Joystick_Ground, OUTPUT);
  digitalWrite(Joystick_Ground, LOW);
	
	for (char i = 0; i < 4; ++i) directionTimers[i] = 0;

	//sevenSeg.shutdown(0, false);
	/* Set the brightness to a medium values */
	//sevenSeg.setIntensity(0,8);
	/* and clear the display */
	//sevenSeg.clearDisplay(0);

	ResetMap();
}

void loop()
{
  CheckForAttractToggle();

	if (millis() >= turnMillis)
	{
		if (!attractMode)
		{
			DetermineControllerDirection();
			DeterminePacmanDirection();
		}
	
		DetermineGhostsReleased();
		DetermineGhostDirections();
		DetermineCharacterPositions();
		DrawMap();
		StepForward();
	}
}
