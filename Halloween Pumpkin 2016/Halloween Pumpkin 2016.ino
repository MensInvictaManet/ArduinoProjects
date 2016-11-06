//  Controller should only change the arduinoPins depending on where we plug in to the Arduino
const int arduinoPins[32] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53 };
const int rowPinIndex[8] = { 8, 13, 0, 11, 7, 1, 6, 3 };
const int colPinIndex[8] = { 12, 5, 4, 9, 2, 10, 14, 15 };

const int MovementLeftInputPin = 8;
const int MovementRightInputPin = 10;
bool movementDetectedLeft = false;
bool movementDetectedRight = false;

float timeCount = 0;

enum CharacterIndex
{
	LEFT2			= 0,
	LEFT1			= 1,
	CENTER			= 2,
	RIGHT1			= 3,
	RIGHT2			= 4,
	CHARACTER_COUNT
};

enum EyesLocation
{
	EYES_LEFT 		= 0,
	EYES_CENTER 	= 1,
	EYES_RIGHT 		= 2,
	EYE_LOCATION_COUNT
};

int animationFrameData[CHARACTER_COUNT][8][8] = 
{
	{	// Left 2
		{0,0,1,1,1,1,0,0},
		{0,1,0,0,0,0,1,0},
		{1,0,0,0,0,0,0,1},
		{1,1,1,0,0,0,0,1},
		{1,1,1,0,0,0,0,1},
		{1,0,0,0,0,0,0,1},
		{0,1,0,0,0,0,1,0},
		{0,0,1,1,1,1,0,0}
	},
	{	// Left 1
		{0,0,1,1,1,1,0,0},
		{0,1,0,0,0,0,1,0},
		{1,0,0,0,0,0,0,1},
		{1,0,1,1,0,0,0,1},
		{1,0,1,1,0,0,0,1},
		{1,0,0,0,0,0,0,1},
		{0,1,0,0,0,0,1,0},
		{0,0,1,1,1,1,0,0}
	},
	{	// Center
		{0,0,1,1,1,1,0,0},
		{0,1,0,0,0,0,1,0},
		{1,0,0,0,0,0,0,1},
		{1,0,0,1,1,0,0,1},
		{1,0,0,1,1,0,0,1},
		{1,0,0,0,0,0,0,1},
		{0,1,0,0,0,0,1,0},
		{0,0,1,1,1,1,0,0}
	},
	{	// Right 1
		{0,0,1,1,1,1,0,0},
		{0,1,0,0,0,0,1,0},
		{1,0,0,0,0,0,0,1},
		{1,0,0,0,1,1,0,1},
		{1,0,0,0,1,1,0,1},
		{1,0,0,0,0,0,0,1},
		{0,1,0,0,0,0,1,0},
		{0,0,1,1,1,1,0,0}
	},
	{	// Right 2
		{0,0,1,1,1,1,0,0},
		{0,1,0,0,0,0,1,0},
		{1,0,0,0,0,0,0,1},
		{1,0,0,0,0,1,1,1},
		{1,0,0,0,0,1,1,1},
		{1,0,0,0,0,0,0,1},
		{0,1,0,0,0,0,1,0},
		{0,0,1,1,1,1,0,0}
	}
};

EyesLocation currentEyesLocation = EYES_CENTER;
const int eyesLeftFrameCount = 1;
const CharacterIndex eyesLeftAnimation[eyesLeftFrameCount] = { LEFT2 };
const int eyesCenterFrameCount = 1;
const CharacterIndex eyesCenterAnimation[eyesCenterFrameCount] = { CENTER };
const int eyesRightFrameCount = 1;
const CharacterIndex eyesRightAnimation[eyesRightFrameCount] = { RIGHT2 };

const int eyesCenterToRightFrameCount = 2;
const CharacterIndex EyesCenterToRightAnimation[eyesCenterToRightFrameCount] = { RIGHT1, RIGHT2 };
const int eyesRightToCenterFrameCount = 2;
const CharacterIndex EyesRightToCenterAnimation[eyesRightToCenterFrameCount] = { RIGHT1, CENTER };
const int eyesRightToLeftFrameCount = 4;
const CharacterIndex EyesRightToLeftAnimation[eyesRightToLeftFrameCount] = { RIGHT1, CENTER, LEFT1, LEFT2 };
const int eyesCenterToLeftFrameCount = 2;
const CharacterIndex EyesCenterToLeftAnimation[eyesCenterToLeftFrameCount] = { LEFT1, LEFT2 };
const int eyesLeftToRightFrameCount = 4;
const CharacterIndex EyesLeftToRightAnimation[eyesLeftToRightFrameCount] = { LEFT1, CENTER, RIGHT1, RIGHT2 };
const int eyesLeftToCenterFrameCount = 2;
const CharacterIndex EyesLeftToCenterAnimation[eyesLeftToCenterFrameCount] = { LEFT1, CENTER };

const CharacterIndex* currentAnimation = eyesCenterAnimation;
int currentAnimationFrameCount = eyesCenterFrameCount;

const int animationFrames = 20;
const CharacterIndex animation[animationFrames] = { LEFT2, LEFT2, LEFT2, LEFT1, CENTER, CENTER, CENTER, RIGHT1, RIGHT2, RIGHT2, RIGHT2, RIGHT2, RIGHT2, RIGHT1, CENTER, CENTER, CENTER, LEFT1, LEFT2, LEFT2 };
int currentFrame = 0;

//  Animation drawing data
static unsigned long timeStart = millis();
static unsigned long lastIndex = 999;
static unsigned long millisecondsPerFrame = 70;

void LightPinTest()
{
	for (int i = 0; i < 64; ++i)
	{
		pinMode(arduinoPins[rowPinIndex[i / 8]], OUTPUT);
		pinMode(arduinoPins[colPinIndex[i % 8]], OUTPUT);
		digitalWrite(arduinoPins[rowPinIndex[i / 8]], HIGH);
		digitalWrite(arduinoPins[colPinIndex[i % 8]], LOW);
		
		pinMode(arduinoPins[rowPinIndex[i / 8] + 16], OUTPUT);
		pinMode(arduinoPins[colPinIndex[i % 8] + 16], OUTPUT);
		digitalWrite(arduinoPins[rowPinIndex[i / 8] + 16], HIGH);
		digitalWrite(arduinoPins[colPinIndex[i % 8] + 16], LOW);
		
		delay(250);
		
		pinMode(arduinoPins[rowPinIndex[i / 8]], INPUT);
		pinMode(arduinoPins[colPinIndex[i % 8]], INPUT);
		
		pinMode(arduinoPins[rowPinIndex[i / 8] + 16], INPUT);
		pinMode(arduinoPins[colPinIndex[i % 8] + 16], INPUT);
	}
}

void drawScreen(int character[8][8])
{
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			// draw some letter bits
			if(character[i][7-j] == 1)
			{
			    pinMode(arduinoPins[rowPinIndex[i]], OUTPUT);
			    pinMode(arduinoPins[colPinIndex[j]], OUTPUT);
				digitalWrite(arduinoPins[rowPinIndex[i]], HIGH);
				digitalWrite(arduinoPins[colPinIndex[j]], LOW);
				
			    pinMode(arduinoPins[rowPinIndex[i] + 16], OUTPUT);
			    pinMode(arduinoPins[colPinIndex[j] + 16], OUTPUT);
				digitalWrite(arduinoPins[rowPinIndex[i] + 16], HIGH);
				digitalWrite(arduinoPins[colPinIndex[j] + 16], LOW);
      		}
      		
      		pinMode(arduinoPins[rowPinIndex[i]], INPUT);
			pinMode(arduinoPins[colPinIndex[j]], INPUT);
			
      		pinMode(arduinoPins[rowPinIndex[i] + 16], INPUT);
			pinMode(arduinoPins[colPinIndex[j] + 16], INPUT);
    	}
	}
}

void DrawCurrentAnimationFrame(int frameIndex)
{
	frameIndex = min(frameIndex, currentAnimationFrameCount - 1);
	drawScreen(animationFrameData[currentAnimation[frameIndex]]);
}

void AnimateEyesToLeft()
{
	if (currentEyesLocation == EYES_LEFT) return;
	
	if (currentEyesLocation == EYES_CENTER)
	{
		Serial.println("Eyes going center to left");
		currentAnimation = EyesCenterToLeftAnimation;
		currentAnimationFrameCount = eyesCenterToLeftFrameCount;
	}
	else if (currentEyesLocation == EYES_RIGHT)
	{
		Serial.println("Eyes going right to left");
		currentAnimation = EyesRightToLeftAnimation;
		currentAnimationFrameCount = eyesRightToLeftFrameCount;
	}
	
	timeStart = millis();
	currentEyesLocation = EYES_LEFT;
}

void AnimateEyesToCenter()
{
	if (currentEyesLocation == EYES_CENTER) return;
	
	if (currentEyesLocation == EYES_LEFT)
	{
		Serial.println("Eyes going left to center");
		currentAnimation = EyesLeftToCenterAnimation;
		currentAnimationFrameCount = eyesLeftToCenterFrameCount;
	}
	else if (currentEyesLocation == EYES_RIGHT)
	{
		Serial.println("Eyes going right to center");
		currentAnimation = EyesRightToCenterAnimation;
		currentAnimationFrameCount = eyesRightToCenterFrameCount;
	}
	
	timeStart = millis();
	currentEyesLocation = EYES_CENTER;
}

void AnimateEyesToRight()
{
	if (currentEyesLocation == EYES_RIGHT) return;
	
	if (currentEyesLocation == EYES_LEFT)
	{
		Serial.println("Eyes going left to right");
		currentAnimation = EyesLeftToRightAnimation;
		currentAnimationFrameCount = eyesLeftToRightFrameCount;
	}
	else if (currentEyesLocation == EYES_CENTER)
	{
		Serial.println("Eyes going center to right");
		currentAnimation = EyesCenterToRightAnimation;
		currentAnimationFrameCount = eyesCenterToRightFrameCount;
	}
	
	timeStart = millis();
	currentEyesLocation = EYES_RIGHT;
}

void setup()
{
  	Serial.begin(9600);
  
	//  Make sure all pins are completely off, set to INPUT
	for (int i = 0; i < 8; i++)
	{
		pinMode(arduinoPins[rowPinIndex[i]], INPUT);
		pinMode(arduinoPins[colPinIndex[i]], INPUT);
		
		pinMode(arduinoPins[rowPinIndex[i] + 16], INPUT);
		pinMode(arduinoPins[colPinIndex[i] + 16], INPUT);
	}
	
	
	for (int j = 0; j < 8; ++j)
	{
  		pinMode(arduinoPins[rowPinIndex[j]], INPUT);
		pinMode(arduinoPins[colPinIndex[j]], INPUT);
		
  		pinMode(arduinoPins[rowPinIndex[j] + 16], INPUT);
		pinMode(arduinoPins[colPinIndex[j] + 16], INPUT);
	}
	
	pinMode(MovementLeftInputPin, INPUT);
	pinMode(MovementRightInputPin, INPUT);
}

void loop()
{
	int movementLeft = 0;
	int movementRight = 0;
	movementLeft = digitalRead(MovementLeftInputPin);
	movementRight = digitalRead(MovementRightInputPin);
	if (movementLeft == HIGH && movementDetectedLeft == false && currentEyesLocation != EYES_LEFT)
	{
    	Serial.println("Movement Detected Left!");
    	movementDetectedLeft = true;
    	AnimateEyesToLeft();
	}
	else if (movementRight == HIGH && movementDetectedRight == false && currentEyesLocation != EYES_RIGHT)
	{
    	Serial.println("Movement Detected Right!");
    	movementDetectedRight = true;
    	AnimateEyesToRight();
	}
	else if (movementLeft == LOW && movementDetectedLeft == true)
	{
    	Serial.println("Movement Left Stopped!");
    	movementDetectedLeft = false;
    	if (movementDetectedRight == false) AnimateEyesToCenter();
	}
	else if (movementRight == LOW && movementDetectedRight == true)
	{
    	Serial.println("Movement Right Stopped!");
    	movementDetectedRight = false;
    	if (movementDetectedLeft == false) AnimateEyesToCenter();
	}
	
	//  Draw the current frame by determining the current animation and frame time
	const unsigned long timeCurrent = millis();
	const unsigned long timeDelta = timeCurrent - timeStart;
	const unsigned long frameIndex = timeDelta / millisecondsPerFrame;
	DrawCurrentAnimationFrame(frameIndex);
}