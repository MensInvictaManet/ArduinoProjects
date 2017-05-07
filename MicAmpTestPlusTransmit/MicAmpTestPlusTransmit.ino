/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/

#include <Wire.h>
 
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
 
void setup() 
{
  Wire.begin(); 
  Serial.begin(9600);
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
  double volts = (peakToPeak * 5.0) / 1024;  // convert to volts
  
  int x = int(volts * 100.0f);
  Serial.println(x);
  
  Wire.beginTransmission(9); // transmit to device #9
  Wire.write(x);              // sends "volts" 
  Wire.endTransmission();    // stop transmitting
}
