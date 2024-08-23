#ifndef DATA_HEADER_INCLUDED
#define DATA_HEADER_INCLUDED

#define MAX_SLAVES  (15)

class Data
{
  public:
    unsigned short config_slaves;
    unsigned short comm_ok;
    unsigned short cmd;
    unsigned short on;
    unsigned short high;
    unsigned short low;
    unsigned short flow;
    unsigned int temp_dg[MAX_SLAVES];
    unsigned int hum_pc[MAX_SLAVES];
    unsigned int volt[MAX_SLAVES];
    unsigned short tick_ms[MAX_SLAVES];
    unsigned short total_s[MAX_SLAVES];
    unsigned short errs[MAX_SLAVES];

    int day;
    int month;
    int year;
    int hour;
    int min;
    int sec;

    Data(void)
    {
      reset();
    }

    void reset(void)
    {
      config_slaves=0;
      comm_ok=0;
      cmd=0;
      on=0;
      high=0;
      low=0;
      flow=0;

      for (int i=0;i<MAX_SLAVES;i++)
      {
        volt[i]=0;
        temp_dg[i]=0;
        hum_pc[i]=0;
        tick_ms[i]=0;
        total_s[i]=0;
        errs[i]=0;
      }

      day=1;
      month=1;
      year=2024;
      hour=0;
      min=0;
      sec=0;
    }

    void copyFrom(Data *pSrc)
    {
      config_slaves=pSrc->config_slaves;
      comm_ok=pSrc->comm_ok;
      cmd=pSrc->cmd;
      on=pSrc->on;
      high=pSrc->high;
      low=pSrc->low;
      flow=pSrc->flow;

      for (int i=0;i<MAX_SLAVES;i++)
      {
        temp_dg[i]=pSrc->temp_dg[i];
        hum_pc[i]=pSrc->hum_pc[i];
        tick_ms[i]=pSrc->tick_ms[i];
        total_s[i]=pSrc->total_s[i];
        errs[i]=pSrc->errs[i];
        volt[i]=pSrc->volt[i];
      }

      day=pSrc->day;
      month=pSrc->month;
      year=pSrc->year;
      hour=pSrc->hour;
      min=pSrc->min;
      sec=pSrc->sec;
    }

    unsigned short whoChanged(Data *pSrc)
    {
      unsigned short ret=0;

      unsigned short a= config_slaves ^ pSrc->config_slaves;
      unsigned short b= comm_ok ^ pSrc->comm_ok;
      unsigned short c = cmd ^ pSrc->cmd;
      unsigned short d = on ^ pSrc->on;
      unsigned short e = high ^ pSrc->high;
      unsigned short f = low ^ pSrc->low;
      unsigned short g = flow ^pSrc->flow;

      ret = a | b | c | d | e | f | g;

      for (int i=0;i<MAX_SLAVES;i++)
      {
        unsigned short msk=1<<i;
        if (total_s[i]!=pSrc->total_s[i])
          ret|=msk;
      }

      return ret;
    }
};

#endif
