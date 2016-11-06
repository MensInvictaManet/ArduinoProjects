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
#include <SPI.h>
#include <ADXL362.h>

#define LED_STRIP_PIN   8   //  The NeoPixel string data pin

#define NUM_LEDS        30  //  The number of LEDs we want to alter
#define MAX_LEDS        30  //  The number of LEDs on the full strip
#define BRIGHTNESS      60  //  The number (0 to 200) for the brightness setting)
#define DEBUG_OUTPUT    true
#define MESSAGE_DELAY	250

CRGB leds[MAX_LEDS];

ADXL362 xl;
int16_t XValue1, YValue1, ZValue1, Temperature1;
int16_t XValue2, YValue2, ZValue2, Temperature2;
const int swingValue = 400;

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
  	//  Print out the values
	Serial.print("XVALUE=");
	Serial.print(XValue1);	 
	Serial.print("\tYVALUE=");
	Serial.print(YValue1);	 
	Serial.print("\tZVALUE=");
	Serial.print(ZValue1);	 
	Serial.print("\tTEMPERATURE=");
	Serial.println(Temperature1);
	
	setColor(color);
	delay(1000);
	setColor(CRGB::Black);
	
	//  Set the saved values to the current to prevent a double-blink
	xl.readXYZTData(XValue1, YValue1, ZValue1, Temperature1);
	XValue2 = XValue1;
	YValue2 = YValue1;
	ZValue2 = ZValue1;
	Temperature2 = Temperature1;
}

void setup(){
	Serial.begin(9600);
	xl.begin(10);                   // Setup SPI protocol, issue device soft reset
	xl.beginMeasure();              // Switch ADXL362 to measure mode  
	
	//  Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	for (int i = 0; i < MAX_LEDS; ++i) leds[0] = CRGB::Black;
	FastLED.show();
	
	Serial.println("Start Demo: Simple Read");
}

void loop(){
	// read all three axis in burst to ensure all measurements correspond to same sample time
	xl.readXYZTData(XValue1, YValue1, ZValue1, Temperature1);
	
	if (abs(XValue2 - XValue1) > swingValue) 				Blink(CRGB::Cyan);
	else if (abs(YValue2 - YValue1) > swingValue) 			Blink(CRGB::Cyan);
	else if (abs(ZValue2 - ZValue1) > swingValue) 			Blink(CRGB::Cyan);
	
	XValue2 = XValue1;
	YValue2 = YValue1;
	ZValue2 = ZValue1;
	Temperature2 = Temperature1;
	
	delay(100);                // Arbitrary delay to make serial monitor easier to observe
}

