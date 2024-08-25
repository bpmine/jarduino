#ifndef SCHED_HEADER_INCLUDED
#define SCHED_HEADER_INCLUDED

class Sched
{
  private:
    unsigned char m_ucStartHour;
    unsigned char m_ucStartMin;
    unsigned char m_ucDuration;
    unsigned char m_ucDaysOfWeek;

    void init();
  
  public:
    Sched();

    void setSettings(unsigned char ucStartHour,unsigned char ucStartMin,unsigned char ucDuration,unsigned char ucDaysOfWeek);
    void getSettings(unsigned char *o_ucStartHour,unsigned char *o_ucStartMin,unsigned char *o_ucDuration,unsigned char *o_ucDaysOfWeek);
    bool check(int hour,int minute,int doW);
};

#endif
