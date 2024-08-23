/**
 * @file terminal.cpp
 * @brief Gestion du terminal
 *
 * @todo Ajouter les comandes setdate, settime
 **/
#ifndef TERMINAL_HEADER_INCLUDED
#define TERMINAL_HEADER_INCLUDED

#include <arduino.h>

#include "api.h"
#include "master.h"

char _bufferTerm[200];
int _posBufferTerm=0;

static void _help(void)
{
  Serial.println("____________________");
  Serial.println("Aide:");
  Serial.println("  - test");
  Serial.println("  - master on/off");
  Serial.println("  - pump on/off");
  Serial.println("  - pump");
  Serial.println("  - oyas");
  Serial.println("  - oya @ on/off");
  Serial.println("  - trace on/off");
  Serial.println("  - stats");
  Serial.println("  - razerrs");
  Serial.println("  - raztime [@ ou all]");
  Serial.println("  - ping @");
  Serial.println("  - pong");
  Serial.println("  - setdate dd/mm/yyyy");
  Serial.println("  - settime hh:mm:ss");
  Serial.println("  - time");
  Serial.println("  - confslaves");
}

static bool _parseOnOff(const char *params,bool def)
{
  if (strcmp(params,"on")==0)
    return true;
  else if (strcmp(params,"off")==0)
    return false;
  else
    return def;
}

static void _OnOffAns(const char *strTitle,bool on)
{
  Serial.print(strTitle);
  Serial.print(": ");
  if (on==true)
    Serial.println("ON");
  else
    Serial.println("OFF");
}

static void _term_exec_oya(const char *strParams)
{
  int addr;
  char onoff[20];
  if ( (strlen(strParams)>19) || (sscanf(strParams,"%X %s",&addr,onoff)!=2) || (addr<2) || (addr>14))
  {
    Serial.println("Format incorrect!");
    return;      
  }
  
  int pos=0;
  Oya * p=api_find_first_oya(pos);
  while (p!=NULL)
  {
    if (p->addr==addr)
    {
      char tmp[10];
      bool on=_parseOnOff(onoff,false);
      api_set_oya(addr, on);
      sprintf(tmp,"Oya %01X",addr);
      _OnOffAns(tmp,on);      
      return;
    }
    p=api_find_next_oya(pos);
  }
  
  Serial.println("Oya non trouvé!");
}

static void _term_exec_pump(const char *strParams)
{
    Pump *p=api_get_pump();
    if (p==nullptr)
    {
      Serial.println("Erreur interne: pump est NULL");
      return;
    }

    if (strParams==NULL)
    {      
      Serial.println("__________");
      Serial.print("Pompe ");
      Serial.print((int)p->addr);
      Serial.println(":");

      if (p->comm_ok==true)
      {
        Serial.print("  - Etat : ");
        Serial.println(p->on==true?"ON":"OFF");
        Serial.print("  - Flow : ");
        Serial.println(p->flow);
        Serial.print("  - temp : ");
        Serial.print(p->temp_dg);
        Serial.println("°C");
        Serial.print("  - hum  : ");
        Serial.print(p->hum_pc);
        Serial.println("%");
        Serial.print("  - voltage    : ");
        Serial.println(p->voltage);
        Serial.print("  - total s    : ");
        Serial.print(p->total_slave_on_s);
        Serial.println("s");
        Serial.print("  - total errs : ");
        Serial.println(p->total_slave_errs);
        Serial.print("  - tick       : ");
        Serial.println(p->last_slave_tick_ms);
      }
      else
      {
        Serial.println("NO COMM");
      }
    }
    else
    {
      bool on=_parseOnOff(strParams,false);
      api_set_pompe(on);
      _OnOffAns("Pompe",on);
    }
}

static void _term_exec_oyas(void)
{
    int pos=0;
    Serial.println("__________");
    Oya *pOya=api_find_first_oya(pos);
    while (pOya!=NULL)
    {
      char tmp[12];
      sprintf(tmp,"Oya @%01X : ",pOya->addr);
      Serial.print(tmp);

      if (pOya->comm_ok==true)
      {
        char info[200];
        sprintf(info," %-3s %-4s %-4s t:%3d°C h:%3d%% v:%u ton:%us tick:%ums",
                pOya->on==true?"on":"off",
                pOya->low==false?"":"low",
                pOya->high==false?"":"high",
                pOya->temp_dg,
                pOya->hum_pc,
                pOya->voltage,
                pOya->total_slave_on_s,
                pOya->last_slave_tick_ms
                );

        Serial.println(info);
      }
      else
      {
        Serial.println("NO COMM");
      }

      pOya=api_find_next_oya(pos);
    }
    Serial.println();
}

static void _term_disp_slave_stats(Slave *p)
{
  char tmp[20];

  if (p->addr!=1)
    sprintf(tmp,"Oya %01X   : ",p->addr);
  else
    sprintf(tmp,"Pompe %01X : ",p->addr);

  Serial.print(tmp);

  sprintf(tmp," off=%ld ",p->cycles_since_off);Serial.print(tmp);
  sprintf(tmp," on=%ld ",p->cycles_since_on);Serial.print(tmp);
  sprintf(tmp," nok=%ld ",p->cycles_since_nok);Serial.print(tmp);
  sprintf(tmp," ok=%ld ",p->cycles_since_ok);Serial.print(tmp);
  sprintf(tmp," errs=%ld ",p->cycles_errors);Serial.print(tmp);
  Serial.println("");
}

static void _term_exec_stats(void)
{
  Serial.println("___________________");
  Serial.println("Statistiques:");
  int pos=0;
  Oya *pOya=api_find_first_oya(pos);
  while (pOya!=NULL)
  {
    _term_disp_slave_stats(pOya);
    pOya=api_find_next_oya(pos);
  }

  Pump *pPump=api_get_pump();
  if (pPump!=NULL)
  _term_disp_slave_stats(pPump);
}

static void _term_exec_razerrs(void)
{
  int pos=0;
  Oya *pOya=api_find_first_oya(pos);
  while (pOya!=NULL)
  {
    pOya->razErrors();
    pOya=api_find_next_oya(pos);
  }
  Pump *pPump=api_get_pump();
  if (pPump!=NULL)
    pPump->razErrors();
  
  api_raz_all_errs();

  Serial.println("RAZ errors...");
}

static void _term_exec_raztime(const char *strParams)
{
  int addr;
  if (strcmp(strParams,"all")==0)
  {
    api_raz_all_time();
    Serial.println("RAZ time all...");
  }
  else if ( (strlen(strParams)>19) || (sscanf(strParams,"%X",&addr)!=1) || (addr<1) || (addr>14))
  {
    Serial.println("Parametres incorrects");
  }
  else
  {
    api_raz_time((unsigned char)addr);
    Serial.print("RAZ time @");
    char tmp[5];
    sprintf(tmp,"%X",addr);
    Serial.println(tmp);
  }
}

static void _term_exec_time(void)
{
  int year,month,day,hour,minute,second,dow;

  const char *dys[]={
      "Dimanche",
      "Lundi",
      "Mardi",
      "Mercredi",
      "Jeudi",
      "Vendredi",
      "Samedi"
  };

  api_get_date_hour(day, month, year, hour, minute, second,dow);

  if ( (dow<0) || (dow>6) )
    dow=0;

  char tmp[50];
  sprintf(tmp,"%s %02d/%02d/%02d %02d:%02d:%02d",dys[dow],day,month,year,hour,minute,second);
  Serial.println(tmp);
}

static void _term_exec_settime(const char *strParams)
{
  int hour,minute,second;
  if ( sscanf(strParams,"%02d:%02d:%02d",&hour,&minute,&second)==3 )
  {
    api_set_hour(hour, minute, second);
    _term_exec_time();
  }
  else
  {
    Serial.println("Paramètre incorrect!");
  }
}

static void _term_exec_setdate(const char *strParams)
{
  int day,month,year;
  if ( sscanf(strParams,"%02d/%02d/%04d",&day,&month,&year)==3 )
  {
    api_set_date(day,month,year);
    _term_exec_time();
  }
  else
  {
    Serial.println("Paramètre incorrect!");
  }
}

static void _term_exec_config_slaves(const char *strParams)
{
  if (strParams==NULL)
  {
    unsigned short mask=api_get_slaves_config();
    Serial.print("Config esclaves: ");
    Serial.println(mask,HEX);

    for (int i=0;i<14;i++)
    {
      unsigned short m=1<<i;
      if ((mask&m)==m)
      {
        if (i==0)
        {
          Serial.println("  - POMPE @1");
        }
        else
        {
          Serial.print("  - OYA   @");
          Serial.println(i+1,HEX);
          delay(5);
          yield();
        }
      }
    }
  }
  else
  {
    int mask;
    int addr;
    char cmd[10];
    if ( ( sscanf(strParams,"%3s %1X",cmd,&addr)==2 ) && (addr>0) && (addr<15) )
    {
      if (strcmp(cmd,"add")==0)
      {
        unsigned short mask=api_get_slaves_config();
        mask|=( 1<< (addr-1) );
        api_set_slaves_config(mask);
        _term_exec_config_slaves(NULL);
      }
      else if (strcmp(cmd,"rm")==0)
      {
        unsigned short mask=api_get_slaves_config();
        mask&=~( 1<< (addr-1) );
        api_set_slaves_config(mask);
        _term_exec_config_slaves(NULL);
      }
    }
    else if ( sscanf(strParams,"%X",&mask)==1 )
    {
      api_set_slaves_config((unsigned short)mask);
      _term_exec_config_slaves(NULL);
    }
    else
    {
      Serial.println("Parametres incorrects");
    }
  }
}

static void _term_exec_ping(const char *strParams)
{
  int addr;
  if ( (strlen(strParams)>19) || (sscanf(strParams,"%X",&addr)!=1) || (addr<1) || (addr>14))
  {
    Serial.println("Parametres incorrects");
  }
  else
  {
    char tmp[15];
    sprintf(tmp,"Ping @%X",addr);
    Serial.println(tmp);
    api_ping(addr);
  }
}

static void _term_exec_pong(void)
{
  unsigned short pong=api_get_pong();
  for (int i=0;i<15;i++)
  {
    unsigned short msk=1<<i;
    if ((msk&pong)==msk)
    {
      char tmp[15];
      sprintf(tmp,"@%X is ponged",i);
      Serial.println(tmp);
      return;
    }
  }

  Serial.println("Rien.");
}

static void _execCmd(const char *strCmd,const char *strParams)
{
  if (strcmp(strCmd,"test")==0)
  {
    Serial.println("TEST OK");
  }
  else if (strcmp(strCmd,"help")==0)
  {
    _help();
  }
  else if (strcmp(strCmd,"pump")==0)
  {
    _term_exec_pump(strParams);
  }
  else if (strcmp(strCmd,"oyas")==0)
  {
    _term_exec_oyas();
  }
  else if (strcmp(strCmd,"oya")==0)
  {
    _term_exec_oya(strParams);
  }
  else if (strcmp(strCmd,"master")==0)
  {
    bool on=_parseOnOff(strParams,false);
    api_master(on);
    _OnOffAns("Maitre 485",on);
  }
  else if (strcmp(strCmd,"trace")==0)
  {
    bool on=_parseOnOff(strParams,false);
    api_trace(on);
    _OnOffAns("Traces",on);
  }
  else if (strcmp(strCmd,"stats")==0)
  {
    _term_exec_stats();
  }
  else if (strcmp(strCmd,"razerrs")==0)
  {
    _term_exec_razerrs();
  }
  else if (strcmp(strCmd,"raztime")==0)
  {
    _term_exec_raztime(strParams);
  }
  else if (strcmp(strCmd,"time")==0)
  {
    _term_exec_time();
  }
  else if (strcmp(strCmd,"setdate")==0)
  {
    _term_exec_setdate(strParams);
  }
  else if (strcmp(strCmd,"settime")==0)
  {
    _term_exec_settime(strParams);
  }
  else if (strcmp(strCmd,"confslaves")==0)
  {
    _term_exec_config_slaves(strParams);
  }
  else if (strcmp(strCmd,"ping")==0)
  {
    _term_exec_ping(strParams);
  }
  else if (strcmp(strCmd,"pong")==0)
  {
    _term_exec_pong();
  }
}

void serialEvent(void)
{
  if (Serial.available()>0)
  {
    String st=Serial.readString();
    if (st.length()>20)
      return;

    st.trim();

    int inx=st.indexOf(" ");
    if (inx==-1)
    {
      _execCmd(st.c_str(),NULL);
    }
    else
    {
      _execCmd(st.substring(0,inx).c_str(),st.substring(inx+1).c_str());
    }
  }
}

#endif
