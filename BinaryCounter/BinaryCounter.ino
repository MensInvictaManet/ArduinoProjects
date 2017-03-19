int seconds = 0; //start the time on 00:00:00

void setup() { //set outputs and inputs
	pinMode(1, OUTPUT);
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
	
	//pinMode(0, INPUT);
	Serial.begin(9600);
}

void loop() {
  static unsigned long lastTime = millis(); // set up a local variable to hold the last time we moved forward one second
  static unsigned long deltaTime = 0;
  static double multiplier = 1.0;
  //multiplier = double(seconds / 5 + 1);

  deltaTime += int((millis() - lastTime) * multiplier);
  lastTime = millis();
  if (deltaTime > 1000)
  {
    seconds += 1;
    deltaTime -= 1000;
  	for (int i = 1; i <= 10; ++i) Serial.println((seconds & (1 << (i - 1))));
  }

	for (int i = 1; i <= 10; ++i)
	{
	  digitalWrite(i, (seconds & (1 << (i - 1))));
	}
	
	//digitalWrite(1,(seconds & 1));
	//digitalWrite(2,(seconds & 2));
	//digitalWrite(3,(seconds & 4));
	//digitalWrite(4,(seconds & 8));
	//digitalWrite(5,(seconds & 16));
	//digitalWrite(6,(seconds & 32));
	
	 //ledstats = digitalRead(0);  // read input value, for setting leds off, but keeping count
	 //if (ledstats == LOW) {
		//for(i=1; i<=13; i++)
		//{
			//digitalWrite(i, LOW);
		//}
	//}
}