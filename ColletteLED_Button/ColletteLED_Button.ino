
#include <FastLED.h>

#define LED_STRIP_PIN   9   //  The NeoPixel string data pin
#define BUTTON_PIN		10	//  The pin that controls the button signal

#define NUM_LEDS        30  //  The number of LEDs we want to alter
#define MAX_LEDS        30  //  The number of LEDs on the full strip
#define BRIGHTNESS      60  //  The number (0 to 200) for the brightness setting)
#define DEBUG_OUTPUT    true

CRGB leds[MAX_LEDS];

unsigned long buttonTimer = 0;

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

void Blink(CRGB color)
{
#if DEBUG_OUTPUT
	Serial.println("Button Pressed!");
#endif
	
	setColor(color);
	delay(1000);
	setColor(CRGB::Black);
}

void setup(){
	Serial.begin(9600);
	
	pinMode(BUTTON_PIN, INPUT_PULLUP); // connect internal pull-up
	
	//  Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	for (int i = 0; i < MAX_LEDS; ++i) leds[0] = CRGB::Black;
	FastLED.show();
	
	buttonTimer = millis();
	
	Serial.println("Program Start!");
}

void loop(){
	unsigned long currentMillis = millis();
	if (buttonTimer < currentMillis)
	{
		if (digitalRead(BUTTON_PIN) == LOW)
		{
			buttonTimer = currentMillis + 1000;
			Blink(CRGB::Cyan);
		}
	}
}

