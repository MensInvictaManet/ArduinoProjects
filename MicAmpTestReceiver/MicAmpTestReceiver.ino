// Chapter 7 - Communications
// I2C Slave
// By Cornel Amariei for Packt Publishing

// Include the required Wire library for I2C
#include <Wire.h>

int x = 0;

void setup() {
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
  
  Serial.begin(9600);
}

void receiveEvent(int bytes) {
  x = Wire.read();    // read one character from the I2C
}

void loop() {
  Serial.println(x);
}

