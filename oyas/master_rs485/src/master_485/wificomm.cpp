/**
 * @file wificomm.cpp
 * @brief IMPLEMENTATION - GESTION DE LA COMMUNICATION WIFI
 **/
#include "wificomm.h"

#include <arduino.h>
#include <ArduinoJson.h>

#include "api.h"
#include "slaves.hpp"
#include "pins.h"
#include "Logger.h"

#include "data.hpp"

#define COMM_SOF 1
#define COMM_EOF 2

WifiComm Comm;

typedef enum {
  OFF,
  IDLE,               ///< Ne fait rien
  SEND_DATA,          ///< Envoi de l'info master
  WAIT_ACK_DATA      ///< Attente Ack suite info master
} T_SEND_STATE;


Data data;
T_SEND_STATE send_state;

WifiComm::WifiComm()
{
  pos=0;
  pStr=nullptr;
  flgRemoteActive=false;
  flgAlive=false;
  commands=0;
  stophighs=0;
  flgDepanage=false;
  send_state=OFF;
  tmrSendData.start();
}

void WifiComm::begin(HardwareSerial* pSerial)
{
  pStr=pSerial;
  if (pStr!=nullptr)
  {
    pStr->begin(9600);
    digitalWrite(PIN_PWR_WIFI,LOW);
    digitalWrite(PIN_PWR_WIFI_INV,HIGH);
  }

  pos=0;
  flgAlive=false;
  flgRemoteActive=false;
  commands=0;
  send_state=OFF;
  tmrSendData.start();
}

void WifiComm::setPower(bool on)
{
  if (on)
  {
    digitalWrite(PIN_PWR_WIFI,HIGH);
    digitalWrite(PIN_PWR_WIFI_INV,LOW);
    send_state=IDLE;
  }
  else
  {

    digitalWrite(PIN_PWR_WIFI,LOW);
    digitalWrite(PIN_PWR_WIFI_INV,HIGH);
    send_state=OFF;
  }

  pos=0;
  flgAlive=false;
  flgRemoteActive=false;
  commands=0;
  tmrSendData.start();
}

void WifiComm::pubDataInfo(void)
{
  StaticJsonDocument<400> doc;

  doc["type"]="data";
  doc["slaves"]=data.config_slaves;
  doc["comms"]=data.comm_ok;
  doc["cmds"]=data.cmd;
  doc["ons"]=data.on;
  doc["lows"]=data.low;
  doc["highs"]=data.high;
  doc["flow"]=data.flow;
  doc["bigs"]=data.bigs;
  doc["rmt"]=data.remote;

  char dte[40];
  sprintf(dte,"%04d-%02d-%02dT%02d:%02d:%02dZ",
          data.year,
          data.month,
          data.day,
          data.hour,
          data.min,
          data.sec
          );
  doc["date"]=dte;

  char jsonOutput[400];
  serializeJson(doc, jsonOutput);

  pStr->print("\x01");
  pStr->print(jsonOutput);
  pStr->print("\x02");
}

void WifiComm::execCommands(unsigned short cmds,unsigned short stophighs,bool active,bool depan)
{
  if (active==true)
  {
    flgRemoteActive=true;
    commands=cmds;
    this->stophighs=stophighs;
    flgDepanage=depan;
  }
  else
  {
    flgRemoteActive=false;
    commands=0;
    this->stophighs=0;
    flgDepanage=0;
  }
}

bool WifiComm::isRemoteActive(void)
{
  return flgRemoteActive;
}

bool WifiComm::isAlive(void)
{
  return flgAlive;
}
unsigned short WifiComm::getCommands(void)
{
  return commands;
}

bool WifiComm::isDepanage(void)
{
  return flgDepanage;
}

unsigned short WifiComm::getStopHighs(void)
{
  return stophighs;
}

void WifiComm::recv(void)
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

          StaticJsonDocument<50> doc;
          DeserializationError error = deserializeJson(doc, buffer);
          pos=0;

          if (error)
          {
            pStr->print("\x01");
            pStr->print(F("ERR: Erreur de dé-sérialisation: "));
            pStr->print(error.f_str());
            pStr->print("\x02");

            return;
          }
          if (strcmp(doc["req"],"cmds")==0)
          {
            if (doc.containsKey("cmds") && doc.containsKey("ctrl"))
            {
              unsigned short cmds=doc["cmds"];
              bool ctrl=doc["ctrl"];
              unsigned short stops=cmds&0xFFFE;
              if (doc.containsKey("highs"))
              {
                unsigned short tmp=doc["highs"];
                stops=tmp&0xFFFE;
              }

              bool depan=false;
              if (doc.containsKey("depan"))
                depan=doc["depan"];

              execCommands(cmds,stops,ctrl,depan);

              tmrRemoteActive.start();
              tmrAlive.start();
            }
            else
            {
              execCommands(0,0,false,false);
            }
          }
          else if (strcmp(doc["req"],"ack")==0)
          {
            Serial.println("Ack");
            if (send_state==WAIT_ACK_DATA)
              send_state=IDLE;

            tmrRemoteActive.start();
            tmrAlive.start();
          }
          else if (strcmp(doc["req"],"razt")==0)
          {
            api_raz_all_time();
            tmrRemoteActive.start();
            tmrAlive.start();
          }
          else if (strcmp(doc["req"],"setcfg")==0)
          {
            if (doc.containsKey("slaves"))
            {
              int slaves=doc["slaves"];
              api_set_slaves_config(slaves);
            }

            if (doc.containsKey("bigs"))
            {
              int bigs=doc["bigs"];
              api_set_bigs(bigs);
            }

            tmrRemoteActive.start();
            tmrAlive.start();
          }
          else if (strcmp(doc["req"],"test")==0)
          {
            pStr->print("\x01");
            pStr->print("{\"type\":\"test\",\"res\":\"true\"}");
            pStr->print("\x02");
            tmrAlive.start();
          }
        }
      }
      else
      {
        pos=0;
      }
    }
  }
}

void WifiComm::sendTask(void)
{
  switch (send_state)
  {
    case OFF:
    {
      break;
    }
    case IDLE:
    {
      if (tmrSendData.tick()==true)
        sendData();
      break;
    }
    case SEND_DATA:
    {
      Serial.println("Send");
      pubDataInfo();
      tmrSendAck.start();
      send_state=WAIT_ACK_DATA;
      break;
    }
    case WAIT_ACK_DATA:
    {
      if (tmrSendAck.tick()==true)
        send_state=SEND_DATA;
      break;
    }
  }
}

void WifiComm::loop(void)
{
  if (pStr==nullptr)
    return;

  recv();
  sendTask();

  if (tmrRemoteActive.tick()==true)
  {
    flgRemoteActive=false;
    commands=0;
    stophighs=0;
  }

  if (tmrAlive.tick()==true)
  {
    flgAlive=false;
  }
}

void WifiComm::sendData(void)
{
  if (send_state==IDLE)
  {
    api_latch_data(&data);
    data.remote=flgRemoteActive;
    Serial.println("Latch");
    send_state=SEND_DATA;
    tmrSendData.start();
  }
}

