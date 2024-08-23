#include <arduino.h>

class FlowMeter
{
  private:
    volatile byte m_pulseCount;
    float m_flowRate;    
    unsigned long m_totalMilliLitres;
    unsigned long m_oldTime;

    float m_calibrationFactor = 4.5;    
    
    void reset()
    {      
      m_pulseCount=0;
      m_flowRate=0.0;
      m_totalMilliLitres=0;
      m_oldTime=millis();      
    }
    
  public:
    FlowMeter()
    {
      reset();
    }

    void irqCounter()
    {
      m_pulseCount++;
    }

    void begin(int irq,void (*irqCounter)(void))
    {
      reset();
      attachInterrupt(irq, irqCounter, FALLING);
    }

    float getFlowRate_LpH(void)
    {
      return m_flowRate;
    }

    unsigned long getTotal_mL(void)
    {
      return m_totalMilliLitres;
    }

    unsigned long getTotal_L(void)
    {
      return m_totalMilliLitres/1000;
    }

    bool tick(void)
    {
      if((millis() - m_oldTime) > 1000)
      { 
        byte pulseCount=m_pulseCount;
        unsigned int flowMilliLitres; 
          
        m_flowRate = ((1000.0 / (millis() - m_oldTime)) * pulseCount) / m_calibrationFactor;
        flowMilliLitres = (m_flowRate / 60) * 1000;    
        m_totalMilliLitres += flowMilliLitres;

        m_pulseCount = 0;
        m_oldTime = millis();      
      }
    }
};
