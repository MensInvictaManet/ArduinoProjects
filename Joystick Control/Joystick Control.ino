const int LED_U = 3;
const int LED_D = 4;
const int LED_L = 5;
const int LED_R = 6;
const int LED_F = 7;
const int Button_U = 8;
const int Button_D = 9;
const int Button_L = 10;
const int Button_R = 11;
const int Button_F = 12;

void setup(void)
{
	//  Enable the output LED pins
	pinMode(LED_U, OUTPUT);
	digitalWrite(LED_U, HIGH);
	pinMode(LED_D, OUTPUT);
	digitalWrite(LED_D, HIGH);
	pinMode(LED_L, OUTPUT);
	digitalWrite(LED_L, HIGH);
	pinMode(LED_R, OUTPUT);
	digitalWrite(LED_R, HIGH);
	pinMode(LED_F, OUTPUT);
	digitalWrite(LED_F, HIGH);
	
	//  Enable the joystick input pins
	pinMode(Button_U, INPUT);
	digitalWrite(Button_U, HIGH); //Enable the pull-up resistor
	pinMode(Button_D, INPUT);
	digitalWrite(Button_D, HIGH); //Enable the pull-up resistor
	pinMode(Button_L, INPUT);
	digitalWrite(Button_L, HIGH); //Enable the pull-up resistor
	pinMode(Button_R, INPUT);
	digitalWrite(Button_R, HIGH); //Enable the pull-up resistor
	pinMode(Button_F, INPUT);
	digitalWrite(Button_F, HIGH); //Enable the pull-up resistor
}
void loop(void)
{
	digitalWrite(LED_U, digitalRead(Button_U));
	digitalWrite(LED_D, digitalRead(Button_D));
	digitalWrite(LED_L, digitalRead(Button_L));
	digitalWrite(LED_R, digitalRead(Button_R));
	digitalWrite(LED_F, digitalRead(Button_F));

	//Wait for 100 ms, then go back to the beginning of 'loop' and repeat.
	delay(100);
}