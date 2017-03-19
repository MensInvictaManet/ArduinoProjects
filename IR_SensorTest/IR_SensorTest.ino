#include <IRremote.h>

#define IR_RECEIVE_PIN  10   //  The IR Receiver data pin

#define DEBUG_OUTPUT    true
#define MESSAGE_DELAY	10

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
			break;
		case BUTTON_DOWN:
			Serial.println("Down");
			break;
		case BUTTON_LEFT:
			Serial.println("Left");
			break;
		case BUTTON_RIGHT:
			Serial.println("Right");
			break;
		case BUTTON_CIRCLE:
			Serial.println("Circle");
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
	
	//  Set up the serial connection for debugging
	Serial.begin(9600);
#if DEBUG_OUTPUT
	Serial.write("Program started\n");
#endif
}

void loop()
{
	CheckForMessages();
}