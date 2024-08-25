#include <Wire.h>
#include <RTClib.h>

#include "jard.h"
#include "pins.h"
#include "memoire.h"
#include "autom.h"
#include "pump.h"

DS1307 rtc;
Pump pump1;
Pump pump2;

Jard::Jard():tmrBlink(TEMPS_BLINK_MS,false)
{  
  m_flgBlk=true;
  m_flgBlk2=true;
  m_flgBattOk=false;
  m_flgSunOk=false;
  m_ucMinsSun=0;
  m_ucOldMin=0xFF;
}

void Jard::init()
{ 
  Wire.begin();
  rtc.begin();

  load();

  tmrBlink.start();
  tmrVeille.start();
}

void Jard::load(void)
{
  T_ID id; 
  if (memoire_load_id(&id)==false)
  {
    id.bVersion=1;
    id.bSerial=1;
    memoire_save_id(&id);

    for (int i=MEM_START_ADDR_STATS;i<MEM_STATS_ADDR_MAX;i++)
    {
      memoire_stats_reset(i);
    }

    memoire_set(MEM_SETTINGS_ADDR_PUMP1_EN,0);
    memoire_set(MEM_SETTINGS_ADDR_PUMP1_AUTO,0);
    memoire_set(MEM_SETTINGS_ADDR_PUMP1_REMOTE,0);
    memoire_set(MEM_SETTINGS_ADDR_PUMP1_TIMER,60);
    
    memoire_set(MEM_SETTINGS_ADDR_PUMP2_EN,0);
    memoire_set(MEM_SETTINGS_ADDR_PUMP2_AUTO,0);
    memoire_set(MEM_SETTINGS_ADDR_PUMP2_REMOTE,0);
    memoire_set(MEM_SETTINGS_ADDR_PUMP2_TIMER,60);
  }

  unsigned char ucVal;
  if (memoire_read(MEM_SETTINGS_ADDR_PUMP1_EN,&ucVal)==true)
    mbs.fromBool(MB_PMP1_ENABLE,ucVal==1?true:false);
  if (memoire_read(MEM_SETTINGS_ADDR_PUMP1_AUTO,&ucVal)==true)
    mbs.fromBool(MB_PMP1_AUTO,ucVal==1?true:false);
  if (memoire_read(MEM_SETTINGS_ADDR_PUMP1_REMOTE,&ucVal)==true)
    pump1.setRemote(ucVal==1?true:false);
  if (memoire_read(MEM_SETTINGS_ADDR_PUMP1_TIMER,&ucVal)==true)
    pump1.setTimerDelay_min(ucVal);

  unsigned char ucHour=0xFF;
  unsigned char ucMin=0xFF;
  unsigned char ucDuration=0xFF;
  unsigned char ucDaysWeek=0xFF;
  memoire_read(MEM_SETTINGS_ADDR_PUMP1_STARTH1,&ucHour);
  memoire_read(MEM_SETTINGS_ADDR_PUMP1_STARTM1,&ucMin);
  memoire_read(MEM_SETTINGS_ADDR_PUMP1_DURAT1,&ucDuration);
  memoire_read(MEM_SETTINGS_ADDR_PUMP1_DAYSW,&ucDaysWeek);  
  pump1.setSched(ucHour,ucMin,ucDuration,ucDaysWeek);

  
  if (memoire_read(MEM_SETTINGS_ADDR_PUMP2_EN,&ucVal)==true)
    mbs.fromBool(MB_PMP2_ENABLE,ucVal==1?true:false);
  if (memoire_read(MEM_SETTINGS_ADDR_PUMP2_AUTO,&ucVal)==true)
    mbs.fromBool(MB_PMP2_AUTO,ucVal==1?true:false);
  if (memoire_read(MEM_SETTINGS_ADDR_PUMP2_REMOTE,&ucVal)==true)
    pump2.setRemote(ucVal==1?true:false);  
  if (memoire_read(MEM_SETTINGS_ADDR_PUMP2_TIMER,&ucVal)==true)
    pump2.setTimerDelay_min(ucVal);
  
  memoire_read(MEM_SETTINGS_ADDR_PUMP2_STARTH1,&ucHour);
  memoire_read(MEM_SETTINGS_ADDR_PUMP2_STARTM1,&ucMin);
  memoire_read(MEM_SETTINGS_ADDR_PUMP2_DURAT1,&ucDuration);
  memoire_read(MEM_SETTINGS_ADDR_PUMP2_DAYSW,&ucDaysWeek);
  pump2.setSched(ucHour,ucMin,ucDuration,ucDaysWeek);

  mbs.fromBool(MB_COMM_OK,false);
}

void Jard::aliveComm(void)
{
  tmrComm.start();  
  mbs.fromBool(MB_COMM_OK,true);
}

void Jard::setDateTime(int year,int month,int day,int hour,int minute,int second)
{
  DateTime now = rtc.now();
  DateTime newDte(year,month,day,hour,minute,second);
  rtc.adjust(newDte);  
}

void Jard::getDateStr(char *o_strDate,int i_maxSize)
{
  if (i_maxSize<20)
    return;
    
  char tmp[35];
  DateTime now = rtc.now();
  sprintf(o_strDate, "%02d/%02d/%04d %02d:%02d:%02d",now.day() , now.month(), now.year(), now.hour(), now.minute(), now.second());  
}

void Jard::getDateTime(unsigned short *o_pusYear,unsigned char *o_pucMonth,unsigned char *o_pucDay,unsigned char *o_pucHour,unsigned char *o_pucMin,unsigned char *o_pucSec)
{
    DateTime now = rtc.now();
    
    *o_pusYear=now.year();
    *o_pucMonth=now.month();
    *o_pucDay=now.day();
    
    *o_pucHour=now.hour();
    *o_pucMin=now.minute();
    *o_pucSec=now.second();
}

void Jard::getTimeToSchedule(unsigned char *o_pucHour,unsigned char *o_pucMin,unsigned char *o_pucSec,unsigned char *o_pucDoW)
{
  DateTime now = rtc.now();
  
  *o_pucHour=now.hour();
  *o_pucMin=now.minute();
  *o_pucSec=now.second();
  *o_pucDoW=now.dayOfWeek();
}

void Jard::setSheduler(int num,unsigned char ucStartHour,unsigned char ucStartMin,unsigned char ucDuration_min,unsigned char ucDaysOfWeek)
{
  if (num==1)
  {
    pump1.setSched(ucStartHour,ucStartMin,ucDuration_min,ucDaysOfWeek);
    
    memoire_set(MEM_SETTINGS_ADDR_PUMP1_STARTH1,ucStartHour);
    memoire_set(MEM_SETTINGS_ADDR_PUMP1_STARTM1,ucStartMin);
    memoire_set(MEM_SETTINGS_ADDR_PUMP1_DURAT1,ucDuration_min);
    memoire_set(MEM_SETTINGS_ADDR_PUMP1_DAYSW,ucDaysOfWeek);    
  }
  else if (num==2)
  {
    pump2.setSched(ucStartHour,ucStartMin,ucDuration_min,ucDaysOfWeek);
    
    memoire_set(MEM_SETTINGS_ADDR_PUMP2_STARTH1,ucStartHour);
    memoire_set(MEM_SETTINGS_ADDR_PUMP2_STARTM1,ucStartMin);
    memoire_set(MEM_SETTINGS_ADDR_PUMP2_DURAT1,ucDuration_min);
    memoire_set(MEM_SETTINGS_ADDR_PUMP2_DAYSW,ucDaysOfWeek);        
  }
}

void Jard::getSheduler(int num,unsigned char *o_ucStartHour,unsigned char *o_ucStartMin,unsigned char *o_ucDuration_min,unsigned char *o_ucDaysOfWeek)
{
  if (num==1)
  {
    pump1.getSched(o_ucStartHour,o_ucStartMin,o_ucDuration_min,o_ucDaysOfWeek);
  }  
  else if (num==2)
  {
    pump2.getSched(o_ucStartHour,o_ucStartMin,o_ucDuration_min,o_ucDaysOfWeek);
  }  
}

void Jard::setBatLevel(int i_level_dxv)
{
  m_batt_level_dxv=i_level_dxv;
}

void Jard::setSunLevel(int i_level_dxv)
{
  m_sun_level_dxv=i_level_dxv; 
}

void Jard::setTemp(int i_temp_dg)
{
  m_temp_deg=i_temp_dg; 
}

void Jard::setHum(int i_hum_dg)
{
  m_hum_pc=i_hum_dg;
}

unsigned short Jard::getBattLevel(void)
{
  return m_batt_level_dxv;  
}

unsigned short Jard::getSunLevel(void)
{
  return m_sun_level_dxv;
}

int Jard::getTemp(void)
{
  return m_temp_deg;
}

unsigned short Jard::getTmrVeille_s(void)
{
  return tmrVeille.getRemaining_ms()/(1000UL);
}

unsigned short Jard::getTmrComm_s(void)
{
  return tmrComm.getRemaining_ms()/(1000UL);
}

unsigned short Jard::getTmrPmp1_min(void)
{
  return pump1.getRemaining_ms()/(60000UL);
}

unsigned short Jard::getTmrPmp2_min(void)
{
  return pump2.getRemaining_ms()/(60000UL);
}


unsigned char Jard::getHum(void)
{
  return m_hum_pc; 
}

void Jard::loop(void)
{
	bool outLedPmp1=false;
	bool outLedPmp2=false;
	bool outCmdPmp1=false;
	bool outCmdPmp2=false;
	bool outCpu=false;
	bool outSun=false;
	bool outBatt=false;

	unsigned char ucHour=0;
	unsigned char ucMin=0;
	unsigned char ucSec=0;
  unsigned char DoW=0;

	getTimeToSchedule(&ucHour,&ucMin,&ucSec,&DoW);

	if (tmrBlink.tick()==true)
  {
		m_flgBlk=!m_flgBlk;
   
    if (m_flgBlk==true)
      m_flgBlk2=!m_flgBlk2;
  }

	if (m_batt_level_dxv>=SEUIL_LOW_BATT_H)
		m_flgBattOk=true;
	else if (m_batt_level_dxv<=SEUIL_LOW_BATT_L)
		m_flgBattOk=false;

	if (m_sun_level_dxv>=SEUIL_LOW_SUN_H)
		m_flgSunOk=true;
	else if (m_sun_level_dxv<=SEUIL_LOW_SUN_L)
		m_flgSunOk=false;

  bool on=!mbs_inputs.get(IB_BTN_ON); 
  bool veille=mbs.get(MB_VEILLE);   
	
	if (on==true)
	{    
		if ( veille==true )
		{
			if (    (mbs_inputs.getFalling(IB_BTN_PMP1))
				 || (mbs_inputs.getFalling(IB_BTN_PMP2))
				 || (mbs_inputs.getFalling(IB_BTN_ON))
				 )
			{
				tmrVeille.start();
			}
		}
		else
		{    
			if (mbs_inputs.getFalling(IB_BTN_PMP1))
			{
				pump1.startTimer();
				memoire_stats_inc(MEM_STATS_ADDR_TOT_BTN1);
			}

			if (mbs_inputs.getFalling(IB_BTN_PMP2))
			{
				pump2.startTimer();
				memoire_stats_inc(MEM_STATS_ADDR_TOT_BTN2);
			}
		}

		pump1.setForced(mbs.get(MB_PMP1_FORCED));
		pump1.setEnable(mbs.get(MB_PMP1_ENABLE));
		pump1.setAuto(mbs.get(MB_PMP1_AUTO));
		pump1.setRemote(mbs.get(MB_PMP1_REMOTE));

		pump2.setForced(mbs.get(MB_PMP2_FORCED));
		pump2.setEnable(mbs.get(MB_PMP2_ENABLE));
		pump2.setAuto(mbs.get(MB_PMP2_AUTO));
		pump2.setRemote(mbs.get(MB_PMP2_REMOTE));

		pump1.loop(ucHour,ucMin,DoW,m_flgBattOk,mbs.get(MB_PMP1_RM_CMD));
		pump2.loop(ucHour,ucMin,DoW,m_flgBattOk,mbs.get(MB_PMP2_RM_CMD));

		if ( (veille==true) && (m_flgSunOk==false) )
			outBatt=m_flgBattOk?false:m_flgBlk;
		else
			outBatt=m_flgBattOk?true:m_flgBlk;

		outSun=m_flgSunOk;

		outLedPmp1=pump1.getError()?m_flgBlk:pump1.getOut();
		outCmdPmp1=pump1.getOut();

		outLedPmp2=pump2.getError()?m_flgBlk:pump2.getOut();
		outCmdPmp2=pump2.getOut();
	}
	else
	{
		pump1.stopTimer();
		pump1.setEnable(false);
		pump1.setAuto(false);

		pump2.stopTimer();
		pump2.setEnable(false);
		pump2.setAuto(false);
		tmrVeille.start();
	}
  
  mbs.fromBool(MB_BATT_OK,m_flgBattOk);
  mbs.fromBool(MB_SUN_OK,m_flgSunOk);

	if (m_ucOldMin==0xFF)
		m_ucOldMin=ucMin;

	if (m_ucOldMin!=ucMin)
	{
		if (m_flgSunOk)
		{
			m_ucMinsSun++;
			if (m_ucMinsSun>=60)
			{
				m_ucMinsSun=0;
				memoire_stats_inc(MEM_STATS_ADDR_TOT_SUN_H);
			}
		}

		if (pump1.incMins()==true)
			memoire_stats_inc(MEM_STATS_ADDR_TOT_P1_H);

		if (pump2.incMins()==true)
			memoire_stats_inc(MEM_STATS_ADDR_TOT_P2_H);

		m_ucOldMin=ucMin;
	}

	/// @remark Au bout du timer de comm, on coupe les commandes remote et on reporte la comm NOK
	if (tmrComm.tick()==true)
	{
		mbs.fromBool(MB_COMM_OK,false);
		mbs.fromBool(MB_PMP1_RM_CMD,false);
		mbs.fromBool(MB_PMP2_RM_CMD,false);
	}  

	if ( (mbs_outputs.get(OB_CMD_PMP1)==true) || (mbs_outputs.get(OB_CMD_PMP2)==true) )
		tmrVeille.start();    

  bool outLedOn=on;
	if ( (veille==true) && (m_flgSunOk==false) )
	{
		outLedOn=m_flgBlk2 && m_flgBlk && outLedOn;
		outLedPmp1=false;
		outLedPmp2=false;
	}

  tmrVeille.tick();
  veille=(tmrVeille.isRunning()?false:true);
  mbs.fromBool(MB_VEILLE,veille);

  /// @remark Sleep bascule a true quand la veille arrive
  bool slp=mbs.get(MB_SLEEP); 
  slp=!on;
  //slp=(veille) || (!on) || ( (slp) && (!outCmdPmp1) && (!outCmdPmp2) && (!m_flgSunOk) && (!on) );  
  mbs.fromBool(MB_SLEEP,slp);

	mbs_outputs.start_latch();
	mbs_outputs.fromBool(OB_LED_CPU,outLedOn);
	mbs_outputs.fromBool(OB_LED_BATT,outBatt);
	mbs_outputs.fromBool(OB_LED_SUN,outSun);
	mbs_outputs.fromBool(OB_CMD_PMP1,outCmdPmp1);
	mbs_outputs.fromBool(OB_LED_PMP1,outLedPmp1);

	mbs_outputs.fromBool(OB_CMD_PMP2,outCmdPmp2);
	mbs_outputs.fromBool(OB_LED_PMP2,outLedPmp2);
	mbs_outputs.end_latch();
}
