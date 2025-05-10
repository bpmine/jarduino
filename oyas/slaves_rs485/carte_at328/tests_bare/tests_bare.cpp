#include <Arduino.h>

void setup()
{	
	pinMode(A4,OUTPUT);
	digitalWrite(A4,HIGH);
}

void loop()
{
	digitalWrite(A4,LOW);
	delay(1000);
	digitalWrite(A4,HIGH);
	delay(1000);
}


