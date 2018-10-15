#include <FastLED.h>
#include <TrueRandom.h>

////////////////////////////////////////////////////////////////////////////////
//  LED Strip Primary Settings                                                //
////////////////////////////////////////////////////////////////////////////////

#define LED_STRIP_TYPE    WS2812B     //  'WS2811' / 'WS2812' / 'WS2812B'
#define COLOR_PATTERN     GRB         //  'RGB' for some light types, 'GRB' for others
#define BRIGHTNESS        200         //  The number (0 to 200) for the brightness setting)
#define NUM_LEDS          8           //  The number of LEDs we want to alter
#define LED_STRIP_PIN     0           //  The LED strip data pin


////////////////////////////////////////////////////////////////////////////////
//  Program Data                                                              //
////////////////////////////////////////////////////////////////////////////////

CRGB leds[NUM_LEDS];


////////////////////////////////////////////////////////////////////////////////
//  Utility Methods                                                           //
////////////////////////////////////////////////////////////////////////////////

void fillColor(CRGB color, bool render = true)
{
    //  Set all LEDs to the given color, then display the frame
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = color;
    if (render) FastLED.show();
}


void Blink(CRGB color, int milliWait = 750)
{
    fillColor(color);
    delay(milliWait);
    fillColor(CRGB::Black);
}


////////////////////////////////////////////////////////////////////////////////
//  Pattern Methods                                                           //
////////////////////////////////////////////////////////////////////////////////

void Pattern_PacmanChase(CRGB* ledArray, unsigned int ledCount, unsigned long delayTime = 200)
{
    static CRGB CRGB_PACMAN   = CRGB(230, 240, 0);
    static CRGB CRGB_BLINKY   = CRGB::Red;
    static CRGB CRGB_PINKY    = CRGB::Magenta;
    static CRGB CRGB_INKY     = CRGB::Aqua;
    static CRGB CRGB_CLYDE    = CRGB(255, 130, 0);

    const unsigned int delayFramesBefore = 1;
    const unsigned int delayFramesAfter = 1;
    const unsigned int frameCount = 10 - 1 + ledCount;
    const unsigned int fullFrameCount = delayFramesBefore + frameCount + delayFramesAfter;
    const unsigned long currentFrame = millis() / delayTime % fullFrameCount;

    fillColor(CRGB::Black, false);
    if ((currentFrame >= delayFramesBefore) && (currentFrame < delayFramesBefore + frameCount))
    {
        unsigned int frameIndex = currentFrame - delayFramesBefore;
        if ((frameIndex + 0 < ledCount) && (frameIndex + 0 >= 0)) ledArray[frameIndex + 0] = CRGB_PACMAN;
        if ((frameIndex - 3 < ledCount) && (frameIndex - 3 >= 0)) ledArray[frameIndex - 3] = CRGB_BLINKY;
        if ((frameIndex - 5 < ledCount) && (frameIndex - 5 >= 0)) ledArray[frameIndex - 5] = CRGB_PINKY;
        if ((frameIndex - 7 < ledCount) && (frameIndex - 7 >= 0)) ledArray[frameIndex - 7] = CRGB_INKY;
        if ((frameIndex - 9 < ledCount) && (frameIndex - 9 >= 0)) ledArray[frameIndex - 9] = CRGB_CLYDE;
    }
    FastLED.show();
}

void Pattern_GlowFlow(CRGB* ledArray, unsigned int ledCount, unsigned long delayTime = 250, unsigned long hueChange = 1)
{
    fill_rainbow(ledArray, ledCount, byte(millis() / delayTime * hueChange), -1);
    FastLED.show();
}

void Pattern_BasicBlink(CRGB* ledArray, unsigned int ledCount, CRGB::HTMLColorCode color = CRGB::Red, unsigned long delayTime = 750)
{
    fill_solid(ledArray, ledCount, (millis() % (delayTime * 2) > delayTime) ? color : CRGB::Black);
    FastLED.show();
}

void Pattern_StarField()
{
    byte pixelState[NUM_LEDS];
    CRGB pixelTargetColor[NUM_LEDS];
    unsigned long pixelSettingTime[NUM_LEDS];
    double pixelToColorTime[NUM_LEDS];
    double percentage = 0.0;

    for (int i = 0; i < NUM_LEDS; ++i)
    {
        pixelState[i] = 3;
        pixelSettingTime[i] = millis();
        pixelToColorTime[i] = TrueRandom.random(4000);
    }

    unsigned long startTime = millis();
    while (millis() < startTime + 100000)
    {
        for (int i = 0; i < NUM_LEDS; ++i)
        {
            if (pixelState[i] == 0) // Ready to set to a new color
            {
                pixelTargetColor[i] = CRGB(TrueRandom.random(255), TrueRandom.random(255), TrueRandom.random(255));
                delay(3);
                pixelState[i] = 1;
                pixelSettingTime[i] = millis();
                pixelToColorTime[i] = 1000 + TrueRandom.random(1000); // Time it takes to translate to a new color
            }
            else if (pixelState[i] == 1) // Translating to a new color
            {
                percentage = double(millis() - pixelSettingTime[i]);
                if (percentage > pixelToColorTime[i]) percentage = pixelToColorTime[i];
                percentage /= pixelToColorTime[i];
                leds[i] = CRGB((pixelTargetColor[i].r * percentage), (pixelTargetColor[i].g * percentage), (pixelTargetColor[i].b * percentage));
                if (millis() > pixelSettingTime[i] + (unsigned long)(pixelToColorTime[i])) pixelState[i] = 2;
            }
            else if (pixelState[i] == 2) // Translating back to black
            {
                percentage = millis() - (pixelSettingTime[i] + (unsigned long)(pixelToColorTime[i]));
                if (percentage > (unsigned long)(pixelToColorTime[i])) percentage = (unsigned long)(pixelToColorTime[i]);
                percentage = double(percentage) / double(pixelToColorTime[i]);
                leds[i] = CRGB(byte(pixelTargetColor[i].r * (1.0 - percentage)), byte(pixelTargetColor[i].g * (1.0 - percentage)), byte(pixelTargetColor[i].b * (1.0 - percentage)));
                if (millis() > pixelSettingTime[i] + pixelToColorTime[i] * 2)
                {
                    pixelState[i] = 3;
                    pixelSettingTime[i] = millis();
                    pixelToColorTime[i] = TrueRandom.random(10000); // Time it takes to choose a new color
                }
            }
            else if (pixelState[i] == 3)
            {
                if (millis() > pixelSettingTime[i] + (unsigned long)(pixelToColorTime[i])) pixelState[i] = 0;
            }
            FastLED.show();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//  Setup and Loop                                                            //
////////////////////////////////////////////////////////////////////////////////

void setup()
{
    //  Setup the LED strip and color all LEDs black
    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_PATTERN>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
    fillColor(CRGB::Black);

#if DEBUG_OUTPUT
    Serial.println("Start Demo: Simple Read");
#endif
}

void loop()
{
    //Pattern_PacmanChase(leds, NUM_LEDS);
    //Pattern_GlowFlow(leds, NUM_LEDS);
    //Pattern_BasicBlink(leds, NUM_LEDS, CRGB::Red);
    Pattern_StarField();
}

