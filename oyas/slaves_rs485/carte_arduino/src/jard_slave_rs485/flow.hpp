#ifndef FLOW_HEADER_INCLUDED
#define FLOW_HEADER_INCLUDED

static volatile int _count;
void isr_flow(void)
{
  _count++;
}

class Flow
{
  private:
    int m_pin;    
    unsigned long m_t0;
    int m_flow_mLpMin;
   
  public:
    Flow(void)
    {
      m_pin=0;
      _count=0;
      m_t0=0;
      m_flow_mLpMin=-1;
    }

    void begin(int pin)
    {      
      m_pin=pin;
      _count=0;
      m_t0=micros();
      m_flow_mLpMin=-1;
      attachInterrupt(0, isr_flow, RISING);      
    }

    void tick(void)
    {      
      unsigned long t=micros();
      unsigned long ulDelta_us=0;
      if (t>=m_t0)
        ulDelta_us=t-m_t0;
      else
        ulDelta_us=0xFFFFFFFF-m_t0+t;

      if (ulDelta_us>=1000000)
      {
        int count=_count;
        double flow=(double)count*1000000/ulDelta_us;
        //flow=flow/7.5;
        m_flow_mLpMin=trunc(flow);
        _count=0;
        m_t0=micros();        
      }
    }

    int getFlow(void)
    {
      return m_flow_mLpMin;
    }  
};

Flow Flow;

#endif
