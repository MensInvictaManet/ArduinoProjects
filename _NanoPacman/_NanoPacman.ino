#include <FastLED.h>
#include <LedControl.h>

////////////////////////////////////////
///////////// GAME DATA ////////////////
////////////////////////////////////////

#define BRIDGE_LEVEL	10
#define STEP_DELAY		132
#define RUN_AWAY_TIMER	40
#define CHASE_TIMER		300
#define REVIVE_COUNTER	60
#define ATTRACT_TIMER	1000

#define MAP_W			24
#define MAP_H			23
#define XXX				0

#define COLOR_PACMAN	CRGB::Yellow

static const CRGB GhostColors[6] = { CRGB::Red, CRGB::Magenta, CRGB::Aquamarine, CRGB::DarkOrange, CRGB::DarkBlue, CRGB::DarkGray };

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
	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, XXX, XXX,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,
	 33, XXX, XXX, XXX,  30, XXX, XXX, XXX, XXX, XXX,  29, XXX, XXX,  26, XXX, XXX, XXX, XXX, XXX,  25, XXX, XXX, XXX,  22,
	 32, XXX, XXX, XXX,  31, XXX, XXX, XXX, XXX, XXX,  28, XXX, XXX,  27, XXX, XXX, XXX, XXX, XXX,  24, XXX, XXX, XXX,  23,
	 34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,
	 78, XXX, XXX, XXX,  84, XXX, XXX,  77, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,  64, XXX, XXX, 100, XXX, XXX, XXX,  58,
	 79, XXX, XXX, XXX,  85, XXX, XXX,  76, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,  65, XXX, XXX, 101, XXX, XXX, XXX,  59,
	 80,  81,  82,  83,  86, XXX, XXX,  75,  74,  73,  72, XXX, XXX,  69,  68,  67,  66, XXX, XXX, 102,  63,  62,  61,  60,
	XXX, XXX, XXX, XXX,  87, XXX, XXX, XXX, XXX, XXX,  71, XXX, XXX,  70, XXX, XXX, XXX, XXX, XXX, 103, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX,  88, XXX, XXX,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, XXX, XXX, 104, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX,  89, XXX, XXX, 130, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 113, XXX, XXX, 105, XXX, XXX, XXX, XXX,
	137, 136, 135, 134, 133, 132, 131, 129, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 114, 106, 107, 108, 109, 110, 111, 112,
	XXX, XXX, XXX, XXX, 138, XXX, XXX, 128, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 115, XXX, XXX, 141, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX, 139, XXX, XXX, 127, 125, 124, 123, 122, 121, 120, 119, 118, 116, XXX, XXX, 142, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX, 140, XXX, XXX, 126, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 117, XXX, XXX, 143, XXX, XXX, XXX, XXX,
	165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, XXX, XXX, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144,
	166, XXX, XXX, XXX, 172, XXX, XXX, XXX, XXX, XXX, 255, XXX, XXX, 252, XXX, XXX, XXX, XXX, XXX, 217, XXX, XXX, XXX, 218,
	167, XXX, XXX, XXX, 173, XXX, XXX, XXX, XXX, XXX, 254, XXX, XXX, 253, XXX, XXX, XXX, XXX, XXX, 216, XXX, XXX, XXX, 219,
	168, 169, XXX, XXX, 174, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 215, XXX, XXX, 221, 220,
	XXX, 170, XXX, XXX, 175, XXX, XXX, 237, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 224, XXX, XXX, 214, XXX, XXX, 222, XXX,
	XXX, 171, XXX, XXX, 176, XXX, XXX, 236, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 225, XXX, XXX, 213, XXX, XXX, 223, XXX,
	181, 180, 179, 178, 177, XXX, XXX, 235, 234, 233, 232, XXX, XXX, 229, 228, 227, 226, XXX, XXX, 212, 211, 210, 209, 208,
	182, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 231, XXX, XXX, 230, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 207,
	183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206
};

//  Dot Types: 0 (No Dot), 1 (Basic Dot), 2 (Power Pill)
const PROGMEM byte PillColor[5] = { 40, 255, 128, 0, 128 };
const PROGMEM byte DefaultDotMap[MAP_W * MAP_H] = 
{
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,	// 24
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 48
	2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, // 72
	0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, // 96
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 120
	1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, // 144
	0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, // 168
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, // 192
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 216
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, // 240
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 264
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, // 288
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 312
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, // 336
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, // 360
	1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, // 384
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 408
	2, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 2, // 432
	0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, // 456
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, // 480
	0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, // 504
	
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 528
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, // 552
};

#define DEFAULT_DOT_COUNT 98
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

char GetDotIndex(int space)
{
	for (int i = 0; i < DEFAULT_DOT_COUNT; ++i)
		if (pgm_read_word_near(DotPositions + i) == space) return i;
		
	for (int i = 0; i < DEFAULT_PILL_COUNT; ++i)
		if (pgm_read_word_near(PillPositions + i) == space) return DEFAULT_DOT_COUNT + i;
		
	return -1;
}

byte DotsRemainingByteFlag[13]; //  98 bits representing the Dots remaining. 4 bits representing the Power Pills. 2 bits unused. 

inline bool IsMapPosition(unsigned char x, unsigned char y) 	{ return ((x == 0 && y == 0) || (pgm_read_byte_near(LightMap + (MAP_W * y) + x) != 0)); }
bool CanMove(const Character& c, int x, int y)					{ return (((c.Y == BRIDGE_LEVEL) && ((c.X + x == -1) || c.X + x == MAP_W)) || ((c.X + x >= 0) && (c.X + x < MAP_W) && (c.Y + y >= 0) && (c.Y + y < MAP_H) && (IsMapPosition(c.X + x, c.Y + y)))); }
inline bool CanContinue(const Character& c)						{ return CanMove(c, c.SpeedX, c.SpeedY); }
inline bool CanTurn(const Character& c)							{ return (CanMove(c, c.SpeedY, c.SpeedX) || CanMove(c, -c.SpeedY, -c.SpeedX)); }
inline void Move(Character& c)									{ c.X += c.SpeedX; c.Y += c.SpeedY; }
inline bool IsAtOrigin(Character& c)							{ return (c.X == c.originX && c.Y == c.originY); }
inline bool PacmanOnSpawn()										{ return ((Pacman.Y == 8) && (Pacman.X >= 7) && (Pacman.X <= 16)) || (Pacman.X == 10 && (Pacman.Y == 7 || Pacman.Y == 6)); }

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

#define LED_STRIP_PIN   10	//  The NeoPixel string data pin
#define BUTTON_PIN		9	//  The pin that controls the button signal
#define Button_U 		5
#define Button_D 		6
#define Button_L 		7
#define Button_R 		8

const uint16_t BUTTON_POWER = 0xD827; // i.e. 0x10EFD827
const uint16_t BUTTON_A = 0xF807;
const uint16_t BUTTON_B = 0x7887;
const uint16_t BUTTON_C = 0x58A7;
const uint16_t BUTTON_UP = 0xA05F;
const uint16_t BUTTON_DOWN = 0x00FF;
const uint16_t BUTTON_LEFT = 0x10EF;
const uint16_t BUTTON_RIGHT = 0x807F;
const uint16_t BUTTON_CIRCLE = 0x20DF;

#define NUM_LEDS		256	//  The number of LEDs we want to access
#define BRIGHTNESS  	80	//  The number (0 to 200) for the brightness setting)

CRGB 	leds[NUM_LEDS];
#define FILL_COLOR(color)		memset(leds, color, sizeof(CRGB) * NUM_LEDS);

unsigned long turnMillis = 0;
Direction controllerDirection;
bool ghostRunAwayDelay = false;

char directionTimers[4];
char ghostBlink = 0;
char powerPillBlink = 0;
bool attractMode = false;

LedControl sevenSeg = LedControl(13,12,11,1);

void setScoreDisplay(int score, int level)
{
	if (!attractMode)
	{
		sevenSeg.clearDisplay(0);
		
		//  Show the score, from 0 to 9999
		score = min(9999, score);
		if (score >= 0)				sevenSeg.setDigit(0, 0, score % 10, false);
		if (score >= 10)			sevenSeg.setDigit(0, 1, (score % 100 - score % 10) / 10, false);
		if (score >= 100)			sevenSeg.setDigit(0, 2, (score % 1000 - score % 100) / 100, false);
		if (score >= 1000)			sevenSeg.setDigit(0, 3, (score % 10000 - score % 1000) / 1000, false);
		
		//  Show the level, from 1 to 9
		level = min(8, level) + 1;
		if (level >= 0)				sevenSeg.setDigit(0, 7, level, false);
	}
	else
	{
		sevenSeg.clearDisplay(0);
	}
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
	FILL_COLOR(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	FastLED.show();
	delay(250);
	FILL_COLOR(0x151515);
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
	FILL_COLOR(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	delay(250);
	FILL_COLOR(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	delay(250);
	FILL_COLOR(0x151515);
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
	FILL_COLOR(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	delay(250);
	FILL_COLOR(0x151515);
	FastLED.show();
	delay(250);
	DrawMap();
	delay(250);
	FILL_COLOR(0x151515);
	FastLED.show();
	delay(500);
	StepForwardAllTheWay();
	SetControllerDirection(DIRECTION_NONE);
}

void EatTransition()
{
	//  Set all LEDs to white, then render
	FILL_COLOR(0x151515);
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
	for (int i = 0; i < 13; ++i) DotsRemainingByteFlag[i] = 255; //  Set all dots and power pills to be active according to the bit flag
	
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
		Serial.println("Can't spawn while Pacman is in the spawn area.");
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

	if 		(dirU && !dirD && !dirL && !dirR)	SetControllerDirection(DIRECTION_U);
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
				Serial.println("Behavior: EATEN"); // DEBUG
				if (IsAtOrigin(Ghosts[i]) && ++Ghosts[i].BehaviorCounter >= REVIVE_COUNTER && !PacmanOnSpawn())
				{
					Serial.println("Behavior: RANDOM"); // DEBUG
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
		int dotIndex = -1;
		if ((dotIndex = GetDotIndex((MAP_W * Pacman.Y) + Pacman.X)) != -1)
		{
			if (DotsRemainingByteFlag[dotIndex / 8] & (1 << (dotIndex % 8)))
			{
				if (dotIndex >= DEFAULT_DOT_COUNT) PowerPill();
				DotsRemainingByteFlag[dotIndex / 8] ^= (1 << (dotIndex % 8));
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
}

void DrawMap()
{
	if (++ghostBlink == 4) ghostBlink = 0;
	if (++powerPillBlink == 4) powerPillBlink = 0;
	
	//  Set all LEDs to black
	FILL_COLOR(CRGB::Black);
	
	//  Draw all of the dots
	static byte dotColor = 0;
	static byte powerPillFrame = 0;
	static char dotIndex = -1;
	
	for (int i = 0; i < MAP_W * MAP_H; ++i)
	{
		dotColor = 0;
		if ((dotIndex = GetDotIndex(i)) == -1) continue;
		if (!(DotsRemainingByteFlag[dotIndex / 8] & (1 << (dotIndex % 8)))) continue;
		dotColor = ((dotIndex < DEFAULT_DOT_COUNT) ? 0 : (1 + powerPillBlink));
		
		leds[pgm_read_byte_near(LightMap + i)].setRGB(pgm_read_byte_near(PillColor + dotColor - 1), pgm_read_byte_near(PillColor + dotColor - 1), pgm_read_byte_near(PillColor + dotColor - 1));
	}
	
	//  Draw all active ghosts
	for (char i = 0; i < 4; ++i)
	{
		if (Ghosts[i].Active == false) continue;
		bool eaten = (Ghosts[i].Behavior == BEHAVIOR_EATEN);
		if (eaten && IsAtOrigin(Ghosts[i])) continue;
		bool runningAway = (Ghosts[i].Behavior == BEHAVIOR_RUN_AWAY);
		bool blinking = (runningAway && Ghosts[i].BehaviorCounter > (RUN_AWAY_TIMER / 2));
		bool blinkWhite = (blinking && (ghostBlink > 1));
		CRGB color = ((eaten || blinkWhite) ? GhostColors[5] : (!runningAway ? GhostColors[i] : GhostColors[4]));
		leds[pgm_read_byte_near(LightMap + (MAP_W * Ghosts[i].Y) + Ghosts[i].X)] = color;
	}

	//  Draw Pacman
	if (!attractMode) leds[pgm_read_byte_near(LightMap + (MAP_W * Pacman.Y) + Pacman.X)] = COLOR_PACMAN;

	//  Show the new LED layout
	FastLED.show();
}

void setup()
{
	Serial.begin(115200);
	Serial.println("Pac-Man: START");
	
	randomSeed(analogRead(0));

	// Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	FILL_COLOR(CRGB::Black);
	FastLED.show();

	//  Enable the reset button input pin
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	//digitalWrite(BUTTON_PIN, HIGH); //Enable the pull-up resistor

	//  Enable the joystick input pins
	pinMode(Button_U, INPUT);
	digitalWrite(Button_U, HIGH); //Enable the pull-up resistor
	pinMode(Button_D, INPUT);
	digitalWrite(Button_D, HIGH); //Enable the pull-up resistor
	pinMode(Button_L, INPUT);
	digitalWrite(Button_L, HIGH); //Enable the pull-up resistor
	pinMode(Button_R, INPUT);
	digitalWrite(Button_R, HIGH); //Enable the pull-up resistor
	
	for (char i = 0; i < 4; ++i) directionTimers[i] = 0;

	sevenSeg.shutdown(0, false);
	/* Set the brightness to a medium values */
	sevenSeg.setIntensity(0,8);
	/* and clear the display */
	sevenSeg.clearDisplay(0);

	ResetMap();
}

void loop()
{
	static unsigned long buttonAttractTimer = 0;
	if (digitalRead(BUTTON_PIN) == LOW)
	{
		if (buttonAttractTimer == 0) buttonAttractTimer = millis();
		else if (millis() - buttonAttractTimer > ATTRACT_TIMER)
		{
			if (attractMode) AttractModeTransition();
			attractMode = !attractMode;
			ResetMap();
			buttonAttractTimer = 0;
		}
	}
	else buttonAttractTimer = 0;

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

