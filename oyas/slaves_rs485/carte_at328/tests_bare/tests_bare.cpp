#include <Arduino.h>

#define ENABLE_RS485

void setup()
{	
	pinMode(A4,OUTPUT);
	digitalWrite(A4,HIGH);
	
	#ifdef ENABLE_RS485
		pinMode(A5,OUTPUT);
		digitalWrite(A5,LOW);
		Serial.begin(9600);
	#endif
}

void loop()
{
	digitalWrite(A4,LOW);
	delay(1000);
	digitalWrite(A4,HIGH);
	#ifdef ENABLE_RS485
		digitalWrite(A5,HIGH);
		delay(20);
		Serial.print("A");
		delay(280);
		digitalWrite(A5,LOW);
	#endif
	delay(700);
}


