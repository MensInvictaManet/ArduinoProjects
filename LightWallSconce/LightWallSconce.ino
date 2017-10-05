#include <FastLED.h> // Include the FastLED library
#include <Wire.h>

#define DEBUG_LEVEL 0

#define BUTTON_DELAY 500

/* Connect the data line of the LED strip to pin 9. */
#define LED_STRIP_PIN   10 

#define PATTERN_BUTTON_PIN        2  //  The pin that controls the button signal
#define SUBPATTERN_BUTTON_PIN     3  //  The pin that controls the button signal
#define PAUSE_BUTTON_PIN          4  //  The pin that controls the button signal

#define LIGHTS_PER_SCONCE   8
#define LIGHT_SCONCE_COUNT  8
byte CurrentIndex = 0;
byte CurrentMode = 0; // Modes = { RainbowFlow, MusicReact, BaseLight }
byte CurrentSubMode = 0; // Modes = { RainbowFlow:{ RainbowSpread, RainbowCover }, MusicReact:{ BlackToWhite, BlackToColorSpread }, BaseLight:{ StillColor, Flash } }
bool Paused = false;

#define NUM_LEDS        LIGHTS_PER_SCONCE * LIGHT_SCONCE_COUNT  //  The number of LEDs we want to alter
#define MAX_LEDS        NUM_LEDS  //  The number of LEDs on the full strip
#define BRIGHTNESS      60  //  The number (0 to 200) for the brightness setting)
CRGB leds[MAX_LEDS];

int wireTransmission = 0;
unsigned long ButtonTimer = 0;

const unsigned long lightUpdateDelay = 250;
unsigned long lightUpdateTime = lightUpdateDelay;

CRGB Wheel(byte wheelPosition)
{
    wheelPosition = 255 - wheelPosition;
    if(wheelPosition < 85)
    {
        return CRGB(255 - wheelPosition * 3, 0, wheelPosition * 3);
    }
    else if(wheelPosition < 170)
    {
        wheelPosition -= 85;
        return CRGB(0, wheelPosition * 3, 255 - wheelPosition * 3);
    }
    else
    {
        wheelPosition -= 170;
        return CRGB(wheelPosition * 3, 255 - wheelPosition * 3, 0);
    }
}

byte ReturnIndex(byte index, byte IndexCount)
{
  return (index % IndexCount);
}

void fillColor(CRGB color, bool fullStrip = false)
{
  for( int j = 0; j < (fullStrip ? MAX_LEDS : NUM_LEDS); j++) leds[j] = color;
  FastLED.show(); // display this frame
}

void ControlLights()
{
  int brightness = min(wireTransmission * 2, 255);
  Serial.println(wireTransmission);
  if (brightness < 90) brightness = 0;
  else if (brightness < 120) brightness /= 3;
  CRGB brightnessColor = CRGB(brightness, brightness, brightness);
  
  switch (CurrentMode)
  {
    case 0: // RainbowFlow
      if (CurrentSubMode == 0) // RainbowSpread)
      {
        for (int i = 0; i < LIGHT_SCONCE_COUNT; ++i)
          for (int j = 0; j < LIGHTS_PER_SCONCE; ++j)
            leds[i * LIGHTS_PER_SCONCE + j] = Wheel(ReturnIndex(CurrentIndex + (i * (256 / LIGHT_SCONCE_COUNT)), 256));
      }
      else if (CurrentSubMode == 1) // RainbowCover
      {
          for (int i = 0; i < NUM_LEDS; ++i) leds[i] = Wheel(ReturnIndex(CurrentIndex, 256));
      }
      break;

    case 1: // MusicReact
      if (CurrentSubMode == 1) // BlackToWhite
      {
        if (brightness == 0) fillColor(CRGB::Black, true);
        else for (int i = 0; i < NUM_LEDS; ++i) leds[i] = brightnessColor;
      }
      else if (CurrentSubMode == 0) // BlackToColorSpread
      {
        if (brightness == 0) fillColor(CRGB::Black, true);
        else
          for (int i = 0; i < LIGHT_SCONCE_COUNT; ++i)
            for (int j = 0; j < LIGHTS_PER_SCONCE; ++j)
              leds[i * LIGHTS_PER_SCONCE + j] = Wheel(ReturnIndex(CurrentIndex + (i * (256 / LIGHT_SCONCE_COUNT)), 256));
      }
      break;

    case 2: // BaseLight
      for (int i = 0; i < NUM_LEDS; ++i) leds[i] = CRGB::White;
      break;
  }

  FastLED.show();
}

void receiveEvent(int bytes) {
  wireTransmission = Wire.read();    // read one character from the I2C
}

void setup()
{
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
  
  Serial.begin(9600);

  pinMode(PATTERN_BUTTON_PIN, INPUT_PULLUP); // connect internal pull-up
  pinMode(SUBPATTERN_BUTTON_PIN, INPUT_PULLUP); // connect internal pull-up
  pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP); // connect internal pull-up
  ButtonTimer = millis();
  
  FastLED.addLeds<WS2811, LED_STRIP_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  fillColor(CRGB::Black, true);

  ControlLights();
  
  Serial.begin(9600); // Use serial to debug. 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  if (DEBUG_LEVEL >= 1) Serial.println("Program Start!");
}

void loop() 
{
  unsigned long currentMillis = millis();
  if (ButtonTimer < currentMillis)
  {
    if (digitalRead(PATTERN_BUTTON_PIN) == LOW)
    {
      ButtonTimer = currentMillis + 250;
      CurrentMode = ReturnIndex(CurrentMode + 1, 3);
      CurrentSubMode = 0;
      lightUpdateTime = millis();
      Paused = false;
    }
    else if (digitalRead(SUBPATTERN_BUTTON_PIN) == LOW)
    {
      ButtonTimer = currentMillis + 250;
      ControlLights();
      CurrentSubMode = ReturnIndex(CurrentSubMode - 1, 2);
    }
    else if (digitalRead(PAUSE_BUTTON_PIN) == LOW)
    {
      ButtonTimer = currentMillis + 250;
      Paused = !Paused;
    }
  }

  bool regularUpdate = (millis() > lightUpdateTime);
  bool mode2Update = (millis() > 50);
  if (regularUpdate || ((CurrentMode == 1)  && mode2Update))
  {
    if (!Paused && regularUpdate) CurrentIndex += 5;
    if (regularUpdate) lightUpdateTime += lightUpdateDelay;
    
    ControlLights();
  }
}
