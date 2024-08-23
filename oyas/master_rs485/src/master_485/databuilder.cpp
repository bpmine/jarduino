#include "data.hpp"
#include "databuilder.h"
#include "slaves.hpp"

#include <RTClib.h>

DataBuilder::DataBuilder(Data *pData,bool raz)
{
  this->pData=pData;
  if (pData==nullptr)
    return;

  if (raz==true)
    pData->reset();

  pData->config_slaves=0;
}

inline unsigned short DataBuilder::mask(int addr)
{
  unsigned short msk=1<<addr;
  return msk;
}


void DataBuilder::set(Slave *slave)
{
  if (slave==nullptr)
    return;

  if ( (slave->addr>0) && (slave->addr<15) )
  {
    int addr=(unsigned char)slave->addr;
    unsigned short msk=mask(addr);

    if (slave->comm_ok)
      pData->comm_ok|=msk;

    if (slave->cmd)
      pData->cmd|=msk;

    if (slave->on)
      pData->on|=msk;

    pData->temp_dg[addr]=slave->temp_dg;
    pData->hum_pc[addr]=slave->hum_pc;
    pData->tick_ms[addr]=slave->last_slave_tick_ms;
    pData->total_s[addr]=slave->total_slave_on_s;
    pData->errs[addr]=slave->total_slave_errs;

    pData->config_slaves|=msk;
  }
}

void DataBuilder::set(Pump *pump)
{
  if (pump==nullptr)
    return;

  pData->flow=pump->flow;

  set((Slave *)pump);
}

void DataBuilder::set(Oya *oya)
{
  if (oya==nullptr)
    return;

  if ( (oya->addr>0) && (oya->addr<15) )
  {
    unsigned short msk=mask((unsigned char)oya->addr);

    if (oya->low)
      pData->low|=msk;
    if (oya->high)
      pData->high|=msk;

    set((Slave *)oya);
  }
}

void DataBuilder::set(int day,int month,int year,int h,int m,int s)
{
  pData->day=day;
  pData->month=month;
  pData->year=year;
  pData->hour=h;
  pData->min=m;
  pData->sec=s;
}

void DataBuilder::set(DateTime *pDt)
{
  if (pDt==nullptr)
    return;

  set(pDt->day(),pDt->month(),pDt->year(),pDt->hour(),pDt->minute(),pDt->second());
}
