
#include <FastLED.h>

#define LED_STRIP_PIN_1   	9   //  The NeoPixel string data pin
#define LED_STRIP_PIN_2   	10   //  The NeoPixel string data pin

#define NUM_LEDS        	30  //  The number of LEDs we want to alter
#define MAX_LEDS        	30  //  The number of LEDs on the full strip
#define NUM_LEDS_VIRTUAL	46  //  The number of LEDs to virtually travel. AKA the loop will continue until the position passes this number
#define BRIGHTNESS      	60  //  The number (0 to 200) for the brightness setting)
#define DEBUG_OUTPUT    	true

CRGB leds[MAX_LEDS];
int position = 0;

void ClearColors()
{
	for (int i = 0; i < MAX_LEDS; ++i) leds[i] = CRGB::Black;
}

inline bool IsPositionOnStrip(int position)
{
	return ((position >= 0) && (position < NUM_LEDS));
}

void setup(){
	//  Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN_2, GRB>(&leds[MAX_LEDS / 2], MAX_LEDS / 2).setCorrection( TypicalLEDStrip );
	FastLED.addLeds<WS2811, LED_STRIP_PIN_1, GRB>(leds, MAX_LEDS / 2).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	ClearColors();
	FastLED.show();
}

CRGB colors[8] = { CRGB::Black, CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, CRGB::Cyan, CRGB::Magenta };

int positionOffset(const int& pos)
{
	return (pos < NUM_LEDS / 2) ? (NUM_LEDS / 2) - 1 - pos : pos;
}

void PacmanChase()
{
	ClearColors();
	int P1 = ((position >= NUM_LEDS_VIRTUAL) ? 0 : position);
	int P2 = (((P1 - 3) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 3) : (P1 - 3));
	int P3 = (((P1 - 5) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 5) : (P1 - 5));
	int P4 = (((P1 - 7) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 7) : (P1 - 7));
	int P5 = (((P1 - 9) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 9) : (P1 - 9));
	
	if (IsPositionOnStrip(P1)) leds[positionOffset(P1)] = CRGB::Yellow;
	if (IsPositionOnStrip(P2)) leds[positionOffset(P2)] = CRGB::Red;
	if (IsPositionOnStrip(P3)) leds[positionOffset(P3)] = CRGB::Cyan;
	if (IsPositionOnStrip(P4)) leds[positionOffset(P4)] = CRGB::Orange;
	if (IsPositionOnStrip(P5)) leds[positionOffset(P5)] = CRGB::Magenta;
	
	FastLED.show();
	position = P1 + 1;
	delay(150);
}

void loop()
{
	PacmanChase();
}

