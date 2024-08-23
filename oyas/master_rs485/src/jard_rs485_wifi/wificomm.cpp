/**
 * @file wificomm.cpp
 * @brief IMPLEMENTATION - GESTION DE LA COMMUNICATION WIFI/MAITRE OYA
 **/
#include "wificomm.h"

#include <arduino.h>
#include <ArduinoJson.h>
#include <EspMQTTClient.h>

#define COMM_SOF 1
#define COMM_EOF 2

WifiComm Comm;

WifiComm::WifiComm()
{
  pos=0;
  pStr=nullptr;
  flgMqtt_ok=false;
  flgMaster_ok=false;
}

void WifiComm::begin(HardwareSerial* pSerial)
{
  pStr=pSerial;
  if (pStr!=nullptr)
  {
    pStr->begin(9600);
  }

  pos=0;
  flgMqtt_ok=false;
  flgMaster_ok=false;

  tmrMqtt.start();
  tmrMaster.stop();
}

extern void sendData(char *strMsg);

void WifiComm::OnRecvFromMaster(char *msg)
{
  sendData(msg);
  tmrMaster.start();
}

void WifiComm::recv_management(void)
{
  while (pStr->available())
  {
    int b = pStr->read();
    if ((b > 0) && (b < 255))
    {
      if (pos==0)
      {
        if (b==COMM_SOF)
        {
          pos++;
        }
      }
      else if (pos<COMM_BUFFER_MAX_SIZE)
      {
        if ( (b!=COMM_EOF) && (b!=COMM_SOF) )
        {
          buffer[pos-1]=b;
          pos++;
        }
        else if (b==COMM_SOF)
        {
          pos=0;
        }
        else
        {
          buffer[pos-1]=0;
          OnRecvFromMaster((char *)buffer);
          pos=0;
        }
      }
      else
      {
        pos=0;
      }
    }
  }  
}

void WifiComm::sendMsgToMaster(const char *msg)
{
  tmrMqtt.start();

  if (pStr!=nullptr)
  {
    pStr->write("\x01");
    pStr->write(msg);
    pStr->write("\x02");
  }
}

void WifiComm::loop(void)
{
  if (pStr==nullptr)
    return;

  recv_management();

  if (tmrMqtt.tick()==true)
    flgMqtt_ok=false;
    
  if (tmrMaster.tick()==true)
    flgMaster_ok=false;
}

bool WifiComm::isMqttOk(void)
{
  return flgMqtt_ok;    
}

bool WifiComm::isMasterOk(void)
{
  return flgMaster_ok;
}
