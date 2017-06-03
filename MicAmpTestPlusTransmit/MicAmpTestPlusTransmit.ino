#define WIRE_ENABLED 0
#define DEBUG_OUTPUT 1

#if WIRE_ENABLED
  #include <Wire.h>
#endif
 
const int sampleWindow = 10; // Sample window width in mS (50 mS = 20Hz)
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
  
  // collect data for 50 mS
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
  byte soundRating = max(min(byte(double(peakToPeak) / 4), 255), 0);

#if DEBUG_OUTPUT
  //  NOTE: Debug data to show a more rounded output (which will be done on the other side, so we send the original)
  byte soundLevelAltered = (soundRating / 25) * 25;
  if (soundLevelAltered < 40) soundLevelAltered = 0;
  Serial.println(soundLevelAltered);
#endif

#if WIRE_ENABLED
  Wire.beginTransmission(9);  // transmit to device #9
  Wire.write(soundRating);    // sends soundRating
  Wire.endTransmission();     // stop transmitting
#endif
}
