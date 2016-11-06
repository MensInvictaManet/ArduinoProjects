/* Arduino USB Joystick HID - Racing Wheel Setup 
   // ---------------------------------------
   // Casino Game Maker, Inc.
   // ---------------------------------------
   
   Arduino-big-joystick Firmware Author: Darran Hunt
 */

#define NUM_BUTTONS  40
#define NUM_AXES  8        // 8 axes, X, Y, Z, etc

typedef struct joyReport_t 
{
    int16_t axis[NUM_AXES];
    uint8_t button[(NUM_BUTTONS+7)/8]; // 8 buttons per byte
} joyReport_t;

joyReport_t joyReport;

void setup(void);
void loop(void);
void setButton(joyReport_t *joy, uint8_t button);
void clearButton(joyReport_t *joy, uint8_t button);
void sendJoyReport(joyReport_t *report);

// Maps to X axis.
const int wheel = A0;

// Pedal analog input pins
const int brake = A2; // 
const int gas   = A1; // Y Axis

// Paddle shifter buttons
// Inputs are the pins the buttons are connected to on the Arduino
const int paddleButton1_IN = 8;
const int paddleButton2_IN = 9;

// Outputs are button IDs used to map the input from the board to the OS button
//left paddle
const int paddleButton1_OUT = 5;
//right paddle
const int paddleButton2_OUT = 4;

//Wheel FFB Outputs - pins 2, 3, 4, 5, 6 are used to communicate with the ffb board
const int FFB_Pins = B01111100;
//lights for visual feedback
const int FFB_Right = 12;
const int FFB_Left  = 13;


// For PPPP, Left side is LSB and Right is MSB  
// D is direction, P is power, U is unused
// Pins are inverted so 0000 for P is max voltage and 1111 is 0 voltage
const int FFB_Power_Off       = B00111100;
bool Steering_Reversed = false;

uint8_t PORT_Control          = B00111100;
uint8_t Power                 = 0;

#define SET_POWER(y)  (PORT_Control = POWER[y])

#define DIRECTION_PIN   B01000000
#define ON_LEFT   (PORT_Control |= DIRECTION_PIN)
#define ON_RIGHT  (PORT_Control &= ~DIRECTION_PIN)

const int POWER[] = 
{
  0x3C,
  0x38,
  0x34,
  0x30,
  0x2C,
  0x28,
  0x24,
  0x20,
  0x1C,
  0x18,
  0x14,
  0x10,
  0x0C,
  0x08,
  0x04,
  0x00
};

// Used for tracking the current wheel position and delta
int Wheel_Current = 0;
int Wheel_Previous = 0;
bool Returning_To_Center = false;

long Long_Wheel_Position = 0;

// Used for calibrating the inputs;
int Wheel_Max     = 0;
int Wheel_Min     = 0;
int Wheel_Mid     = 0;
int Wheel_Center  = 0;
int Wheel_Range   = 0;

long Feedback_Range_N = 25;
long Feedback_Range = 32767 / Feedback_Range_N;

long Current_Position = 0;

unsigned long Cur_Time = 0;
unsigned long Prev_Time = 0;
long Interval = 500; // half a second

void setup() 
{
    Serial.begin(115200);
    delay(200);

    for (uint8_t ind=0; ind<8; ind++) 
    {
      joyReport.axis[ind] = ind*1000;
    }
    for (uint8_t ind=0; ind<sizeof(joyReport.button); ind++) 
    {
      joyReport.button[ind] = 0;
    }

    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(paddleButton1_IN, INPUT);
    pinMode(paddleButton2_IN, INPUT);
    pinMode(FFB_Right, OUTPUT);
    pinMode(FFB_Left, OUTPUT);

    //Init the output pins for the wheel and turn off the power
    DDRD = FFB_Pins;
    PORTD = FFB_Power_Off;
    
    // Initialize wheel position
    Wheel_Current = Wheel_Previous = analogRead(wheel);

    Set_Wheel_Endpoints();
    Interval = 100;
}

// Send an HID report to the USB interface
void sendJoyReport(struct joyReport_t *report)
{
    Serial.write((uint8_t *)report, sizeof(joyReport_t));
}

// turn a button on
void setButton(joyReport_t *joy, uint8_t button)
{
    uint8_t index = button/8;
    uint8_t bit = button - 8*index;

    joy->button[index] |= 1 << bit;
}

// turn a button off
void clearButton(joyReport_t *joy, uint8_t button)
{
    uint8_t index = button/8;
    uint8_t bit = button - 8*index;

    joy->button[index] &= ~(1 << bit);
}

bool Is_In_Range(int current_position, int position_to_check, int range)
{
  return current_position < position_to_check + range && current_position > position_to_check - range;
}

void Set_Wheel_Endpoints()
{
    int Temp = 0;
    int Count_1 = 0;
    int Count_2 = 0;
    int Count_3 = 0;

    digitalWrite(FFB_Right, LOW);
    digitalWrite(FFB_Left, LOW);
    
    while(!Wheel_Min || !Wheel_Mid || !Wheel_Max)
    {
      if(!digitalRead(paddleButton1_IN) && !digitalRead(paddleButton2_IN) && !Wheel_Mid)
      {
        Count_1++;
        digitalWrite(FFB_Right, HIGH);
        digitalWrite(FFB_Left, HIGH);
        Count_2 = 0;
        Count_3 = 0;
        if(Count_1 > 100);
        {
          Wheel_Mid = analogRead(wheel);
          digitalWrite(FFB_Right, LOW);
          digitalWrite(FFB_Left, LOW);
        }
      }
      else if(!digitalRead(paddleButton1_IN)&& !Wheel_Min)
      {
        Count_2++;
        digitalWrite(FFB_Left, HIGH);
        digitalWrite(FFB_Right, LOW);
        Count_1 = 0;
        Count_3 = 0;
        if(Count_2 > 100);
        {
          Wheel_Min = analogRead(wheel);
          digitalWrite(FFB_Left, LOW);
        }
      }
      else if(!digitalRead(paddleButton2_IN)&& !Wheel_Max)
      {
        Count_3++;
        digitalWrite(FFB_Right, HIGH);
        digitalWrite(FFB_Left, LOW);
        Count_1 = 0;
        Count_2 = 0;
        if(Count_3 > 100);
        {
          Wheel_Max = analogRead(wheel);
          digitalWrite(FFB_Right, LOW);
        }
      }
    }
    	
    if(Wheel_Max < Wheel_Min)
    {
      Temp = Wheel_Max;
      Wheel_Max = Wheel_Min;
      Wheel_Min = Temp;
      Steering_Reversed = true;
    }
    Wheel_Range = Wheel_Max - Wheel_Min;
    Wheel_Center = Wheel_Mid - Wheel_Min;
}

int Get_Wheel_Position()
{
  int Temp = analogRead(wheel);
  Wheel_Current = Temp;
  if(Temp < Wheel_Min)
  {
    digitalWrite(FFB_Left, LOW);
    Wheel_Min = Temp;
    Wheel_Range = Wheel_Max - Wheel_Min;
    Wheel_Center = Wheel_Mid - Wheel_Min;
  }
  else if(Temp > Wheel_Max)
  {
    digitalWrite(FFB_Right, LOW);
    Wheel_Max = Temp;
    Wheel_Range = Wheel_Max - Wheel_Min;
    Wheel_Center = Wheel_Max - Wheel_Mid;
  }

  Temp -= Wheel_Min;
  
  if(Steering_Reversed)
  {
     Temp = Wheel_Range - Temp;
  }
   
  return Temp;
}

double Get_Normalized_Gas_Position()
{
  double norm = double(analogRead(gas) - 500) / double(675 - 500);
  return norm < 0 ? 0.0 :
         norm > 1 ? 1.0 :
         norm;
}

double Get_Normalized_Brake_Position()
{
  double norm = double(analogRead(brake) - 500) / double(675 - 500);
  return norm < 0 ? 0.0 :
         norm > 1 ? 1.0 :
         norm;
}

/* 
 *  Read each input and send to PC
 *  Also check wheel turn changes and apply appropriate signal to controller
 *  to simulate ffb
 */
void loop() 
{
    //Start Button Control ----------------------------
    
    if(!digitalRead(paddleButton1_IN)) 
      setButton(&joyReport, paddleButton1_OUT);
    else
      clearButton(&joyReport, paddleButton1_OUT);
      
    if(!digitalRead(paddleButton2_IN)) 
      setButton(&joyReport, paddleButton2_OUT);
    else
      clearButton(&joyReport, paddleButton2_OUT);
      
    // End Button Control -----------------------------


    //Start: Wheel Control ----------------------------
    
    Current_Position = Get_Wheel_Position();

    //Set Report Values
     Long_Wheel_Position = (long(Current_Position) - long(Wheel_Center))*long(32767);
     
    if(Current_Position < Wheel_Center)
    {
      Long_Wheel_Position /= (long(Wheel_Center) - long(Wheel_Min));
    }
    else
    {
      Long_Wheel_Position /= (long(Wheel_Max) - long(Wheel_Center));
    }

    Power = abs(Long_Wheel_Position/Feedback_Range);
    Power += 1;
    
    if(Power > 15)
      Power = 15;
    
    SET_POWER(Power);
    
    if(Long_Wheel_Position > 0)
      ON_LEFT;
    else
      ON_RIGHT;

    PORTD = PORT_Control;
    
    //End: Wheel Control -------------------------------
    
    joyReport.axis[0] = int(Long_Wheel_Position);
    joyReport.axis[1] = (Get_Normalized_Gas_Position()  * 32767); // y
    joyReport.axis[2] = (Get_Normalized_Brake_Position()  * 32767); // z?

    //Send Report to PC      
    Cur_Time = millis();
    if (Cur_Time - Prev_Time >= Interval) 
    {
      sendJoyReport(&joyReport);
      Prev_Time = Cur_Time;
    }
    //delay(100);
}
