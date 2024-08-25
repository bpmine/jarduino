/*
 * Jard.h
 */
#ifndef JARD_INCLUDED
#define JARD_INCLUDED

#include "IJardCmd.h"
#include "timer.h"

#define TEMPS_TIMER_MS    		(60*60*1000UL)   	///< Une heure d'arrosage si timer presse
#define TEMPS_BLINK_MS    		(800UL)
#define TEMPS_AVANT_VEILLE_MS	(60*1000UL)			///< Entree en veille au bout d'une minute
#define TEMPS_COMM_KO_MS		  (5*1000UL)			///< Temps avant de detecter la perte de la comm

#define SEUIL_LOW_BATT_L  (98)
#define SEUIL_LOW_BATT_H  (108)
#define SEUIL_LOW_SUN_L   (138)
#define SEUIL_LOW_SUN_H   (142)

class Jard: public IJardCmd
{
  private:
	  Timer tmrVeille=Timer(TEMPS_AVANT_VEILLE_MS);
    Timer tmrComm=Timer(TEMPS_COMM_KO_MS);
    Timer tmrBlink;
    bool m_flgBlk;
    bool m_flgBlk2;
  
    int m_batt_level_dxv;
    int m_sun_level_dxv;
    int m_temp_deg;
    int m_hum_pc;

    bool m_flgBattOk;
    bool m_flgSunOk;

    unsigned char m_ucOldMin;
    unsigned char m_ucMinsSun;

    void aliveComm(void);
    void setDateTime(int year,int month,int day,int hour,int minute,int second);
    void getDateTime(unsigned short *o_pusYear,unsigned char *o_pucMonth,unsigned char *o_pucDay,unsigned char *o_pucHour,unsigned char *o_pucMin,unsigned char *o_pucSec);
    void getTimeToSchedule(unsigned char *o_pucHour,unsigned char *o_pucMin,unsigned char *o_pucSec,unsigned char *o_pucDoW);
    void setSheduler(int num,unsigned char ucStartHour,unsigned char ucStartMin,unsigned char ucDuration_min,unsigned char ucDaysOfWeek);
    void getSheduler(int num,unsigned char *o_ucStartHour,unsigned char *o_ucStartMin,unsigned char *o_ucDuration_min,unsigned char *o_ucDaysOfWeek); 

    unsigned short getBattLevel(void);
    unsigned short getSunLevel(void);
    int getTemp(void);
    unsigned char getHum(void);
    
    unsigned short getTmrVeille_s(void);
    unsigned short getTmrComm_s(void);
    unsigned short getTmrPmp1_min(void);
    unsigned short getTmrPmp2_min(void);
    
    void load(void);
  
  public:
    Jard();

    void init();

    void setBatLevel(int i_level_dxv);
    void setSunLevel(int i_level_dxv);
    void setTemp(int i_temp_dg);
    void setHum(int i_hum_dg);

    void getDateStr(char *o_strDate,int i_maxSize);
    

    void loop(void);
};
#endif
