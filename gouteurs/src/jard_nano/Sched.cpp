#include "Sched.h"


void Sched::init()
{
  m_ucStartHour=0xFF;
  m_ucStartMin=0xFF;
  m_ucDuration=0xFF;
  m_ucDaysOfWeek=0xFF;  
}

Sched::Sched()
{
  init();
}

void Sched::setSettings(unsigned char ucStartHour,unsigned char ucStartMin,unsigned char ucDuration,unsigned char ucDaysOfWeek)
{
  if ( ucStartHour>23 )
  {
    init();
    return;
  }
  
  if ( ucStartMin>59 )
  {
    init();
    return;
  }

  m_ucStartHour=ucStartHour;
  m_ucStartMin=ucStartMin;
  m_ucDuration=ucDuration;
  m_ucDaysOfWeek=ucDaysOfWeek;
}

void Sched::getSettings(unsigned char *o_ucStartHour,unsigned char *o_ucStartMin,unsigned char *o_ucDuration,unsigned char *o_ucDaysOfWeek)
{
  *o_ucStartHour=m_ucStartHour;
  *o_ucStartMin=m_ucStartMin;
  *o_ucDuration=m_ucDuration;
  *o_ucDaysOfWeek=m_ucDaysOfWeek;
}

bool Sched::check(int hour,int minute,int doW)
{
  /// 0:Sunday ...  6:Saturday
  if ( (doW>6) || (doW<0) )
    return false;

  if ( (hour<0) || (hour>23) )
    return false;
  
  if ( (minute<0) || (minute>59) )
    return false;

  bool isDay=true;
  if ( m_ucDaysOfWeek!=0xFF )
  {  
    unsigned char ucMask=(unsigned char)(1<<doW);
    if ( (m_ucDaysOfWeek & ucMask) != ucMask )
      isDay=false;
  }

  unsigned long minsSinceMidnightStart=m_ucStartHour*60+m_ucStartMin;       ///< Debut du pompage (en mins depuis minuit)
  unsigned long minsSinceMidnightEnd=minsSinceMidnightStart+m_ucDuration;   ///< Fin du pompage (mins depuis minuit. Peut dépasser 24h)
  unsigned long minsSinceMidnight=hour*60+minute;                           ///< Heure courante (en mins depuis minuit)

  /// @remark On calcule éventuellement le nombre de mins apres minuit (si a cheval sur minuit)
  unsigned long minsSinceMidnightEndAfterMidnight=0;
  if (minsSinceMidnightEnd>23*60+59)
	  minsSinceMidnightEndAfterMidnight=minsSinceMidnightEnd-(24*60);

  if (isDay)
  {
    if (    ( (minsSinceMidnight>=minsSinceMidnightStart) && (minsSinceMidnight<=minsSinceMidnightEnd) )
         || ( (minsSinceMidnightEndAfterMidnight!=0) && (minsSinceMidnight<=minsSinceMidnightEndAfterMidnight) )
         )
    {
          return true;
    }  
  }
  
  return false;  
}
  
