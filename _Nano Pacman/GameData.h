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
	 32, XXX, XXX, XXX,  30, XXX, XXX, XXX, XXX, XXX,  28, XXX, XXX,  26, XXX, XXX, XXX, XXX, XXX,  24, XXX, XXX, XXX,  22,
	 33, XXX, XXX, XXX,  31, XXX, XXX, XXX, XXX, XXX,  29, XXX, XXX,  27, XXX, XXX, XXX, XXX, XXX,  25, XXX, XXX, XXX,  23,
	 34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,
	115, XXX, XXX, XXX, 239, XXX, XXX, 112, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 101, XXX, XXX, 100, XXX, XXX, XXX,  58,
	116, XXX, XXX, XXX, 240, XXX, XXX, 113, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 102, XXX, XXX,  99, XXX, XXX, XXX,  59,
	117, 118, 119, 120, 241, XXX, XXX, 114, 111, 110, 109, XXX, XXX, 106, 105, 104, 103, XXX, XXX,  98,  61,  62,  63,  60,
	XXX, XXX, XXX, XXX, 242, XXX, XXX, XXX, XXX, XXX, 108, XXX, XXX, 107, XXX, XXX, XXX, XXX, XXX,  97, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX, 243, XXX, XXX, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, XXX, XXX,  96, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX, 244, XXX, XXX, 232, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 209, XXX, XXX,  95, XXX, XXX, XXX, XXX,
	121, 122, 123, 124, 245, 237, 238, 233, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 210, 207, 208,  94,  64,  65,  66,  67,
	XXX, XXX, XXX, XXX, 246, XXX, XXX, 234, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 211, XXX, XXX,  93, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX, 247, XXX, XXX, 235, 214, 215, 216, 217, 218, 219, 220, 221, 212, XXX, XXX,  92, XXX, XXX, XXX, XXX,
	XXX, XXX, XXX, XXX, 248, XXX, XXX, 236, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 213, XXX, XXX,  91, XXX, XXX, XXX, XXX,
	125, 126, 127, 128, 249, 191, 192, 193, 194, 195, 196, XXX, XXX, 201, 202, 203, 204, 205, 206,  90,  68,  69,  70,  71,
	129, XXX, XXX, XXX, 250, XXX, XXX, XXX, XXX, XXX, 197, XXX, XXX, 200, XXX, XXX, XXX, XXX, XXX,  89, XXX, XXX, XXX,  72,
	130, XXX, XXX, XXX, 251, XXX, XXX, XXX, XXX, XXX, 198, XXX, XXX, 199, XXX, XXX, XXX, XXX, XXX,  88, XXX, XXX, XXX,  73,
	131, 132, XXX, XXX, 252, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 163,  87, XXX, XXX,  75,  74,
	XXX, 133, XXX, XXX, 253, XXX, XXX, 177, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 190, XXX, XXX,  86, XXX, XXX,  76, XXX,
	XXX, 134, XXX, XXX, 254, XXX, XXX, 178, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 189, XXX, XXX,  85, XXX, XXX,  77, XXX,
	138, 135, 136, 137, 255, XXX, XXX, 179, 180, 181, 182, XXX, XXX, 185, 186, 187, 188, XXX, XXX,  84,  78,  79,  80,  81,
	139, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, 183, XXX, XXX, 184, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX,  82,
	140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162,  83
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