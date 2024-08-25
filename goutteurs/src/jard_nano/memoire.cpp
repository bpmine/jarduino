#include "memoire.h"

#include <EEPROM.h>

#define MAGIC1  (0xA5)
#define MAGIC2  (0xDA)

bool memoire_save_id(T_ID *pID)
{
  pID->bMagicStart=MAGIC1;
  pID->bMagicEnd=MAGIC2;
  
  unsigned char *p=(unsigned char *)pID;
  for (int i = 0 ; i < sizeof(T_ID) ; i++)
  {
    EEPROM.write(i,p[i]);
  }    

  return true;  
}

bool memoire_load_id(T_ID *pID)
{
  unsigned char *p=(unsigned char *)pID;

  for (int i = 0 ; i < sizeof(T_ID) ; i++)
  {
    p[i]=EEPROM.read(i);
  }  

  if ( (pID->bMagicStart!=MAGIC1) || (pID->bMagicEnd!=MAGIC2) )
  {
    return false;
  }

  return true;  
}

bool memoire_stats_reset(int addr)
{
  if ( (addr<MEM_START_ADDR_STATS) || (addr>=MEM_STATS_ADDR_MAX) )
    return false;

  EEPROM.write(addr,0);
  EEPROM.write(addr+1,0);

  return true;
}

bool memoire_stats_read(int addr,unsigned short *o_pusValue)
{
  if ( (addr<MEM_START_ADDR_STATS) || (addr>=MEM_STATS_ADDR_MAX) )
    return false;

  int a=EEPROM.read(addr);
  int b=EEPROM.read(addr+1);
  unsigned short ctr=(b<<8)|a;
  *o_pusValue=ctr;
  
  return true;
}

bool memoire_stats_inc(int addr)
{
  unsigned short usVal;

  if (memoire_stats_read(addr,&usVal)==false)
    return false;

  usVal++;  

  EEPROM.update(addr,usVal&0xFF);
  EEPROM.update(addr+1,(usVal>>8)&0xFF);

  return true;  
}

bool memoire_set(int addr,unsigned char i_ucValue)
{
  EEPROM.update(addr,i_ucValue);

  return true;  
}

bool memoire_read(int addr,unsigned char *o_pucValue)
{
  *o_pucValue=EEPROM.read(addr);

  return true;    
}
