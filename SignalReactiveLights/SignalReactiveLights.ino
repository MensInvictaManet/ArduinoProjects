#include <Wire.h>
#include <FastLED.h>

#define LED_STRIP_PIN   8   //  The NeoPixel string data pin

#define NUM_LEDS        30  //  The number of LEDs we want to alter
#define MAX_LEDS        30  //  The number of LEDs on the full strip
#define BRIGHTNESS      60  //  The number (0 to 200) for the brightness setting)
#define DEBUG_OUTPUT    false

#define MINMAX_TIMER    333
#define MINMAX_SPLIT    75

CRGB leds[MAX_LEDS];

int x = 0;
unsigned long lastMillis = 0;


double maxX = -9999;
double minX = 9999;

// Fill the dots one after the other with a color
void fillColor(CRGB color, bool fullStrip = false, int count = NUM_LEDS)
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

  for( int j = 0; j < (fullStrip ? MAX_LEDS : count); j++) leds[j] = color;
  FastLED.show(); // display this frame
}


void Blink(CRGB color, int milliWait = 1000)
{
  fillColor(color);
  delay(milliWait);
  fillColor(CRGB::Black);
}


void setup()
{
  Serial.begin(9600);
  
  // Start the I2C Bus as Slave on address 9, then attach a function to trigger when something is received.
  Wire.begin(9);
  Wire.onReceive(receiveEvent);

  //  Setup the LED strip and color all LEDs black
  FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  fillColor(CRGB::Black, true);
}


void receiveEvent(int bytes)
{
  x = Wire.read();    // read one character from the I2C
  if (double(x) > maxX) maxX = double(x);
  if (double(x) < minX) minX = double(x);
}


void loop()
{
  if (millis() > lastMillis + MINMAX_TIMER)
  {
    lastMillis = millis();
    if (maxX > minX + MINMAX_SPLIT) maxX -= 1;
    Serial.print(minX);
    Serial.print(" - ");
    Serial.println(maxX);
  }
  
  int ledCount = int(((double(x) - minX) / (maxX - minX)) * double(NUM_LEDS));
  fillColor(CRGB::Black);
  fillColor(CRGB::Aqua, false, ledCount);
  FastLED.show();
}
