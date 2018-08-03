#define   LED_REACTION    0

#if LED_REACTION
const int LED_U = 3;
const int LED_D = 4;
const int LED_L = 5;
const int LED_R = 6;
const int LED_F = 7;
#endif

const int Button_U = 2;
const int Button_D = 3;
const int Button_L = 4;
const int Button_R = 5;
const int Button_F = 6;
const int Button_G = 7;

void setup(void)
{
    // Open serial communications and wait for port to open:
    Serial.begin(115200);
    while (!Serial) { ; }
    
#if LED_REACTION
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
#endif
  	
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
    pinMode(Button_G, OUTPUT);
    digitalWrite(Button_G, LOW);  //  Set the controller ground pin
}
void loop(void)
{
#if LED_REACTION
    digitalWrite(LED_U, digitalRead(Button_U));
    digitalWrite(LED_D, digitalRead(Button_D));
    digitalWrite(LED_L, digitalRead(Button_L));
    digitalWrite(LED_R, digitalRead(Button_R));
    digitalWrite(LED_F, digitalRead(Button_F));
#else
    Serial.print(digitalRead(Button_U) ? "U" : "X");
    Serial.print(digitalRead(Button_D) ? "D" : "X");
    Serial.print(digitalRead(Button_L) ? "L" : "X");
    Serial.print(digitalRead(Button_R) ? "R" : "X");
    Serial.print(digitalRead(Button_F) ? "F" : "X");
    Serial.println("");
#endif
  
  	//Wait for 100 ms, then go back to the beginning of 'loop' and repeat.
  	delay(100);
}
