#define WIRE_ENABLED 1
#define DEBUG_OUTPUT 1

#if WIRE_ENABLED
  #include <Wire.h>
#endif

int lastState = -1;
int currState = 0;
#define STATIC_SCREEN_CHECK(state) if ((currState = state) == lastState) return; else lastState = currState;

#define SOUND_IGNORE_MAX 10

unsigned int soundRating = 0; 
const int sampleWindow = 15; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
 
void setup() 
{
#if WIRE_ENABLED
  Wire.begin(); 
#endif

  Serial.begin(9600);
}
 
 
void loop() 
{
  unsigned long startMillis = millis();  // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level
  
  unsigned int signalMax = 0;
  unsigned int signalMin = 1000;
  
  // collect data for 50 mSsoundRating
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(A0);
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
  soundRating = max(min(byte(double(peakToPeak) / 4), 255), 0);
  if (soundRating < SOUND_IGNORE_MAX) soundRating = 0; //  Throw out anything too low to register

#if DEBUG_OUTPUT
  Serial.println(soundRating);
#endif

  STATIC_SCREEN_CHECK(soundRating); //  Don't continue if nothing has changed

#if WIRE_ENABLED
  Wire.beginTransmission(9);  // transmit to device #9
  Wire.write(soundRating);    // sends soundRating
  Wire.endTransmission();     // stop transmitting
#endif


}
