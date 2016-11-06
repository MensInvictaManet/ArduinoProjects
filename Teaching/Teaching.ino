#include <IRremote.h>

#define IR_RECEIVE_PIN  10   //  The IR Receiver data pin

const int LightPin_1 = 2;
const int LightPin_2 = 3;
const int LightPin_4 = 4;
const int LightPin_8 = 5;

const int ButtonPin_U = 6;
const int ButtonPin_D = 7;
const int ButtonPin_R = 8;

#define MESSAGE_DELAY	250

const uint16_t BUTTON_POWER = 0xD827; // i.e. 0x10EFD827
const uint16_t BUTTON_A = 0xF807;
const uint16_t BUTTON_B = 0x7887;
const uint16_t BUTTON_C = 0x58A7;
const uint16_t BUTTON_UP = 0xA05F;
const uint16_t BUTTON_DOWN = 0x00FF;
const uint16_t BUTTON_LEFT = 0x10EF;
const uint16_t BUTTON_RIGHT = 0x807F;
const uint16_t BUTTON_CIRCLE = 0x20DF;

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;
uint16_t lastCode = 0;

int Counter = 0;
unsigned long buttonTimer = 0;

void CheckForMessages()
{
	static bool delayAfterMessage = false;
	static unsigned long messageDelay = 0;
	unsigned long currentTime = millis();
	
	if (delayAfterMessage && (messageDelay <= currentTime))
	{
		irrecv.resume(); // Receive the next value
		delayAfterMessage = false;
	}
	
	if (!delayAfterMessage && (irrecv.decode(&results)))
	{
		messageDelay = currentTime + MESSAGE_DELAY;
		delayAfterMessage = true;
		
		/* read the RX'd IR into a 16-bit variable: */
		uint16_t resultCode = (results.value & 0xFFFF);
		
		/* The remote will continue to spit out 0xFFFFFFFF if a 
		button is held down. If we get 0xFFFFFFF, let's just
		assume the previously pressed button is being held down */
		if (resultCode == 0xFFFF)
			resultCode = lastCode;
		else
			lastCode = resultCode;
	
		// This switch statement checks the received IR code against
		// all of the known codes. Each button press produces a 
		// serial output, and has an effect on the LED output.
		switch (resultCode)
		{
		case BUTTON_POWER:
			Serial.println("Power");
			break;
		case BUTTON_A:
			Serial.println("A");
			break;
		case BUTTON_B:
			Serial.println("B");
			break;
		case BUTTON_C:
			Serial.println("C");
			break;
		case BUTTON_UP:
			Serial.println("Up");
			Counter++;
			break;
		case BUTTON_DOWN:
			Serial.println("Down");
			Counter--;
			break;
		case BUTTON_LEFT:
			Serial.println("Left");
			break;
		case BUTTON_RIGHT:
			Serial.println("Right");
			break;
		case BUTTON_CIRCLE:
			Serial.println("Circle");
			Counter = 0;
			break;
		default:
			Serial.print("Unrecognized code received: 0x");
			Serial.println(results.value, HEX);
			break;        
		}
	}
}

void setup()
{
	//  Give power to the radio receiver
	irrecv.enableIRIn();
	
	pinMode(LightPin_1, OUTPUT);
	pinMode(LightPin_2, OUTPUT);
	pinMode(LightPin_4, OUTPUT);
	pinMode(LightPin_8, OUTPUT);
	
	digitalWrite(LightPin_1, HIGH);
	digitalWrite(LightPin_2, HIGH);
	digitalWrite(LightPin_4, HIGH);
	digitalWrite(LightPin_8, HIGH);
	
	pinMode(ButtonPin_U, INPUT_PULLUP); // connect internal pull-up
	pinMode(ButtonPin_D, INPUT_PULLUP); // connect internal pull-up
	pinMode(ButtonPin_R, INPUT_PULLUP); // connect internal pull-up
	
  	Serial.begin(9600);
}

void loop()
{
	unsigned long currentMillis = millis();
	if (buttonTimer <= currentMillis)
	{
		if (digitalRead(ButtonPin_U) == LOW)
		{
			buttonTimer = currentMillis + 500;
			Counter++;
		}
		if (digitalRead(ButtonPin_D) == LOW)
		{
			buttonTimer = currentMillis + 500;
			Counter--;
		}
		if (digitalRead(ButtonPin_R) == LOW)
		{
			buttonTimer = currentMillis + 500;
			Counter = 0;
		}
	}
	
	CheckForMessages();
	
	//  Light Display based on the counter
	if ((Counter & 1) != 0) digitalWrite(LightPin_1, LOW);
	else digitalWrite(LightPin_1, HIGH);
	
	if ((Counter & 2) != 0) digitalWrite(LightPin_2, LOW);
	else digitalWrite(LightPin_2, HIGH);
	
	if ((Counter & 4) != 0) digitalWrite(LightPin_4, LOW);
	else digitalWrite(LightPin_4, HIGH);
	
	if ((Counter & 8) != 0) digitalWrite(LightPin_8, LOW);
	else digitalWrite(LightPin_8, HIGH);
}
