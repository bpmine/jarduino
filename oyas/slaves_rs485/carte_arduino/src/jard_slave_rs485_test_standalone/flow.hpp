#ifndef FLOW_HEADER_INCLUDED
#define FLOW_HEADER_INCLUDED

static volatile unsigned long _count=0;

ISR(PCINT2_vect) 
{
  if (bitRead(PIND, 4))
    _count++;
}

class Flow
{
  private:
    unsigned long m_t0;
    int m_flow_LpH;
    int m_freq_hz;
   
  public:
    Flow(void)
    {
      _count=0;
      m_t0=0;
      m_flow_LpH=0;
      m_freq_hz=0;
    }

    void begin(void)
    {      
      _count=0;
      m_t0=micros();
      m_flow_LpH=0;
      m_freq_hz=0;

      pinMode(4,INPUT);
      PCICR |= (1 << PCIE2); // Activation de l'interruption sur le groupe PCINT23-16
      PCMSK2 |= (1 << PCINT20); // Activation de l'interruption sur PCINT20 (correspondant à D4)}      
    }

    void start(void)
    {
      _count=0;
      m_t0=micros();      
    }

    void tick(void)
    {      
      unsigned long t=micros();
      unsigned long ulDelta_us=0;
      
      if (t>=m_t0)
        ulDelta_us=t-m_t0;
      else
        ulDelta_us=0xFFFFFFFF-m_t0+t;

      if (ulDelta_us>=2000000)
      {        
        PCMSK2 &= ~(1 << PCINT20);
        unsigned long latched_count=_count;        
       
        double tmp=(double)latched_count*1000000/ulDelta_us;
        m_freq_hz=trunc(tmp);
        tmp=7.9576*tmp-25.1405;
        if (tmp<0)
          tmp=0;
        if (tmp>1000)
          tmp=1000;
          
        m_flow_LpH=int(tmp);
        
        m_t0=micros();
        _count=0;
        PCMSK2 |= (1 << PCINT20);
      }
    }

    int getFlow(void)
    {
      return m_flow_LpH;
    }  

    int getFreq(void)
    {
      return m_freq_hz;
    }  
};

Flow Flow;

#endif
