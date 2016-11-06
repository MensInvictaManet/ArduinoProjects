//simple Tx on pin D12
//Written By : Mohannad Rawashdeh
// 3:00pm , 13/6/2013
//http://www.genotronex.com/
//..................................
#include <VirtualWire.h>

#define VW_DATA_PIN     12
#define DISPLAY_PIN     13
#define MESSAGE_LENGTH  100
#define DEBUG_OUTPUT    false

int lastData = -1;

enum MessageID
{
  MI_TOGGLE_POWER          = 1,
  MI_COLOR_RED             = 2,
  MI_COLOR_GREEN           = 3,
  MI_COLOR_BLUE            = 4,
  MI_COLOR_PURPLE          = 5,
  MI_COLOR_ORANGE          = 6,
  MI_COLOR_CYCLE           = 7,
  MI_PTRN_BASIC            = 8,
  MI_PTRN_RAINBOW          = 9
};

const int MESSAGE_PIN_COUNT = 9;
const int MESSAGE_PIN_LOWER = 2;
const int messagePins[MESSAGE_PIN_COUNT][2] =
{
  { 5,   MI_TOGGLE_POWER },
  { 4,   MI_COLOR_RED },
  { 3,   MI_COLOR_GREEN },
  { 2,   MI_COLOR_BLUE },
  { 8,   MI_COLOR_PURPLE },
  { 7,   MI_COLOR_ORANGE },
  { 6,   MI_COLOR_CYCLE },
  { 10,  MI_PTRN_BASIC },
  { 9,   MI_PTRN_RAINBOW }
};

void SendData(uint8_t message, int light)
{
  digitalWrite(DISPLAY_PIN, HIGH);
  const long int startTime = millis();
  while (millis() < startTime + MESSAGE_LENGTH)
  {
#if DEBUG_OUTPUT
    Serial.print("Sending message: ");
    Serial.print(message);
    Serial.print("\n");
#endif

    vw_send(&message, 1);
    vw_wait_tx(); // Wait until the whole message is gone
  }
  digitalWrite(DISPLAY_PIN, LOW);
}


void setup()
{
  pinMode(DISPLAY_PIN, OUTPUT);
  pinMode(VW_DATA_PIN, OUTPUT);
  
  for (int i = 0; i < MESSAGE_PIN_COUNT; ++i) pinMode(messagePins[i][0], INPUT_PULLUP);
  
  vw_set_ptt_inverted(true); //
  vw_set_tx_pin(VW_DATA_PIN);
  vw_setup(400);// speed of data transfer Kbps
  
#if DEBUG_OUTPUT
  Serial.begin(9600);
  Serial.print("Program Started...\n");
#endif
}

void loop()
{
  for (int i = 0; i < MESSAGE_PIN_COUNT; ++i)
  {
    if (digitalRead(messagePins[i][0]) == 0)
    {
#if DEBUG_OUTPUT
      Serial.print("Checking pin ");
      Serial.print(messagePins[i][0]);
      Serial.print(": ");
      Serial.print(i);
      Serial.write("\n");
#endif

      SendData(messagePins[i][1], true);
    }
  }
}
