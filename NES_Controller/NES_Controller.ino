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

#define CONTROLLER_COUNT        2

const int NES_DataPins[]        = {2, 6};
const int NES_LatchPins[]       = {3, 7};
const int NES_ClockPins[]       = {4, 8};

#define NES_DATA_PIN            2
#define NES_LATCH_PIN           3
#define NES_CLOCK_PIN           4

int NES_Register[CONTROLLER_COUNT] = {}; //  We will use this to hold current button states

void NES_Controller_Test()
{
  for (int i = 0; i < CONTROLLER_COUNT; ++i) {
    if (bitRead(NES_Register[i], LEFT_BUTTON) == 0)
    {
      Serial.print(i);
      Serial.println(" LEFT");
    }
    if (bitRead(NES_Register[i], RIGHT_BUTTON) == 0)
    {
      Serial.print(i);
      Serial.println(" RIGHT");
    }
    if (bitRead(NES_Register[i], UP_BUTTON) == 0)
    {
      Serial.print(i);
      Serial.println(" UP");
    }
    if (bitRead(NES_Register[i], DOWN_BUTTON) == 0)
    {
      Serial.print(i);
      Serial.println(" DOWN");
    }
    if (bitRead(NES_Register[i], A_BUTTON) == 0)
    {
      Serial.print(i);
      Serial.println(" A");
    }
    if (bitRead(NES_Register[i], B_BUTTON) == 0)
    {
      Serial.print(i);
      Serial.println(" B");
    }
    if (bitRead(NES_Register[i], START_BUTTON) == 0)
    {
      Serial.print(i);
      Serial.println(" START");
    }
    if (bitRead(NES_Register[i], SELECT_BUTTON) == 0)
    {
      Serial.print(i);
      Serial.println(" SELECT");
    }
  }
}

byte readNesController() 
{  

  for (byte i = 0; i < CONTROLLER_COUNT; ++i) {
    // Pre-load a variable with all 1's which assumes all buttons are not pressed. But while we cycle through the
    // bits, if we detect a LOW, which is a 0, we clear that bit. In the end, we find all the buttons states at once.
    NES_Register[i] = 255;
  
    //  We pulse the NES_LATCH_PIN so that the register grabs what it sees on its parallel data pins.
    digitalWrite(NES_LatchPins[i], HIGH);
    digitalWrite(NES_LatchPins[i], LOW);
  
    //  We check each button in order, saving off the bit if that particular pulse results in a LOW
    //  value on the Data Pin, then jumping to the next check by puling the Clock Pin
    for (byte j = 0; j < 8; ++j) {
      if (digitalRead(NES_DataPins[i]) == LOW) { bitClear(NES_Register[i], j); }
      digitalWrite(NES_ClockPins[i], HIGH);
      digitalWrite(NES_ClockPins[i], LOW);
    }
  }
}

void SetupControllers() {
  for (byte i = 0; i < CONTROLLER_COUNT; ++i) { NES_Register[i] = 0; }

  for (byte i = 0; i < CONTROLLER_COUNT; ++i) {
    pinMode(NES_DataPins[i], INPUT);
    
    // Set appropriate pins to outputs
    pinMode(NES_ClockPins[i], OUTPUT);
    pinMode(NES_LatchPins[i], OUTPUT);
    
    // Set initial states
    digitalWrite(NES_ClockPins[i], LOW);
    digitalWrite(NES_LatchPins[i], LOW);
  }
}

void setup() {
  SetupControllers();

  Serial.begin(9600);
  while (!Serial) { ; }
  Serial.println("Program START!");
}

void loop()
{
  readNesController();
  NES_Controller_Test();
}
