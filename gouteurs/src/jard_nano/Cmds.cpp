/*
   Cmds.cpp

    Created on: 26 juin 2020
        Author: Utilisateur
*/

#include "Cmds.h"
#include "IJardCmd.h"
#include <arduino.h>
#include "pins.h"
#include "autom.h"
#include "memoire.h"

extern "C"{
#include "mdbus.h"
}

#define MODBUS_JARDUINO_VERSION (8)


void user_mdbus_send(void *back,unsigned char* pbuff, int sz)
{
  if (back!=NULL)
  {
    HardwareSerial *pSerial=(HardwareSerial*)back;
    pSerial->write(pbuff,sz);
  }
}

int user_mdbus_read_coils(unsigned short addr, unsigned short count, unsigned char* o_pBuffer)
{
  cmds.m_pJardCmd->aliveComm();
  
  for (int i=0;i<count;i++)
  {
    unsigned char ucVal;
    switch (addr+i)
    {
      case 10:ucVal=mbs.get(MB_PMP1_FORCED);break;
      case 11:ucVal=mbs.get(MB_PMP1_ENABLE);break;
      case 12:ucVal=mbs.get(MB_PMP1_AUTO);break;
      case 13:ucVal=mbs.get(MB_PMP1_REMOTE);break;
      case 14:ucVal=mbs.get(MB_PMP1_RM_CMD);break;

      case 20:ucVal=mbs.get(MB_PMP2_FORCED);break;
      case 21:ucVal=mbs.get(MB_PMP2_ENABLE);break;
      case 22:ucVal=mbs.get(MB_PMP2_AUTO);break;
      case 23:ucVal=mbs.get(MB_PMP2_REMOTE);break;
      case 24:ucVal=mbs.get(MB_PMP2_RM_CMD);break;

      default:return MDBUS_ERR;
    }
          
    mdbus_fill_coil_data(o_pBuffer, i, ucVal);
  }
  
  return MDBUS_OK; 
}

int user_mdbus_write_coils(unsigned short addr, unsigned short count, unsigned char* i_pBuffer)
{
  cmds.m_pJardCmd->aliveComm();
  
  for (int i = 0; i < count; i++)
  {
    unsigned char val = mdbus_get_coil_data(i_pBuffer, i);
    switch (addr+i)
    {
      case 10:mbs.fromBool(MB_PMP1_FORCED,val);break;
      case 11:mbs.fromBool(MB_PMP1_ENABLE,val);memoire_set(MEM_SETTINGS_ADDR_PUMP1_EN,val);break;
      case 12:mbs.fromBool(MB_PMP1_AUTO,val);memoire_set(MEM_SETTINGS_ADDR_PUMP1_AUTO,val);break;
      case 13:mbs.fromBool(MB_PMP1_REMOTE,val);memoire_set(MEM_SETTINGS_ADDR_PUMP2_REMOTE,val);break;
      case 14:mbs.fromBool(MB_PMP1_RM_CMD,val);break;
      

      case 20:mbs.fromBool(MB_PMP2_FORCED,val);break;
      case 21:mbs.fromBool(MB_PMP2_ENABLE,val);memoire_set(MEM_SETTINGS_ADDR_PUMP2_EN,val);break;
      case 22:mbs.fromBool(MB_PMP2_AUTO,val);memoire_set(MEM_SETTINGS_ADDR_PUMP2_AUTO,val);break;
      case 23:mbs.fromBool(MB_PMP2_REMOTE,val);memoire_set(MEM_SETTINGS_ADDR_PUMP2_REMOTE,val);break;
      case 24:mbs.fromBool(MB_PMP2_RM_CMD,val);break;
      
      default:return MDBUS_ERR;
    }
  }

  return MDBUS_OK;  
}

int user_mdbus_read_inputs(unsigned short addr, unsigned short count, unsigned char* o_pBuffer)
{
  cmds.m_pJardCmd->aliveComm();
  
  for (int i=0;i<count;i++)
  {
    unsigned char ucVal;
    switch (addr+i)
    {
      case 0:ucVal=mbs_outputs.get(OB_CMD_PMP1);break;
      case 1:ucVal=mbs_outputs.get(OB_CMD_PMP2);break;      
      case 2:ucVal=mbs.get(MB_BATT_OK);break;
      case 3:ucVal=mbs.get(MB_SUN_OK);break;
      case 4:ucVal=mbs.get(MB_VEILLE);break;

      default:return MDBUS_ERR;
    }
          
    mdbus_fill_coil_data(o_pBuffer, i, ucVal);
  }
  
  return MDBUS_OK; 
}

int user_mdbus_read_input_registers(unsigned short addr, unsigned short count, unsigned char* o_pBuffer)
{
  cmds.m_pJardCmd->aliveComm();
  
    for (int i = 0; i < count; i++)
    {
      unsigned short usVal;
      switch (addr+i)
      {
        case 0:usVal=cmds.m_pJardCmd->getBattLevel();break;
        case 1:usVal=cmds.m_pJardCmd->getSunLevel();break;
        case 2:usVal=cmds.m_pJardCmd->getTemp();break;
        case 3:usVal=cmds.m_pJardCmd->getHum();break;
        case 4:usVal=cmds.m_pJardCmd->getTmrComm_s();break;
        case 5:usVal=cmds.m_pJardCmd->getTmrVeille_s();break;

        case 10:usVal=cmds.m_pJardCmd->getTmrPmp1_min();break;

        case 20:usVal=cmds.m_pJardCmd->getTmrPmp2_min();break;

        case 100:memoire_stats_read(MEM_STATS_ADDR_TOT_BOOTS,&usVal);break;
        case 101:memoire_stats_read(MEM_STATS_ADDR_TOT_P1_H,&usVal);break;
        case 102:memoire_stats_read(MEM_STATS_ADDR_TOT_P2_H,&usVal);break;
        case 103:memoire_stats_read(MEM_STATS_ADDR_TOT_SUN_H,&usVal);break;
        case 104:memoire_stats_read(MEM_STATS_ADDR_TOT_BTN1,&usVal);break;
        case 105:memoire_stats_read(MEM_STATS_ADDR_TOT_BTN2,&usVal);break;

        case 200:
        {
          T_ID id;
          
          if (memoire_load_id(&id)==true)
            usVal=id.bVersion;
          else
            usVal=0;
            
          break;          
        }

        case 201:
        {
          T_ID id;
          
          if (memoire_load_id(&id)==true)
            usVal=id.bSerial;
          else
            usVal=0;
            
          break;          
        }
        case 202:usVal=MODBUS_JARDUINO_VERSION;break;
        
        default:return MDBUS_ERR;
      }
      mdbus_fill_register_data(o_pBuffer, i, usVal);
    }

    return MDBUS_OK;
}

int user_mdbus_read_holding_registers(unsigned short addr, unsigned short count, unsigned char* o_pBuffer)
{
  unsigned short usYear=0;
  unsigned char ucMonth=0;
  unsigned char ucDay=0;
  unsigned char ucHour=0;
  unsigned char ucMin=0;
  unsigned char ucSec=0;

  unsigned char ucHourStart1=0;
  unsigned char ucMinStart1=0;
  unsigned char ucDuration1=0;
  unsigned char ucDaysWeek1=0;  

  unsigned char ucHourStart2=0;
  unsigned char ucMinStart2=0;
  unsigned char ucDuration2=0;
  unsigned char ucDaysWeek2=0;  

  cmds.m_pJardCmd->aliveComm();

  if ( (addr>=0) && (addr<=5) )
  {
    cmds.m_pJardCmd->getDateTime(&usYear,&ucMonth,&ucDay,&ucHour,&ucMin,&ucSec);
  }

  cmds.m_pJardCmd->getSheduler(1,&ucHourStart1,&ucMinStart1,&ucDuration1,&ucDaysWeek1);
  cmds.m_pJardCmd->getSheduler(2,&ucHourStart2,&ucMinStart2,&ucDuration2,&ucDaysWeek2);
  
  for (int i = 0; i < count; i++)
  {
    unsigned short usVal;
    switch (addr+i)
    {
      case 0:usVal=usYear;break;
      case 1:usVal=ucMonth;break;
      case 2:usVal=ucDay;break;
      case 3:usVal=ucHour;break;
      case 4:usVal=ucMin;break;
      case 5:usVal=ucSec;break;
        
      case 10:usVal=ucHourStart1;break;
      case 11:usVal=ucMinStart1;break;
      case 12:usVal=ucDuration1;break;
      case 13:usVal=ucDaysWeek1;break;
      //case 14:usVal=cmds.m_pJardCmd->getPmpTimer_min(1);break;
      
      case 20:usVal=ucHourStart2;break;
      case 21:usVal=ucMinStart2;break;
      case 22:usVal=ucDuration2;break;
      case 23:usVal=ucDaysWeek2;break;
      //case 24:usVal=cmds.m_pJardCmd->getPmpTimer_min(2);break;

      default:return MDBUS_ERR;
    }
    mdbus_fill_register_data(o_pBuffer, i, usVal);
  }

  return MDBUS_OK;
}

int user_mdbus_write_holding_registers(unsigned short addr, unsigned short count, unsigned char* i_pBuffer)
{
  unsigned short usYear=0;
  unsigned char ucMonth=0;
  unsigned char ucDay=0;
  unsigned char ucHour=0;
  unsigned char ucMin=0;
  unsigned char ucSec=0;
  
  unsigned char ucHourStart1=0;
  unsigned char ucMinStart1=0;
  unsigned char ucDuration1=0;
  unsigned char ucDaysWeek1=0;

  unsigned char ucHourStart2=0;
  unsigned char ucMinStart2=0;
  unsigned char ucDuration2=0;
  unsigned char ucDaysWeek2=0;

  cmds.m_pJardCmd->aliveComm();
  
  cmds.m_pJardCmd->getDateTime(&usYear,&ucMonth,&ucDay,&ucHour,&ucMin,&ucSec);
  cmds.m_pJardCmd->getSheduler(1,&ucHourStart1,&ucMinStart1,&ucDuration1,&ucDaysWeek1);
  cmds.m_pJardCmd->getSheduler(2,&ucHourStart2,&ucMinStart2,&ucDuration2,&ucDaysWeek2);

  bool flgModifDtm=false;
  bool flgModifSch1=false;
  bool flgModifSch2=false;
  for (int i = 0; i < count; i++)
  {
    unsigned short usVal = mdbus_get_register_data(i_pBuffer, i);
    switch (addr+i)
    {
      case 0:usYear=usVal;flgModifDtm=true;break;
      case 1:ucMonth=(usVal&0xFF);flgModifDtm=true;break;
      case 2:ucDay=(usVal&0xFF);flgModifDtm=true;break;
      case 3:ucHour=(usVal&0xFF);flgModifDtm=true;break;
      case 4:ucMin=(usVal&0xFF);flgModifDtm=true;break;
      case 5:ucSec=(usVal&0xFF);flgModifDtm=true;break;

      case 10:ucHourStart1=(usVal&0xFF);flgModifSch1=true;break;
      case 11:ucMinStart1=(usVal&0xFF);flgModifSch1=true;break;
      case 12:ucDuration1=(usVal&0xFF);flgModifSch1=true;break;
      case 13:ucDaysWeek1=(usVal&0xFF);flgModifSch1=true;break;
      //case 14:cmds.m_pJardCmd->setPmpTimer_min(1,(usVal&0xFF));break;
          
      case 20:ucHourStart2=(usVal&0xFF);flgModifSch2=true;break;
      case 21:ucMinStart2=(usVal&0xFF);flgModifSch2=true;break;
      case 22:ucDuration2=(usVal&0xFF);flgModifSch2=true;break;
      case 23:ucDaysWeek2=(usVal&0xFF);flgModifSch2=true;break;
      //case 24:cmds.m_pJardCmd->setPmpTimer_min(2,(usVal&0xFF));break;

        case 200:
        {
          T_ID id;
          
          if (memoire_load_id(&id)==true)
          {
            id.bVersion=(usVal&0xFF);
            memoire_save_id(&id);
          }
            
          break;          
        }

        case 201:
        {
          T_ID id;
          
          if (memoire_load_id(&id)==true)
          {
            id.bSerial=(usVal&0xFF);
            memoire_save_id(&id);
          }
            
          break;          
        }

      default:return MDBUS_ERR;
    }
  }

  if (flgModifSch1)
  {
    cmds.m_pJardCmd->setSheduler(1,ucHourStart1,ucMinStart1,ucDuration1,ucDaysWeek1);
  }
  
  if (flgModifSch2)
  {
    cmds.m_pJardCmd->setSheduler(2,ucHourStart2,ucMinStart2,ucDuration2,ucDaysWeek2);
  }

  if (flgModifDtm)
  {
    cmds.m_pJardCmd->setDateTime(usYear,ucMonth,ucDay,ucHour,ucMin,ucSec);
  }

  return MDBUS_OK;
}

T_MDBUS_CTX ctx;
unsigned char md_buffer[150];
Cmds cmds;

Cmds::Cmds()
{
  m_pSerial = NULL;
}

void Cmds::init(HardwareSerial *pSerial, IJardCmd *pJardCmd)
{
  m_pSerial = pSerial;
  m_pJardCmd = pJardCmd;

  if ( (m_pSerial == NULL) && (m_pJardCmd != NULL) )
    return;

  m_pSerial->begin(115200);

  mdbus_init(&ctx,md_buffer, sizeof(md_buffer),4);  
  ctx.back=(void *)m_pSerial;
}

void Cmds::onSerialEvent()
{
  if (m_pSerial == NULL)
    return;

  while (m_pSerial->available() > 0) 
  {
    int incomingByte = m_pSerial->read();
    if ( (incomingByte>=0) && (incomingByte<=255) )
      mdbus_receive(&ctx, (unsigned char)incomingByte);
  } 
}
