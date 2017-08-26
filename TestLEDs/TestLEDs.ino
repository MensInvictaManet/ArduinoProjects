#include <FastLED.h>

#define DEBUG_OUTPUT    0

#define LED_STRIP_PIN   0   //  The NeoPixel string data pin

#define NUM_LEDS        8  //  The number of LEDs we want to alter
#define MAX_LEDS        8  //  The number of LEDs on the full strip
#define BRIGHTNESS  	  40  //  The number (0 to 200) for the brightness setting)
#define DEBUG_OUTPUT    false

CRGB leds[MAX_LEDS];

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

void setup(){
#if DEBUG_OUTPUT
  Serial.begin(9600);
#endif
	
	//  Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	fillColor(CRGB::Black, true);

#if DEBUG_OUTPUT
	Serial.println("Start Demo: Simple Read");
#endif
}

void loop(){
	Blink(CRGB::Red);
	Blink(CRGB::Green);
	Blink(CRGB::Blue);
	Blink(CRGB::Yellow);
	Blink(CRGB::Magenta);
	Blink(CRGB::Cyan);
	Blink(CRGB::White);
}

