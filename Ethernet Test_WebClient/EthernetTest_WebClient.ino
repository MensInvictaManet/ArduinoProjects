// Demo using DHCP and DNS to perform a web client request.
// 2011-06-08 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>
#include <LedControl.h>
#include <MemoryFree.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

#define bufferSize	600
#define openerSize	302

byte Ethernet::buffer[bufferSize];
String jsonBuffer = "";
int stringPos = 0;
bool finished = false;
static uint32_t timer = 0;

#define FOLLOWER_COUNT_DATA_INDEX 11
int dataIndex = 0;

LedControl sevenSeg = LedControl(2,3,4,1);
const unsigned long sevenSegDelay = 250;

char website[] PROGMEM = "api.twittercounter.com";
//char arguments[] = "apikey=e9335031a759f251ee9b4e2e6634e1c5&twitter_id=3412359690"; // PrismBlade
//char arguments[] = "apikey=17870925e17e1a15635e24ceb93d452a&twitter_id=2804837748"; // Vamplettes
char arguments[] = "apikey=f7093cadde85559425422051421b7b80&twitter_id=460695962"; // Mariah
bool messageBegin = true;

static void my_callback (byte status, word off, word len)
{
	if (finished) return;
	
	Ethernet::buffer[len + (messageBegin ? openerSize : 0) + off] = 0;
	for (int i = 0; i < len - (messageBegin ? openerSize : 0); ++i)
	{
		jsonBuffer += ((const char*)(Ethernet::buffer) + (messageBegin ? openerSize : 0) + off)[i];
		if ((stringPos = jsonBuffer.indexOf("\":")) > 0)
		{
			jsonBuffer = jsonBuffer.substring(stringPos + 2);
			dataIndex += 1;
		}
		if (dataIndex == FOLLOWER_COUNT_DATA_INDEX)
		{
			//  We're at the data entry just past "followers_current", so wait for a comma and grab the string
			if ((stringPos = jsonBuffer.indexOf(",")) > 0)
			{
				jsonBuffer = jsonBuffer.substring(0, stringPos);
				finished = true;
				break;
			}
			
		}
	}

	if (finished)
	{
		//  Grab the number which should be in the buffer now
		int count = jsonBuffer.toInt();
		Serial.print(F("Follower Count Acquired: "));
		Serial.println(count);
		
		sevenSeg.setDigit(0, 0, count % 10, false);
		sevenSeg.setDigit(0, 1, (count % 100 - count % 10) / 10, false);
		sevenSeg.setDigit(0, 2, (count % 1000 - count % 100) / 100, false);
		sevenSeg.setDigit(0, 3, (count % 10000 - count % 1000) / 1000, false);
		sevenSeg.setDigit(0, 4, (count % 100000 - count % 10000) / 10000, false);
		sevenSeg.setDigit(0, 5, (count % 1000000 - count % 100000) / 100000, false);
		sevenSeg.setDigit(0, 6, (count % 10000000 - count % 1000000) / 1000000, false);
		sevenSeg.setDigit(0, 7, (count % 100000000 - count % 10000000) / 10000000, false);
	}
	messageBegin = false;
}
void writeArduinoOn7Segment() {
	for (int i = 0; i < 10; ++i)
	{
  		sevenSeg.setDigit(0,0,i,false);
  		delay(sevenSegDelay);
	}
	sevenSeg.clearDisplay(0);
	delay(sevenSegDelay);
} 

void setup ()
{
	Serial.begin(57600);
	ether.persistTcpConnection(true);
	Serial.println(F("PROGRAM START"));

	if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) Serial.println( "Failed to access Ethernet controller");
	if (!ether.dhcpSetup()) Serial.println("DHCP failed");

	ether.printIp("IP:  ", ether.myip);
	ether.printIp("GW:  ", ether.gwip);  
	ether.printIp("DNS: ", ether.dnsip);  

	if (!ether.dnsLookup(website)) Serial.println("DNS failed");
    
	ether.printIp("SRV: ", ether.hisip);
	
	
	sevenSeg.shutdown(0, false);
	/* Set the brightness to a medium values */
	sevenSeg.setIntensity(0,8);
	/* and clear the display */
	sevenSeg.clearDisplay(0);
}

void loop ()
{
	ether.packetLoop(ether.packetReceive());
	
	if (millis() > timer)
	{
		//writeArduinoOn7Segment();
		timer = millis() + 60000;
		messageBegin = true;
		finished = false;
		jsonBuffer = "";
		dataIndex = 0;
		ether.browseUrl(PSTR("/?"), arguments, website, my_callback);
	}
}