#ifndef STATES_HEADER_INCLUDED
#define STATES_HEADER_INCLUDED

#include "timer.h"

class StateMachine;
class State
{
protected:
   StateMachine* machine;

public:
   State(StateMachine* machine) : machine(machine) {}
   virtual ~State(void) {}
   virtual void onEnter() {}
   virtual void onRun() {}
   virtual void onLeave() {}
   virtual void onTimeout() {}
};

class StateMachine 
{
private:
   State* current_state;
   Timer tmrTimeout;

public:
   StateMachine() : current_state(nullptr) {}

   void setState(State *pState)
   {
      if (current_state) current_state->onLeave();
     
      current_state = pState;
      if (current_state) current_state->onEnter();
   }

   void run() 
   {
     if (tmrTimeout.tick()==true)
     {
       timeout();
     }
     else
     {
       if (current_state) current_state->onRun();
     }
   }

   void timeout()
   {
      if (current_state) current_state->onTimeout();
   }

   void startTimeOut(unsigned long ulTimeout_ms)
   {
     tmrTimeout.setDuration_ms(ulTimeout_ms);
     tmrTimeout.start();
   }

   void stopTimeOut(void)
   {
     tmrTimeout.stop();
   }
};

#endif
