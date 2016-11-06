int ledPinR = 3;
int ledPinG = 6;
int ledPinB = 9;

#define DEBUG_OUTPUT 0]

const int commonColorCount = 15;
const int colors[commonColorCount][3] = 
{
	{ 0, 0, 0 },		//  Black
	{ 147, 112, 219 },	//  Medium Purple
	{ 199, 21, 133 },	//  Medium Violet Red
	{ 255, 20, 147 },	//  Deep Pink
	{ 255, 0, 0 },		//  Red
	{ 255, 140, 0 },	//  Dark Orange
	{ 255, 69, 0 },		//  Orange Red
	{ 255, 165, 0 },	//  Orange
	{ 200, 200, 0 },	//  Yellow
	{ 034, 139, 034 },	//  Forest Green
	{ 0, 250, 154 },	//  Medium Spring Green
	{ 032, 178, 170 },  //  Light Sea Green
	{ 0, 100, 0 },  	//  Dark Green
	{ 0, 255, 255 },  	//  Cyan
	{ 070, 130, 180 },  //  Steel Blue
};

const char* colorNames[commonColorCount] = 
{
	"Black",
	"Medium Purple",
	"Medium Violet Red",
	"Deep Pink",
	"Red",
	"Dark Orange",
	"Orange Red",
	"Orange",
	"Yellow",
	"Forest Green",
	"Medium Spring Green",
	"Light Sea Green",
	"Dark Green",
	"Cyan",
	"Steel Blue"
};

struct Color
{
public:
	Color(int r, int g, int b)
	{
		R = r;
		G = g;
		B = b;
	}

	int R;
	int G;
	int B;
	
	bool isZero()
	{
		return (R == 0 && G == 0 && B == 0);
	}
	
	int getValue()
	{
		return R + G + B;
	}
};

Color colorCurrent(0, 0, 0);
Color colorDelta(0, 0, 0);
bool colorDisplay = true;
const int colorDeltaDelay = 15;
const int minColorTime = 5000;
unsigned long nextChangeTime = 0;
bool adding = false;

void printColor()
{
#ifdef DEBUG_OUTPUT
	Serial.print("Current Color(");
	Serial.print(colorCurrent.R);
	Serial.print(", ");
	Serial.print(colorCurrent.G);
	Serial.print(", ");
	Serial.print(colorCurrent.B);
	Serial.print(")\n");
#endif
}

void setup()
{ 
#ifdef DEBUG_OUTPUT
	//  Set up serial communication
	Serial.begin(9600);
#endif
	
	//  Seed the random generator
	randomSeed(analogRead(0));
	
	//  Set the pins to output
	pinMode(ledPinR, OUTPUT);
	pinMode(ledPinG, OUTPUT);
	pinMode(ledPinB, OUTPUT);
	analogWrite(ledPinR, 0);
	analogWrite(ledPinG, 0);
	analogWrite(ledPinB, 0);
} 

/* Note:
When driving LED's using common anode LED AMP's you have to inverse the duty cycle,
i. e. 255 is off and 0 is full power.
*/

void loop()
{
	adding = false;
	if (colorDelta.R > 0) { colorDelta.R -= 1; colorCurrent.R = min(255, colorCurrent.R + 1); delay(colorDeltaDelay); analogWrite(ledPinR, 255 - colorCurrent.R); adding = true; }
	if (colorDelta.G > 0) { colorDelta.G -= 1; colorCurrent.G = min(255, colorCurrent.G + 1); delay(colorDeltaDelay); analogWrite(ledPinG, 255 - colorCurrent.G); adding = true; }
	if (colorDelta.B > 0) { colorDelta.B -= 1; colorCurrent.B = min(255, colorCurrent.B + 1); delay(colorDeltaDelay); analogWrite(ledPinB, 255 - colorCurrent.B); adding = true; }
	if (adding == false)
	{
		if (colorDelta.R < 0) { colorDelta.R += 1; colorCurrent.R = max(0,   colorCurrent.R - 1); delay(colorDeltaDelay); analogWrite(ledPinR, 255 - colorCurrent.R); }
		if (colorDelta.G < 0) { colorDelta.G += 1; colorCurrent.G = max(0,   colorCurrent.G - 1); delay(colorDeltaDelay); analogWrite(ledPinG, 255 - colorCurrent.G); }
		if (colorDelta.B < 0) { colorDelta.B += 1; colorCurrent.B = max(0,   colorCurrent.B - 1); delay(colorDeltaDelay); analogWrite(ledPinB, 255 - colorCurrent.B); }
	}
	
		
	if (colorDelta.isZero() && nextChangeTime < millis())
	{
		printColor();
		int newColorIndex = 0;
		do
		{
			newColorIndex = 1 + random(commonColorCount - 1);
			colorDelta.R = colors[newColorIndex][0] - colorCurrent.R;
			colorDelta.G = colors[newColorIndex][1] - colorCurrent.G;
			colorDelta.B = colors[newColorIndex][2] - colorCurrent.B;
		} while (colorDelta.isZero());
		Serial.print(colorNames[newColorIndex]);
		
		nextChangeTime = millis() + minColorTime;
		
#ifdef DEBUG_OUTPUT
		Serial.print(" - COLOR DELTA: Color(");
		Serial.print(colorDelta.R);
		Serial.print(", ");
		Serial.print(colorDelta.G);
		Serial.print(", ");
		Serial.print(colorDelta.B);
		Serial.print(")\n");
#endif
	}
}