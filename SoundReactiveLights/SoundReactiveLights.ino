#include <FastLED.h>

#define LED_STRIP_PIN   9   //  The NeoPixel string data pin
#define MIC_PIN         A0

#define NUM_LEDS        21  //  The number of LEDs we want to alter
#define MAX_LEDS        21  //  The number of LEDs on the full strip
#define BRIGHTNESS      60  //  The number (0 to 200) for the brightness setting)
#define DEBUG_OUTPUT    false

#define DC_OFFSET       0  // DC offset in mic signal - if unusure, leave 0
#define NOISE           100  // Noise/hum/interference in mic signal
#define SAMPLES         60  // Length of buffer for dynamic level adjustment
#define TOP             (NUM_LEDS +1) // Allow dot to go slightly off scale

byte
  peak      = 0,      // Used for falling dot
  dotCount  = 0,      // Frame counter for delaying dot-falling speed
  volCount  = 0;      // Frame counter for storing past volume data
  
int
  vol[SAMPLES],       // Collection of prior volume samples
  lvl       = 10,     // Current "dampened" audio level
  minLvlAvg = 0,      // For dynamic adjustment of graph low & high
  maxLvlAvg = 512;

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

  pinMode(MIC_PIN, INPUT);
  
  //  Setup the LED strip and color all LEDs black
  //FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
  //FastLED.setBrightness(BRIGHTNESS);
  //fillColor(CRGB::Black, true);

  memset(vol,0,sizeof(int)*SAMPLES);//Thanks Neil!

  Serial.println("Start Demo: Simple Read");
}

uint8_t getMicValue() {
  uint8_t  i;
  uint16_t minLvl, maxLvl;
  int      n, height;
  n   = analogRead(MIC_PIN);                 // Raw reading from mic 

  Serial.print(n);
  
  n   = abs(n - 512 - DC_OFFSET);            // Center on zero
  n   = (n <= NOISE) ? 0 : (n - NOISE);      // Remove noise/hum
  lvl = ((lvl * 7) + n) >> 3;    // "Dampened" reading (else looks twitchy)
  
  Serial.print(" : ");
  Serial.print(lvl);
  Serial.print(" : ");
  
  // Calculate bar height based on dynamic min/max levels (fixed point):
  height = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

  Serial.println(height);
 
  if(height < 0L)       height = 0;      // Clip output
  else if(height > TOP) height = TOP;
  if(height > peak)     peak   = height; // Keep 'peak' dot at top

  // Color pixels based on rainbow gradient
  for(i = 0; i < NUM_LEDS; i++) {  
    if(i >= height)               
      leds[i] = CRGB::Black;
    else
    {
      uint8_t mapValue = map(i,0,NUM_LEDS - 1,100,150);
      leds[i] = CRGB(mapValue, mapValue, mapValue);
    }
  }

  FastLED.show();
}

void loop(){
  getMicValue();
}

