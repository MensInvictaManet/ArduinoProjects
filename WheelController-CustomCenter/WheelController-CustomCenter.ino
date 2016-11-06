/* Arduino USB Joystick HID - Racing Wheel Setup 
   // ---------------------------------------
   // Casino Game Maker, Inc.
   // ---------------------------------------
   
   Arduino-big-joystick Firmware Author: Darran Hunt
 */
#include "RunningAverage.h"

#define NUM_BUTTONS  40
#define NUM_AXES  8        // 8 axes, X, Y, Z, etc

typedef struct joyReport_t {
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

// Outputs are button ids used to map the input from the board to the OS button
//left paddle
const int paddleButton1_OUT = 5;
//right paddle
const int paddleButton2_OUT = 4;

//Wheel FFB Outputs - pins 2, 3, 4, 5, 6 are used to communicate with the ffb board
const int FFB_Pins = B01111100;
//lights for visual feedback
int FFB_Left	= 12;
int FFB_Right	= 13;

#define RANGE_PIE_PIECES 35

// For PPPP, Left side is LSB and Right is MSB  
// D is direction, P is power, U is unused
// Pins are inverted so 0000 for P is max voltage and 1111 is 0 voltage
//                              BUDPPPPUU 
int FFB_Turn_Left_Low_Power   = B01110000;
int FFB_Turn_Left_Med_Power   = B01100000;
int FFB_Turn_Left_High_Power  = B01000100;
int FFB_Turn_Right_Low_Power  = B00110000;
int FFB_Turn_Right_Med_Power  = B00100000;
int FFB_Turn_Right_High_Power = B00000100;
const int FFB_Power_Off       = B00111100;

uint8_t PORT_Control          = FFB_Power_Off;

#define DIRECTION_PIN   B01000000
#define ON_LEFT   { (PORT_Control |= DIRECTION_PIN); digitalWrite(FFB_Left, LOW); digitalWrite(FFB_Right, HIGH); }
#define ON_RIGHT  { (PORT_Control &= ~DIRECTION_PIN); digitalWrite(FFB_Right, LOW); digitalWrite(FFB_Left, HIGH); }

uint8_t Power                 = 0;
#define SET_POWER(y)  (PORT_Control = POWER[y])

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

// Used for tracking the current wheel position
int Wheel_Current = 0;
int Wheel_Previous = 0;

enum ReturnToCenter { NOT_RETURNING, RETURNING_LEFT, RETURNING_RIGHT };
ReturnToCenter CurrentReturnValue = NOT_RETURNING;

// Used for calibrating the inputs;
int Wheel_Max   = 0;
int Wheel_Min   = 0;
int Wheel_Center = 0;

double Current_Position = 0.0;

unsigned long Cur_Time = 0;
unsigned long Prev_Time = 0;
long Interval = 500; // half a second

const double TrueCenterDelta = 0.01;
const double OuterCenterDelta = 0.02;
const double LeftAdjust0 = 0.5 - TrueCenterDelta;
const double RightAdjust0 = 0.5 + TrueCenterDelta;
const double LeftAdjust1 = 0.5 - OuterCenterDelta;
const double RightAdjust1 = 0.5 + OuterCenterDelta;
const double LeftAdjust2 = 0.4;
const double RightAdjust2 = 0.6;
const double LeftAdjust3 = 0.3;
const double RightAdjust3 = 0.7;

const double DeadZoneDelta = 0.1;

void Swap(int& val1, int& val2)
{
	val1 ^= val2;
	val2 ^= val1;
	val1 ^= val2;
}

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
    pinMode(FFB_Left, OUTPUT);
    pinMode(FFB_Right, OUTPUT);
	digitalWrite(FFB_Left, HIGH);
	digitalWrite(FFB_Right, HIGH);

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

void TurnAllTheWayLeft()
{
	// Turn Left all the way and set the minimum value
	digitalWrite(FFB_Left, LOW);
	PORTD = FFB_Turn_Left_Med_Power;
	bool turning = true;
	Prev_Time = millis();
	
	//check and see if the wheel is still turning. if it isn't switch directions
	while(turning)
	{ 
		Cur_Time = millis();
		if (Cur_Time - Prev_Time >= Interval)
		{
			Wheel_Current = analogRead(wheel);
			if(Is_In_Range(Wheel_Current, Wheel_Previous, 2))
			{
				Wheel_Min = Wheel_Current;
				turning = false;
				PORTD = FFB_Power_Off;
				digitalWrite(FFB_Left, HIGH);
			}
			else
			{
				Wheel_Current = Wheel_Previous = analogRead(wheel);
				Prev_Time = Cur_Time;
			}
		}
	}
}

void TurnAllTheWayRight()
{
	//  Turn Right all the way and set the maximum value
	digitalWrite(FFB_Right, LOW);
	PORTD = FFB_Turn_Right_Med_Power;
	bool turning = true;
	Prev_Time = millis();
	
	while(turning)
	{ 
		Cur_Time = millis();
		if (Cur_Time - Prev_Time >= Interval)
		{
			Wheel_Current = analogRead(wheel);
			if(Is_In_Range(Wheel_Current, Wheel_Previous, 2))
			{
				Wheel_Max = Wheel_Current;
				turning = false;
				PORTD = FFB_Power_Off;
				digitalWrite(FFB_Right, HIGH);
			}
			else
			{
				Wheel_Current = Wheel_Previous = analogRead(wheel);
				
				Prev_Time = Cur_Time;
			}
		}
	}
}

void Set_Wheel_Endpoints()
{
	delay(2000);
	
	//  Find the minimum and maximum turning range
	TurnAllTheWayRight();
	TurnAllTheWayLeft();
	
	//  If we're using reversed hardware, just swap the values for all turning related features
	if (Wheel_Max < Wheel_Min)
	{
		Swap(FFB_Turn_Right_Low_Power, FFB_Turn_Left_Low_Power);
		Swap(FFB_Turn_Right_Med_Power, FFB_Turn_Left_Med_Power);
		Swap(FFB_Turn_Right_High_Power, FFB_Turn_Left_High_Power);
		Swap(FFB_Left, FFB_Right);
		Swap(Wheel_Max, Wheel_Min);
	}
	
	PORTD = FFB_Power_Off;
	
	//  NOTE: After the min and max turn, it waits two seconds and then determines the middle by where the user centers it, then corrects the min and max
	delay(2000);
	Wheel_Center = Wheel_Current = analogRead(wheel);
	if (Wheel_Center - Wheel_Min > Wheel_Max - Wheel_Center) 	{ Wheel_Min = (Wheel_Center - (Wheel_Max - Wheel_Center)); }
	else 														{ Wheel_Max = (Wheel_Center + (Wheel_Center - Wheel_Min)); }

	//Wheel_Center = (Wheel_Max - Wheel_Min) / 2;
}

double Get_Normalized_Wheel_Position()
{
	int wheelPosition = min(max(Get_Wheel_Position(), Wheel_Min), Wheel_Max);
	double norm = double(wheelPosition - Wheel_Min) / double(Wheel_Max - Wheel_Min);
	
	//  Prevents the normal from going below 0 or over 1 if the endpoints were a little off.
	if (norm < 0) norm = 0.0;
	if (norm > 1) norm = 1.0;
	
	return norm;
}

int Get_Wheel_Position()
{
	Wheel_Current = float(analogRead(wheel));
	return Wheel_Current;
}

double Get_Normalized_Gas_Position()
{
	double norm = double(analogRead(gas) - 500) / double(675 - 500);
	if (norm < 0.0) norm = 0.0;
	if (norm > 1.0) norm = 1.0;
	return norm;
}

double Get_Normalized_Brake_Position()
{
	double norm = double(analogRead(brake) - 500) / double(675 - 500);
	if (norm < 0.0) norm = 0.0;
	if (norm > 1.0) norm = 1.0;
	return norm;
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


    //Start: Wheel Control -----------
    Current_Position = Get_Normalized_Wheel_Position();
    Power = abs(int((Current_Position - 0.5) / 0.5 * RANGE_PIE_PIECES));
    if (Power > 15) Power = 15;
    SET_POWER(Power);
    
    if(Current_Position < 0.5)	ON_LEFT
    else						ON_RIGHT
    
    PORTD = PORT_Control;
  
    /*
    //  Push toward the center if we're far enough away from the center point
    if (Current_Position > RightAdjust1) //  Right of center
    {
        CurrentReturnValue = RETURNING_LEFT;
        PORTD = (Current_Position > RightAdjust2) ? ((Current_Position > RightAdjust3) ? FFB_Turn_Left_High_Power : FFB_Turn_Left_Med_Power) : FFB_Turn_Left_Low_Power;
        digitalWrite(FFB_Left, LOW);
    }
    else if (Current_Position < LeftAdjust1) // Left of center
    {
        CurrentReturnValue = RETURNING_RIGHT;
        PORTD = (Current_Position < LeftAdjust2) ? ((Current_Position < LeftAdjust3) ? FFB_Turn_Right_High_Power : FFB_Turn_Right_Med_Power) : FFB_Turn_Right_Low_Power;
        digitalWrite(FFB_Right, LOW);
    }
    else if (Current_Position > LeftAdjust0 && Current_Position < RightAdjust0)
    {
       CurrentReturnValue = NOT_RETURNING;
       PORTD = FFB_Power_Off;
       digitalWrite(FFB_Left, HIGH);
       digitalWrite(FFB_Right, HIGH);
    }
    */

    // .0015 with low power works if serial printing
    // 
     
    //End: Wheel Control -------------------------------
    
    
    //Set Report Values
    joyReport.axis[0] = (Current_Position * 32767); // x
    joyReport.axis[1] = (Get_Normalized_Gas_Position()  * 32767); // y
    joyReport.axis[2] = (Get_Normalized_Brake_Position()  * 32767); // z?

    //Send Report to PC      
    Cur_Time = millis();
    if (Cur_Time - Prev_Time >= Interval) {
      sendJoyReport(&joyReport);
      Prev_Time = Cur_Time;
    }
}