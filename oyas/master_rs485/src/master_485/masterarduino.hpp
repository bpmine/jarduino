#ifndef MASTERARDUINO_HEADER_INCLUDED
#define MASTERARDUINO_HEADER_INCLUDED

#include "master.h"
#include "pins.h"
#include <arduino.h>

class MasterArduino : public Master
{
private:
  HardwareSerial* pStr;
  int txen;

  void sendBytes(unsigned char* buffer, int size) override
  {
    if (pStr == nullptr)
      return;

    digitalWrite(this->txen, HIGH);
    delay(2);
    pStr->write(buffer,size);
    if (pStr==&Serial)
      while ((UCSR0A & _BV (TXC0)) == 0) {}
    else if (pStr==&Serial1) 
      while ((UCSR1A & _BV (TXC1)) == 0) {}
    else if (pStr==&Serial2) 
      while ((UCSR2A & _BV (TXC2)) == 0) {}
    else if (pStr==&Serial3) 
      while ((UCSR3A & _BV (TXC3)) == 0) {}
    
    delay(1);
    digitalWrite(this->txen, LOW);
  }

  int available(void) override
  {
	  return pStr->available();
  }

  int readByte(void) override
  {
	  return pStr->read();
  }

  void setPower(bool on) override
  {
    digitalWrite(PIN_PWR_ON,on==true?HIGH:LOW);
  }

  void log(const char *msg) override
  {
    if (flgTrace==true)
      Serial.println(msg);
  }

  void log(const char *key,int val) override
  {
    if (flgTrace==true)
    {
      Serial.print(key);
      Serial.print(": ");
      Serial.println(val);
    }
  }

public:
  MasterArduino()
  {
    txen = -1;
    pStr = nullptr;
  }

  void begin(HardwareSerial* pSerial, int txen)
  {
    this->txen = txen;
    pinMode(this->txen, OUTPUT);
    digitalWrite(this->txen, LOW);
    pSerial->begin(BUS_RS485_SPEED);
    this->pStr = pSerial;

    init();
  }
};

extern MasterArduino Master;

#endif
