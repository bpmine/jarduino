/*
 * IJardCmd.h
 */
#ifndef IJARDCMD_INCLUDED
#define IJARDCMD_INCLUDED

class IJardCmd
{
public:
  virtual void aliveComm(void);
  virtual void setDateTime(int year,int month,int day,int hour,int minute,int second)=0;  
  virtual void getDateTime(unsigned short *o_pusYear,unsigned char *o_pucMonth,unsigned char *o_pucDay,unsigned char *o_pucHour,unsigned char *o_pucMin,unsigned char *o_pucSec)=0;
  virtual void getDateStr(char *o_strDate,int i_maxSize)=0;

  virtual void setSheduler(int num,unsigned char ucStartHour,unsigned char ucStartMin,unsigned char ucDuration_min,unsigned char ucDaysOfWeek)=0;
  virtual void getSheduler(int num,unsigned char *o_ucStartHour,unsigned char *o_ucStartMin,unsigned char *o_ucDuration_min,unsigned char *o_ucDaysOfWeek)=0;

  virtual unsigned short getBattLevel(void)=0;
  virtual unsigned short getSunLevel(void)=0;
  virtual int getTemp(void)=0;
  virtual unsigned char getHum(void)=0;
  virtual unsigned short getTmrVeille_s(void);
  virtual unsigned short getTmrComm_s(void);
  virtual unsigned short getTmrPmp1_min(void);
  virtual unsigned short getTmrPmp2_min(void);

  virtual void load(void)=0;
};

#endif
