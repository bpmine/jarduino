#ifndef SLEEPMGT_HEADER_INCLUDED
#define SLEEPMGT_HEADER_INCLUDED

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

volatile uint8_t g_ticks_1s=0;
volatile uint8_t g_wdt=0;


ISR (PCINT3_vect)
{
  PCIFR  |= bit (PCIF0);
}


class Sleep
{
  private:      
    static void init_timer1_50ms(void)
    {
      g_ticks_1s=0;
      //set timer1 interrupt at 1Hz
      TCCR1A = 0;// set entire TCCR1A register to 0
      TCCR1B = 0;// same for TCCR1B
      TCNT1  = 0;//initialize counter value to 0
      // set compare match register for 1hz increments
      //OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
      OCR1A = 781;// = (16*10^6) / (1*1024) - 1 (must be <65536)
      //OCR1A = 65535;
      // turn on CTC mode
      TCCR1B |= (1 << WGM12);  
      // Set CS10 and CS12 bits for 1024 prescaler
      TCCR1B |= (1 << CS12) | (1 << CS10);    
      // enable timer compare interrupt
      TIMSK1 |= (1 << OCIE1A);  
    }

    static void init_wdg(void)
    {
      //WDTCSR |= (1<<WDCE) | (1<<WDE);
      //wdt_enable(WDTO_2S); 
      //wdt_reset();
      //WDTCSR |= _BV(WDIE);
      
      /* Clear the reset flag. */
      MCUSR &= ~(1<<WDRF);
      
      /* In order to change WDE or the prescaler, we need to
       * set WDCE (This will allow updates for 4 clock cycles).
       */
      WDTCSR |= (1<<WDCE) | (1<<WDE);
    
      /* set new watchdog timeout prescaler value */
      WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
      
      /* Enable the WD interrupt (note no reset). */
      WDTCSR |= _BV(WDIE);
    }

    static init_pin_change(void)
    {
      attachInterrupt (digitalPinToInterrupt(11), PCINT3_vect, FALLING);
      PCIFR  |= bit (PCIF0);
      PCICR  |= bit (PCIE0);
      PCMSK0 |= bit (PCINT3);
    }

  public:
    static void init(void)
    {
      init_timer1_50ms();
      init_wdg();
      init_pin_change();
    }

    static void enter_idle_sleep(void)
    {
      set_sleep_mode(SLEEP_MODE_IDLE);
      sleep_enable(); 
      TCNT1  = 0;
      sleep_cpu (); 
    }    

    static void enter_big_sleep(void)
    {
      set_sleep_mode(SLEEP_MODE_PWR_SAVE);
      sleep_enable(); 
      sleep_mode (); 
      sleep_disable();
      power_all_enable();
    }    

    static void tick(void)
    {
      wdt_reset();
    }
};

ISR(TIMER1_COMPA_vect)
{
  g_ticks_1s++;
}

ISR(WDT_vect)
{
  if(g_wdt == 0)
  {
    g_wdt=1;
  }
}

#endif
