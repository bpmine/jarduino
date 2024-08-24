/*
 * pump.h
 */
#ifndef PUMP_INCLUDED
#define PUMP_INCLUDED

#include "timer.h"
#include "sched.h"

class IJardCmd;
class Pump
{
  private:
    Timer m_timer;
    Sched m_sched;

    bool m_enabled;
    bool m_auto;
    bool m_remote;
        
    bool m_forced;
    bool m_rmt_cmd;

    bool m_cmd;
    bool m_out;
    unsigned char m_mins;

  public:   
    Pump();

    void loop(int hour,int min,int DoW,bool batt_ok,bool cmd_remote);

    void setEnable(bool flgEnabled);
    void setForced(bool flgForced);
    void setAuto(bool flgAuto);
    void setRemote(bool flgRemote);
    void setSched(unsigned char startHour,unsigned char startMin,unsigned char ucDuration_min,unsigned char ucDaysOfWeek);
    void getSched(unsigned char *o_startHour,unsigned char *o_startMin,unsigned char *o_ucDuration_min,unsigned char *o_ucDaysOfWeek);

    void startTimer(void);
    void stopTimer(void);

    bool isEnabled(void);
    bool isAuto(void);

    bool getError(void);
    bool getOut(void);
    bool incMins(void);

    unsigned long getRemaining_ms(void);
    unsigned char getTimerDelay_min(void);
    void setTimerDelay_min(unsigned char ucDelay_min);
};
#endif
