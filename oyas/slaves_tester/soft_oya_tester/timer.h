#ifndef TIMER_HEADER_INCLUDED
#define TIMER_HEADER_INCLUDED

class Timer
{
  private:
    unsigned long tick0_ms;
    unsigned long duration_ms;   
    bool running; 
    bool single;
  
  public:
    Timer(unsigned long duration_ms=1000,bool single=true);

    bool tick(void);
    void setDuration_ms(unsigned long duration_ms); 
    void stop(void);
    void start(void);   

    bool isRunning(void);
    unsigned long getRemaining_ms(void);
    unsigned long getDuration_ms(void);
};
#endif
