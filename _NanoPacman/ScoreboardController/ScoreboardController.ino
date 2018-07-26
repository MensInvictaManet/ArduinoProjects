#include <Wire.h>

int digitPinList[5][8] = 
{
  { 2, 3, 4, 5, 34, 35, 36, 37 },
  { 6, 7, 8, 9, 38, 39, 40, 41 },
  { 10, 11, 12, 14, 42, 43, 44, 45 },
  { 26, 27, 31, 29, 46, 47, 48, 49 },
  { 30, 28, 32, 33, 50, 51, 52, 53 }
};

int pinsIndicesStartToFinish[8] = { 0, 4, 1, 3, 2, 5, 6, 7 };

void ClearDigit(int digitIndex)
{
  SetPins(digitIndex, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW);
}

void DisplayNumberOnDigit(int digitIndex, long number)
{
  if (number < 0) return;
  if (number > 9) return;

  switch (number)
  {
    case 0:   SetPins(digitIndex, LOW, HIGH, HIGH, HIGH, LOW, HIGH, HIGH, HIGH);    break;
    case 1:   SetPins(digitIndex, LOW, HIGH, LOW, LOW, LOW, HIGH, LOW, LOW);        break;
    case 2:   SetPins(digitIndex, LOW, HIGH, HIGH, LOW, HIGH, LOW, HIGH, HIGH);     break;
    case 3:   SetPins(digitIndex, LOW, HIGH, HIGH, LOW, HIGH, HIGH, HIGH, LOW);     break;
    case 4:   SetPins(digitIndex, LOW, HIGH, LOW, HIGH, HIGH, HIGH, LOW, LOW);      break;
    case 5:   SetPins(digitIndex, LOW, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, LOW);     break;
    case 6:   SetPins(digitIndex, LOW, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH);    break;
    case 7:   SetPins(digitIndex, LOW, HIGH, HIGH, LOW, LOW, HIGH, LOW, LOW);       break;
    case 8:   SetPins(digitIndex, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH);   break;
    case 9:   SetPins(digitIndex, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW);    break;
  }
}

void SetPins(int digitIndex, int pinG, int pin1, int pin2, int pin3, int pin4, int pin5, int pin6, int pin7)
{
  if (pinG == -1) pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[0]], INPUT);
  else pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[0]], OUTPUT);    digitalWrite(digitPinList[digitIndex][pinsIndicesStartToFinish[0]], pinG);
  
  if (pin1 == -1) pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[1]], INPUT);
  else pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[1]], OUTPUT);    digitalWrite(digitPinList[digitIndex][pinsIndicesStartToFinish[1]], pin1);
  
  if (pin2 == -1) pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[2]], INPUT);
  pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[2]], OUTPUT);    digitalWrite(digitPinList[digitIndex][pinsIndicesStartToFinish[2]], pin2);
  
  if (pin3 == -1) pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[3]], INPUT);
  pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[3]], OUTPUT);    digitalWrite(digitPinList[digitIndex][pinsIndicesStartToFinish[3]], pin3);
  
  if (pin4 == -1) pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[4]], INPUT);
  pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[4]], OUTPUT);    digitalWrite(digitPinList[digitIndex][pinsIndicesStartToFinish[4]], pin4);
  
  if (pin5 == -1) pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[5]], INPUT);
  pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[5]], OUTPUT);    digitalWrite(digitPinList[digitIndex][pinsIndicesStartToFinish[5]], pin5);
  
  if (pin6 == -1) pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[6]], INPUT);
  pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[6]], OUTPUT);    digitalWrite(digitPinList[digitIndex][pinsIndicesStartToFinish[6]], pin6);
  
  if (pin7 == -1) pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[7]], INPUT);
  pinMode(digitPinList[digitIndex][pinsIndicesStartToFinish[7]], OUTPUT);    digitalWrite(digitPinList[digitIndex][pinsIndicesStartToFinish[7]], pin7);
}

/*
void TestPins(int pinIndex1, int pinIndex2)
{
  for (int i = 0; i < 8; ++i) pinMode(digitPinList[i], INPUT);
  pinMode(digitPinList[pinIndex1], OUTPUT);
  pinMode(digitPinList[pinIndex2], OUTPUT);
  digitalWrite(digitPinList[pinIndex1], LOW);
  digitalWrite(digitPinList[pinIndex2], HIGH);
}
*/

void TestDigits()
{
  for (int i = 0; i < 10; ++i)
  {
    DisplayNumberOnDigit(0, (i + 0) % 10);
    DisplayNumberOnDigit(1, (i + 1) % 10);
    DisplayNumberOnDigit(2, (i + 2) % 10);
    DisplayNumberOnDigit(3, (i + 3) % 10);
    DisplayNumberOnDigit(4, (i + 4) % 10);
    delay(1000);
  }

  DisplayNumber(98765);
  delay(1000);
  DisplayNumber(12345);
  delay(1000);
  DisplayNumber(52781);
  delay(1000);
  DisplayNumber(123);
  delay(1000);
  DisplayNumber(501);
  delay(1000);
  DisplayNumber(76);
  delay(1000);
  DisplayNumber(0);
  delay(1000);
}

void DisplayNumber(long number)
{
  for (int i = 0; i < 5; ++i) ClearDigit(i);
  
  if (number < 0)     return;
  if (number > 99999) return;
  
  long digit1 = number % 10;
  DisplayNumberOnDigit(4, digit1);
  number -= digit1;
  number /= 10;

  if (number == 0) return;
  long digit2 = number % 10;
  DisplayNumberOnDigit(3, digit2);
  number -= digit2;
  number /= 10;

  if (number == 0) return;
  long digit3 = number % 10;
  DisplayNumberOnDigit(2, digit3);
  number -= digit3;
  number /= 10;

  if (number == 0) return;
  long digit4 = number % 10;
  DisplayNumberOnDigit(1, digit4);
  number -= digit4;
  number /= 10;

  if (number == 0) return;
  long digit5 = number % 10;
  DisplayNumberOnDigit(0, digit5);
}

void receiveEvent(int bytes)
{
  long receivedScore = 0;
  byte scoreByte = 0;
  if(Wire.available() != 0)
  {
    for (int i = 0; i < bytes; i++)
    {
      scoreByte = Wire.read();
      receivedScore |= (scoreByte << i);
    }
    
    Serial.print("Received: ");
    Serial.print(receivedScore, DEC);
    Serial.print("\n");
    DisplayNumber(receivedScore);
  }
}

void setup() {
  Wire.begin(0x60);
  Wire.onReceive(receiveEvent);
  
  Serial.begin(9600);
}

void loop() {
  return;
  SetPins(0, LOW, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH);
  delay(1500);
  SetPins(0, LOW, HIGH, LOW, LOW, LOW, LOW, LOW, LOW);
  delay(1500);
  SetPins(0, LOW, LOW, HIGH, LOW, LOW, LOW, LOW, LOW);
  delay(1500);
  SetPins(0, LOW, LOW, LOW, HIGH, LOW, LOW, LOW, LOW);
  delay(1500);
  SetPins(0, LOW, LOW, LOW, LOW, HIGH, LOW, LOW, LOW);
  delay(1500);
  SetPins(0, LOW, LOW, LOW, LOW, LOW, HIGH, LOW, LOW);
  delay(1500);
  SetPins(0, LOW, LOW, LOW, LOW, LOW, LOW, HIGH, LOW);
  delay(1500);
  SetPins(0, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH);
  delay(1500);
  return;
  
  ClearDigit(4);
  DisplayNumberOnDigit(0, 8);
  delay(500);
  ClearDigit(0);
  DisplayNumberOnDigit(1, 8);
  delay(500);
  ClearDigit(1);
  DisplayNumberOnDigit(2, 8);
  delay(500);
  ClearDigit(2);
  DisplayNumberOnDigit(3, 8);
  delay(500);
  ClearDigit(3);
  DisplayNumberOnDigit(4, 8);
  delay(500);
  //TestDigits();
}
