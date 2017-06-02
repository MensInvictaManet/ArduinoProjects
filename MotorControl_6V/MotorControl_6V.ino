int motorPin = 2;
bool lightOn = false;
byte motorValue = 0;

unsigned long timerUpdate = 1000;
 
void setup() 
{ 
  pinMode(13, OUTPUT);
  pinMode(motorPin, OUTPUT);
  Serial.begin(9600);
} 
 
 
void loop() 
{ 
  if (millis() > timerUpdate)
  {
    timerUpdate += 1000;
    digitalWrite(13, (lightOn = !lightOn));
    motorValue += 51;
    analogWrite(motorPin, motorValue);
    Serial.println(motorValue);
  }
} 
