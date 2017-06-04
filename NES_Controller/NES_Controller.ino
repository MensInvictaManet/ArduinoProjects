/*
================================================================================
   Notes
================================================================================
- The NES controller contains one 8-bit 4021 shift register inside. 

- This register takes parallel inputs and converts them into a serial output.

- This code first latches the data and then shifts in the first bit on the data line. 
  Then it clocks and shifts in on the data line until all bits are received.
  
- What is debugged are the button states of the NES controller.

- A logical "1" means the button is not pressed. A logical "0" means the button is
  pressed.
  
- This code shifts the first bit of data into the LSB.

- The order of shifting for the buttons is shown in the table below:

        Bit# | Button   
        --------------
          0  |   A  
        --------------
          1  |   B  
        --------------
          2  | Select   
        --------------
          3  | Start  
        --------------
          4  |   Up  
        --------------
          5  |  Down  
        --------------
          6  |  Left   
        --------------
          7  | Right   
        --------------
        
- The NES controller pinout is shown below (looking into controllers
  connector end):
    __________
   /      |
  /       O 1 |   1 - Ground
  |           |   2 - Clock
  | 7 O   O 2 |   3 - Latch
  |           |   4 - Data Out
  | 6 O   O 3 |   5 - No Connection
  |           |   6 - No Connection
  | 5 O   O 4 |   7 - +5V
  |___________|
*/

#define A_BUTTON                0
#define B_BUTTON                1
#define SELECT_BUTTON           2
#define START_BUTTON            3
#define UP_BUTTON               4
#define DOWN_BUTTON             5
#define LEFT_BUTTON             6
#define RIGHT_BUTTON            7

#define NES_CLOCK_PIN           4
#define NES_LATCH_PIN           5
#define NES_DATA_PIN            6

byte NESRegister = 0; //  We will use this to hold current button states

void NES_Controller_Test()
{
    //  Move left or right if the controller is polled with LEFT or RIGHT input
    if (bitRead(NESRegister, LEFT_BUTTON) == 0)
    {
      Serial.println("LEFT");
    }
    if (bitRead(NESRegister, RIGHT_BUTTON) == 0)
    {
      Serial.println("RIGHT");
    }
    if (bitRead(NESRegister, UP_BUTTON) == 0)
    {
      Serial.println("UP");
    }
    if (bitRead(NESRegister, DOWN_BUTTON) == 0)
    {
      Serial.println("DOWN");
    }
    if (bitRead(NESRegister, A_BUTTON) == 0)
    {
      Serial.println("A");
    }
    if (bitRead(NESRegister, B_BUTTON) == 0)
    {
      Serial.println("B");
    }
    if (bitRead(NESRegister, START_BUTTON) == 0)
    {
      Serial.println("START");
    }
    if (bitRead(NESRegister, SELECT_BUTTON) == 0)
    {
      Serial.println("SELECT");
    }
}

byte readNesController() 
{  
  // Pre-load a variable with all 1's which assumes all buttons are not
  // pressed. But while we cycle through the bits, if we detect a LOW, which is
  // a 0, we clear that bit. In the end, we find all the buttons states at once.
  NESRegister = 255;
    
  // Quickly pulse the NES_LATCH_PIN pin so that the register grab what it see on
  // its parallel data pins.
  digitalWrite(NES_LATCH_PIN, HIGH);
  digitalWrite(NES_LATCH_PIN, LOW);
 
  // Upon latching, the first bit is available to look at, which is the state
  // of the A button. We see if it is low, and if it is, we clear out variable's
  // first bit to indicate this is so.
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, A_BUTTON);
    
  // Clock the next bit which is the B button and determine its state just like
  // we did above.
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, B_BUTTON);
  
  // Now do this for the rest of them!
  
  // Select button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, SELECT_BUTTON);

  // Start button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, START_BUTTON);

  // Up button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, UP_BUTTON);
    
  // Down button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, DOWN_BUTTON);

  // Left button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, LEFT_BUTTON);  
    
  // Right button
  digitalWrite(NES_CLOCK_PIN, HIGH);
  digitalWrite(NES_CLOCK_PIN, LOW);
  if (digitalRead(NES_DATA_PIN) == LOW)
    bitClear(NESRegister, RIGHT_BUTTON);
}

void setup()
{
  //  NES CONTROLLER CODE
  // Set appropriate pins to inputs
  pinMode(NES_DATA_PIN, INPUT);
  
  // Set appropriate pins to outputs
  pinMode(NES_CLOCK_PIN, OUTPUT);
  pinMode(NES_LATCH_PIN, OUTPUT);
  
  // Set initial states
  digitalWrite(NES_CLOCK_PIN, LOW);
  digitalWrite(NES_LATCH_PIN, LOW);
  //  NES CONTROLLER CODE

  Serial.begin(9600);
  while (!Serial) { ; }
  Serial.println("Program START!");
}

void loop()
{
  readNesController();
  NES_Controller_Test();
}
