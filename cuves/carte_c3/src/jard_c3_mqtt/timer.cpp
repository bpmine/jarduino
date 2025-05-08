/**
 * @file timer.cpp
 * @brief IMPLEMENTATION - Gestion d'un timer en ms
*/
#include "timer.h"
#include <arduino.h>

Timer::Timer(unsigned long duration_ms,bool single)
{
  this->tick0_ms=millis();
  this->duration_ms=duration_ms;  
  this->running=false;
  this->single=single;
}

unsigned long Timer::getRemaining_ms(void)
{
    unsigned long t=millis();
    unsigned long delta_ms=0;

    if (t>=tick0_ms)
      delta_ms=t-tick0_ms;
    else
      delta_ms=0xFFFFFFFF-tick0_ms+t;

    if ( (delta_ms>=duration_ms) || (running==false) )
      return 0;
    else
      return duration_ms-delta_ms;
}

unsigned long Timer::getDuration_ms(void)
{
  return duration_ms;
}

bool Timer::tick(void)
{
  unsigned long remain_ms=getRemaining_ms();

  if (running==true)
  {
    if (remain_ms==0)
    {
      if (single==true)
      {
        running=false;
      }

      this->tick0_ms=millis();
        
      return true;
    }
  }
    
  return false;    
}

void Timer::setDuration_ms(unsigned long duration_ms)
{
  this->duration_ms=duration_ms;
}

bool Timer::isRunning(void)
{
  return running;
}

void Timer::stop(void)
{
  this->running=false;
}

void Timer::start(void)
{
  this->running=true;
  this->tick0_ms=millis();
}
