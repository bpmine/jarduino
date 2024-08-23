#include "api.h"

#include "masterarduino.hpp"

#include <RTClib.h>

#include "databuilder.h"

MasterArduino Master;

static DS1307 _rtc;

void api_trace(bool en)
{
  Master.setTrace(en);
}

void api_master(bool on)
{
  Master.setEnable(on);
}

Pump *api_get_pump(void)
{
  SlavesList *l=Master.getSlavesList();
  return l->getPump();
}

Oya *api_find_first_oya(int &pos)
{
  SlavesList *l=Master.getSlavesList();
  Oya *p=l->findFirstOya(pos);
  return p;
}

Oya *api_find_next_oya(int &pos)
{
  SlavesList *l=Master.getSlavesList();
  Oya *p=l->findNextOya(pos);
  return p;
}

Oya *api_get_oya(int addr)
{
  return Master.getSlavesList()->getOya(addr);
}

void api_set_pompe(bool on)
{
  Master.set_pump(on);
}

void api_set_oya(char addr,bool on)
{
  Master.set_oya(addr, on);
}

void api_set_commands(unsigned short cmds)
{
  Master.set_commands(cmds);
}

void api_get_date_hour(int &day,int &month,int &year,int &hour,int &minute,int &second,int &dow)
{
  DateTime now = _rtc.now();

  day=now.day();
  month=now.month();
  year=now.year();
  hour=now.hour();
  minute=now.minute();
  second=now.second();
  dow=now.dayOfWeek();
}

void api_get_date_hour(int &day,int &month,int &year,int &hour,int &minute,int &second)
{
  int dow;
  return api_get_date_hour(day, month, year, hour, minute, second,dow);
}

void api_set_date(int day,int month,int year)
{
  DateTime now = _rtc.now();
  _rtc.adjust(DateTime(year,month,day,now.hour(),now.minute(),now.second()));
}

void api_set_hour(int hour,int minute,int second)
{
  DateTime now = _rtc.now();
  _rtc.adjust(DateTime(now.year(),now.month(),now.day(),hour,minute,second));
}

void api_set_slaves_config(unsigned short config)
{
  Master.set_config_slaves(config);
}

unsigned short api_get_slaves_config(void)
{
  return Master.get_config_slaves();
}

void api_raz_all_time(void)
{
  Master.set_raz_time(0xFFFF);
}

void api_raz_time(unsigned char addr)
{
  if ((addr>0) && (addr<15))
  {
    unsigned short msk=1<<(addr-1);
    Master.set_raz_time(msk);
  }
}

void api_raz_all_errs(void)
{
  Master.set_raz_errs(0xFFFF);
}

void api_ping(unsigned char addr)
{
  unsigned short msk=1<<(addr-1);
  Master.set_to_ping(msk);
}

unsigned short api_get_pong(void)
{
  return Master.get_pong_states();
}

void api_latch_data(Data *pData)
{
  DataBuilder bld(pData);
  Pump *pmp=api_get_pump();
  bld.set(pmp);
  int pos;
  Oya *oya=api_find_first_oya(pos);
  while (oya!=nullptr)
  {
    bld.set(oya);
    oya=api_find_next_oya(pos);
  }

  DateTime now = _rtc.now();
  bld.set(&now);
}

