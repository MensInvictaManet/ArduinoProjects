#include <FastLED.h>

////////////////////////////////////////////////////////////////////////////////
//  LED Strip primary settings                                                //
////////////////////////////////////////////////////////////////////////////////

#define LED_STRIP_TYPE  WS2812B   //  'WS2811' / 'WS2812' / 'WS2812B'
#define COLOR_PATTERN   GRB       //  'RGB' for some light types, 'GRB' for others
#define BRIGHTNESS      200       //  The number (0 to 200) for the brightness setting)
#define NUM_LEDS        8         //  The number of LEDs we want to alter
#define MAX_LEDS        NUM_LEDS  //  The number of LEDs on the full strip
#define LED_STRIP_PIN   0         //  The LED strip data pin


////////////////////////////////////////////////////////////////////////////////
//  Program settings                                                          //
////////////////////////////////////////////////////////////////////////////////

#define DEBUG_OUTPUT    0


////////////////////////////////////////////////////////////////////////////////
//  Program data                                                              //
////////////////////////////////////////////////////////////////////////////////

CRGB leds[MAX_LEDS];

#define CRGB_PACMAN   CRGB(230, 240, 0)
#define CRGB_BLINKY   CRGB::Red
#define CRGB_PINKY    CRGB::Magenta
#define CRGB_INKY     CRGB::Aqua
#define CRGB_CLYDE    CRGB(255, 130, 0)


////////////////////////////////////////////////////////////////////////////////
//  Utility methods                                                           //
////////////////////////////////////////////////////////////////////////////////

/*
 * Fills the strip with a single color, then displays the frame
 */
void fillColor(CRGB color, bool fullStrip = false, bool render = true)
{
#if DEBUG_OUTPUT
    Serial.print(String(color[0]) + " - " + String(color[1]) + " - " + String(color[2] + "\n");
    delay(5);
#endif

    //  Set either NUM_LEDS or MAX_LEDS (depending on the given boolean) to the given color, then display the frame
    for (int i = 0; i < (fullStrip ? MAX_LEDS : NUM_LEDS); i++) leds[i] = color;
    if (render) FastLED.show();
}

/*
 * Fills the strip with a single color, waits a given amount of milliseconds, then switches the strip to black
 */
void Blink(CRGB color, int milliWait = 750)
{
    fillColor(color);
    delay(milliWait);
    fillColor(CRGB::Black);
}


////////////////////////////////////////////////////////////////////////////////
//  Pattern methods                                                           //
////////////////////////////////////////////////////////////////////////////////

void Pattern_PacmanChase()
{
    const unsigned int delayFramesBefore = 1;
    const unsigned int delayFramesAfter = 1;
    const unsigned int frameCount = 10 - 1 + NUM_LEDS;
    const unsigned int fullFrameCount = delayFramesBefore + frameCount + delayFramesAfter;
    const unsigned long frameDelay = 200;
    const unsigned long currentFrame = millis() / frameDelay % fullFrameCount;

    fillColor(CRGB::Black, false, false);
    if ((currentFrame >= delayFramesBefore) && (currentFrame < delayFramesBefore + frameCount))
    {
        unsigned int frameIndex = currentFrame - delayFramesBefore;
        if ((frameIndex + 0 < NUM_LEDS) && (frameIndex + 0 >= 0)) leds[frameIndex + 0] = CRGB_PACMAN;
        if ((frameIndex - 3 < NUM_LEDS) && (frameIndex - 3 >= 0)) leds[frameIndex - 3] = CRGB_BLINKY;
        if ((frameIndex - 5 < NUM_LEDS) && (frameIndex - 5 >= 0)) leds[frameIndex - 5] = CRGB_PINKY;
        if ((frameIndex - 7 < NUM_LEDS) && (frameIndex - 7 >= 0)) leds[frameIndex - 7] = CRGB_INKY;
        if ((frameIndex - 9 < NUM_LEDS) && (frameIndex - 9 >= 0)) leds[frameIndex - 9] = CRGB_CLYDE;
    }
    FastLED.show();
}

void Pattern_GlowFlow()
{
    const unsigned long delayTime = 125;
    const unsigned long hueChange = 1;

    fill_rainbow(leds, NUM_LEDS, byte(millis() / delayTime * hueChange), -1);
    FastLED.show();
}

////////////////////////////////////////////////////////////////////////////////
//  Setup and Loop                                                            //
////////////////////////////////////////////////////////////////////////////////

void setup()
{
#if DEBUG_OUTPUT
    Serial.begin(9600);
#endif
	
    //  Setup the LED strip and color all LEDs black
    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_PATTERN>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
    fillColor(CRGB::Black, true);

#if DEBUG_OUTPUT
    Serial.println("Start Demo: Simple Read");
#endif
}

void loop()
{
    //Pattern_PacmanChase();
    Pattern_GlowFlow();
}

