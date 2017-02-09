#define BUTTON_PIN      4	//  The pin that controls the button signal
#define DEBUG_OUTPUT    false
#define BUTTON_DELAY    250

unsigned long buttonTimer = 0;

void setup(){
	Serial.begin(9600);
	
	pinMode(BUTTON_PIN, INPUT_PULLUP);  // connect internal pull-up
  
  pinMode(13, OUTPUT);                //  Set the on-board LED to OUTPUT
  digitalWrite(13, LOW);             //  Set the on-board LED to be OFF
  
	buttonTimer = millis();
	
	Serial.println("Program Start!");
}

void loop(){
	unsigned long currentMillis = millis();
	if (buttonTimer < currentMillis)
	{
		if (digitalRead(BUTTON_PIN) == LOW)
		{
      buttonTimer = currentMillis + BUTTON_DELAY;
			digitalWrite(13, HIGH);
      delay(500);
      digitalWrite(13, LOW);
		}
	}
}

