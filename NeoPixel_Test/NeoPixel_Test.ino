
#include <FastLED.h>

#define LED_STRIP_PIN   	9   //  The NeoPixel string data pin
#define BUTTON_PIN        6   //  The pin that controls the button signal
#define NUM_LEDS        	30  //  The number of LEDs we want to alter
#define MAX_LEDS        	30  //  The number of LEDs on the full strip
#define NUM_LEDS_VIRTUAL	46  //  The number of LEDs to virtually travel. AKA the loop will continue until the position passes this number
#define BRIGHTNESS      	60  //  The number (0 to 200) for the brightness setting)
#define BUTTON_DELAY      250 //  The amount of milliseconds between button checks
#define DEBUG_OUTPUT    	true

unsigned long buttonTimer = 0;

CRGB leds[MAX_LEDS];
int position = 0;

int colorIndex = 1;
#define COLOR_COUNT 4
CRGB colors[COLOR_COUNT] = { CRGB::Black, CRGB::Red, CRGB::Green, CRGB::Blue };

void SetStripColor(int colorIndex)
{
  fill_solid(leds, MAX_LEDS, colors[colorIndex]);
  FastLED.show();
}

void IterateColor()
{
  ++colorIndex;
  if (colorIndex >= COLOR_COUNT) colorIndex = 1;
  SetStripColor(colorIndex);
  Serial.print("Setting strip color: "); Serial.println(colorIndex);
}

inline bool IsPositionOnStrip(int position)
{
  return ((position >= 0) && (position < NUM_LEDS));
}
void setup(){
	//  Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	SetStripColor(colorIndex);

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // connect internal pull-up
  buttonTimer = millis();
  
  Serial.begin(9600);
  while (!Serial) { ; }
  Serial.println("Program START!");
}

void loop()
{
  unsigned long currentMillis = millis();
  if (buttonTimer <= currentMillis)
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      buttonTimer = currentMillis + BUTTON_DELAY;
      IterateColor();
    }
  }
}

