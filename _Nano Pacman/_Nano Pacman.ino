#include <FastLED.h>
#include <LedControl2.h>

#include "GameData.h"

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
bool attractMode = true;

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

