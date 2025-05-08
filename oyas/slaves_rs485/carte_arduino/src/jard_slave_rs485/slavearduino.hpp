#ifndef SLAVEARDUINO_HEADER_INCLUDED
#define SLAVEARDUINO_HEADER_INCLUDED

#include "slavemgr.hpp"
#include "pins.h"

#include <arduino.h>

class SlaveArduino: public SlaveMgr
{
  private:
  	void send(unsigned char *buffer,int size) override
  	{
  	  digitalWrite(PIN_TX_EN,HIGH);
  	  delay(5);
  	  Serial.write(buffer,size);
  	  while ((UCSR0A & _BV (TXC0)) == 0) {}
  	  delay(2);
  	  digitalWrite(PIN_TX_EN,LOW);
  	}
  
  	unsigned long getTick(void) override
  	{
  	  return millis();
  	}

    void onCmdReceived(bool flgCmd) override
    {
     digitalWrite(PIN_DGB_CMD,!digitalRead(PIN_DGB_CMD));
    }

  public:
	  SlaveArduino() {}
	  ~SlaveArduino() {}

    void onSerialEvent(void) override
    {
      while (Serial.available()>0)
      {        
    	  int b=Serial.read();
    	  if ( (b>0) && (b<256) )        
    	    rxFrame.recv((unsigned char)b);
      }
    }

    void begin(unsigned char addr) override
    {
      SlaveMgr::begin(addr);
      
      pinMode(PIN_TX_EN,OUTPUT);
      digitalWrite(PIN_TX_EN,LOW);
      
      Serial.begin(SERIAL_SPEED);
    }
};

SlaveArduino Slave;

#endif
