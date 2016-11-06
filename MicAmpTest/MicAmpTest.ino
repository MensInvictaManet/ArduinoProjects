/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/

#include <FastLED.h>

#define DEBUG_OUTPUT    false

#define LED_STRIP_PIN   10  //  The NeoPixel string data pin
#define NUM_LEDS        30  //  The number of LEDs we want to alter
#define MAX_LEDS        30  //  The number of LEDs on the full strip
#define MIN_BRIGHTNESS  0  	//  The number (0 to 200) for the brightness setting)
#define MAX_BRIGHTNESS  200 //  The number (0 to 200) for the brightness setting)
CRGB leds[MAX_LEDS];
 
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
uint8_t lastBrightness;
 
void setup() 
{
#if DEBUG_OUTPUT
   Serial.begin(9600);
#endif
   
   	//  Setup the LED strip and color all LEDs black
	FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(MIN_BRIGHTNESS);
	for (int i = 0; i < MAX_LEDS; ++i) leds[i] = CRGB::Cyan;
	FastLED.show();
}
 
 
void loop() 
{
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
 
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(1);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double voltage = peakToPeak / 1024.0;  // convert to volts
   voltage *= (voltage * ((1.0 - voltage) / 2.0)); //  Half-Square the voltage to create an exponential scale
   
  	uint8_t brightness = MIN_BRIGHTNESS + uint8_t(voltage * (MAX_BRIGHTNESS - MIN_BRIGHTNESS));
  	if ((brightness < lastBrightness) || ((brightness - lastBrightness) > 4))
  	{
#if DEBUG_OUTPUT
   		Serial.println(brightness);
#endif
  		lastBrightness = brightness;
		FastLED.setBrightness(brightness);
		FastLED.show();
  	}
}