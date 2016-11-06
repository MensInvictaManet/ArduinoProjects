/*
 ADXL362_SimpleRead.ino -  Simple XYZ axis reading example
 for Analog Devices ADXL362 - Micropower 3-axis accelerometer
 go to http://www.analog.com/ADXL362 for datasheet
 
 
 License: CC BY-SA 3.0: Creative Commons Share-alike 3.0. Feel free 
 to use and abuse this code however you'd like. If you find it useful
 please attribute, and SHARE-ALIKE!
 
 Created June 2012
 by Anne Mahaffey - hosted on http://annem.github.com/ADXL362

 Modified May 2013
 by Jonathan Ruiz de Garibay
 
Connect SCLK, MISO, MOSI, and CSB of ADXL362 to
SCLK, MISO, MOSI, and DP 10 of Arduino 
(check http://arduino.cc/en/Reference/SPI for details)
 
*/ 

#include <FastLED.h>

#define LED_STRIP_PIN   10   //  The NeoPixel string data pin

#define NUM_LEDS        5  //  The number of LEDs we want to alter
#define MAX_LEDS        5  //  The number of LEDs on the full strip
#define BRIGHTNESS  	60  //  The number (0 to 200) for the brightness setting)
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
  
  Serial.begin(9600);
	
	//  Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	fillColor(CRGB::Black, true);

	Serial.println("Start Demo: Simple Read");
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

