#include <JeeLib.h>
#include <FastLED.h>
#include <IRremote.h>

ISR(WDT_vect) { Sleepy::watchdogEvent(); }

#define IR_RECEIVE_PIN  10   //  The IR Receiver data pin
#define LED_STRIP_PIN   9   //  The NeoPixel string data pin

#define NUM_LEDS        36  //  The number of LEDs we want to alter
#define MAX_LEDS        60  //  The number of LEDs on the full strip
#define BRIGHTNESS      60  //  The number (0 to 200) for the brightness setting)
#define DEBUG_OUTPUT    true
#define MESSAGE_DELAY	250

const uint16_t BUTTON_POWER = 0xD827; // i.e. 0x10EFD827
const uint16_t BUTTON_A = 0xF807;
const uint16_t BUTTON_B = 0x7887;
const uint16_t BUTTON_C = 0x58A7;
const uint16_t BUTTON_UP = 0xA05F;
const uint16_t BUTTON_DOWN = 0x00FF;
const uint16_t BUTTON_LEFT = 0x10EF;
const uint16_t BUTTON_RIGHT = 0x807F;
const uint16_t BUTTON_CIRCLE = 0x20DF;

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;
uint16_t lastCode = 0;

unsigned int rainbow1Iterator = 0;
unsigned int rainbow2Iterator = 0;

CRGB leds[MAX_LEDS];

enum DisplayState
{
	POWER_SAVE				= 1,
	BASIC					= 2,
	RAINBOW1				= 3,
	RAINBOW2				= 4,
	SETTING_COUNT
};

enum ColorNames 
{
	CRIMSON,
	PINK,
	VIOLET_RED,
	ORCHID,
	PURPLE,
	SLATE_BLUE,
	BLUE,
	ROYAL_BLUE,
	SKY_BLUE,
	TURQUOISE,
	SPRING_GREEN,
	COBALT_GREEN,
	GREEN,
	OLIVE_DRAB,
	YELLOW,
	GOLD,
	ORANGE,
	ORANGE_RED,
	TOMATO,
	RED,
	BLACK,
	COLOR_COUNT
};

DisplayState CurrentState = POWER_SAVE;
int CurrentColor = 0;

const CRGB Colors[COLOR_COUNT] = 
{
	CRGB( 220, 20, 60 ),	// CRIMSON
	CRGB( 255, 152, 163 ),	// PINK
	CRGB( 208, 32, 144 ),	// VIOLET_RED
	CRGB( 139, 71, 137 ),	// ORCHID
	CRGB( 155, 48, 255 ),	// PURPLE
	CRGB( 131, 111, 255 ),	// SLATE_BLUE
	CRGB( 0, 0, 255 ),		// BLUE
	CRGB( 65, 105, 225),	// ROYAL_BLUE
	CRGB( 135, 206, 255),	// SKY_BLUE
	CRGB( 0, 245, 255),		// TURQUOISE
	CRGB( 0, 250, 154),		// SPRING_GREEN
	CRGB( 61, 145, 64),		// COBALT_GREEN
	CRGB( 0, 255, 0 ),		// GREEN
	CRGB( 179, 238, 58),	// OLIVE_DRAB
	CRGB( 255, 255, 0),		// YELLOW
	CRGB( 255, 215, 0),		// GOLD
	CRGB( 255, 165, 0 ),	// ORANGE
	CRGB( 255, 69, 0),		// ORANGE_RED
	CRGB( 255, 99, 71),		// TOMATO
	CRGB( 255, 0, 0 ),		// RED
	CRGB( 0, 0, 0 ),		// BLACK
};

CRGB Wheel(byte WheelPos) {
	if (WheelPos < 85) { return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0); }
	else if (WheelPos < 170) { WheelPos -= 85; return CRGB(255 - WheelPos * 3, 0, WheelPos * 3); }
	else { WheelPos -= 170; return CRGB(0, WheelPos * 3, 255 - WheelPos * 3); }
}

// Fill the dots one after the other with a color
void setColor(CRGB color)
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

  for( int j = 0; j < NUM_LEDS; j++) leds[j] = color;
  FastLED.show(); // display this frame
}

void ResetValues()
{
	rainbow1Iterator = 0;
	rainbow2Iterator = 0;
	//CurrentColor = 0;
	//if (CurrentState == BASIC) setColor(Colors[CurrentColor]);
}

void DisplayRainbow1(unsigned long updateDelay)
{
	static unsigned long nextRunTime = 0;
	unsigned long currentTime = millis();
	while (nextRunTime > currentTime) { delay(nextRunTime - currentTime); currentTime = millis(); }
 
 	if (rainbow1Iterator >= 256) rainbow1Iterator = 0;
 	for (int j = 0; j < NUM_LEDS; j++) leds[j] = Wheel(rainbow1Iterator);
 	FastLED.show();
	nextRunTime = currentTime + updateDelay;
 	rainbow1Iterator += 5;
}

void DisplayRainbow2(unsigned long updateDelay)
{
	static unsigned long nextRunTime = 0;
	unsigned long currentTime = millis();
	while (nextRunTime > currentTime) { delay(nextRunTime - currentTime); currentTime = millis(); }
	
 	if (rainbow2Iterator >= 256) rainbow2Iterator = 0;
 	const int ledsInRow = NUM_LEDS / 3;
 	for (int i = 0; i < ledsInRow; i++)
 		for (int j = 0; j < 3; ++j)
 			leds[i + ledsInRow * j] = Wheel(((i * 256 / ledsInRow) + rainbow2Iterator) & 255);
 			
 	FastLED.show();
	nextRunTime = currentTime + updateDelay;
 	rainbow2Iterator += 7;
}

void CheckForMessages()
{
	static bool delayAfterMessage = false;
	static unsigned long messageDelay = 0;
	unsigned long currentTime = millis();
	
	if (delayAfterMessage && (messageDelay <= currentTime))
	{
		irrecv.resume(); // Receive the next value
		delayAfterMessage = false;
	}
	
	if (!delayAfterMessage && (irrecv.decode(&results)))
	{
		messageDelay = currentTime + MESSAGE_DELAY;
		delayAfterMessage = true;
		
		/* read the RX'd IR into a 16-bit variable: */
		uint16_t resultCode = (results.value & 0xFFFF);
		
		/* The remote will continue to spit out 0xFFFFFFFF if a 
		button is held down. If we get 0xFFFFFFF, let's just
		assume the previously pressed button is being held down */
		if (resultCode == 0xFFFF)
			resultCode = lastCode;
		else
			lastCode = resultCode;
	
		// This switch statement checks the received IR code against
		// all of the known codes. Each button press produces a 
		// serial output, and has an effect on the LED output.
		switch (resultCode)
		{
		case BUTTON_POWER:
			Serial.println("Power");
			CurrentState = POWER_SAVE;
			setColor(Colors[BLACK]);
			break;
		case BUTTON_A:
			Serial.println("A");
			CurrentState = BASIC;
			if (CurrentColor == BLACK) CurrentColor = 0;
			setColor(Colors[CurrentColor]);
			break;
		case BUTTON_B:
			Serial.println("B");
			CurrentState = RAINBOW1;
			break;
		case BUTTON_C:
			Serial.println("C");
			CurrentState = RAINBOW2;
			break;
		case BUTTON_UP:
			Serial.println("Up");
			if (CurrentState == BASIC)
			{
				CurrentColor += 1;
				if (CurrentColor >= BLACK) CurrentColor = 0;
				Serial.println(CurrentColor);
				setColor(Colors[CurrentColor]);
			}
			break;
		case BUTTON_DOWN:
			Serial.println("Down");
			if (CurrentState == BASIC)
			{
				CurrentColor -= 1;
				if (CurrentColor < 0) CurrentColor = BLACK - 1;
				Serial.println(CurrentColor);
				setColor(Colors[CurrentColor]);
			}
			break;
		case BUTTON_LEFT:
			Serial.println("Left");
			break;
		case BUTTON_RIGHT:
			Serial.println("Right");
			break;
		case BUTTON_CIRCLE:
			Serial.println("Circle");
			ResetValues();
			break;
		default:
			Serial.print("Unrecognized code received: 0x");
			Serial.println(results.value, HEX);
			break;        
		}
	}
	else if (CurrentState == RAINBOW1) DisplayRainbow1(300);
	else if (CurrentState == RAINBOW2) DisplayRainbow2(300);
}

void setup()
{
	//  Give power to the radio receiver
	irrecv.enableIRIn();
	
	//  Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	for (int i = 0; i < MAX_LEDS; ++i) leds[0] = CRGB::Black;
	FastLED.show();
	
	//  Set up the serial connection for debugging
	Serial.begin(9600);
#if DEBUG_OUTPUT
	Serial.write("Program started\n");
#endif
}

void loop()
{
	if (CurrentState == POWER_SAVE)
	{
		Sleepy::loseSomeTime(10);
	}
	CheckForMessages();
}