#include "timer.h"
#include "cmds.h"
#include "jard.h"
#include "pins.h"
#include "autom.h"
#include "memoire.h"
#include "sleepmgt.hpp">

#include <DHT.h>

Jard jard;
DHT dhtSensor(PIN_DHT_DATA,DHT22);

void latch_input(int pin,int ob)
{
  if (digitalRead(pin)==HIGH)
    mbs_inputs.set(ob);
  else
    mbs_inputs.reset(ob);
}

void latch_inputs(void)
{
  mbs_inputs.start_latch();

  latch_input(PIN_BTN_PMP1,IB_BTN_PMP1);
  latch_input(PIN_BTN_PMP2,IB_BTN_PMP2);
  latch_input(PIN_BTN_ONOFF,IB_BTN_ON);
  
  mbs_inputs.end_latch();
}

void apply_output(int pin,int ob)
{
    digitalWrite(pin,mbs_outputs.get(ob)==true?HIGH:LOW);  
}
void apply_outputs(void)
{  
  apply_output(PIN_CMD_PMP1,OB_CMD_PMP1);
  apply_output(PIN_CMD_PMP2,OB_CMD_PMP2);
  apply_output(PIN_LED_PMP1,OB_LED_PMP1);
  apply_output(PIN_LED_PMP2,OB_LED_PMP2);
  apply_output(PIN_LED_BATT,OB_LED_BATT);
  apply_output(PIN_LED_SUN,OB_LED_SUN);
  apply_output(PIN_LED_CPU,OB_LED_CPU);  
}

unsigned long getElapsedTimeFrom_ms(unsigned long tick0_ms)
{
  unsigned long delta_ms=0;
  unsigned long t=millis();

  if (t>=tick0_ms)
  {
    delta_ms=t-tick0_ms;
  }
  else
  {
    delta_ms=0xFFFFFFFF-tick0_ms+t;    
  }

  return delta_ms;  
}

void serialEvent()
{
  cmds.onSerialEvent();
}

void setup()
{
  pinMode(PIN_CMD_PMP1,OUTPUT);
  digitalWrite(PIN_CMD_PMP1,LOW);
  pinMode(PIN_CMD_PMP2,OUTPUT);
  digitalWrite(PIN_CMD_PMP2,LOW);
  
  pinMode(PIN_LED_PMP1,OUTPUT);
  digitalWrite(PIN_LED_PMP1,HIGH);
  pinMode(PIN_LED_PMP2,OUTPUT);
  digitalWrite(PIN_LED_PMP2,HIGH);

  pinMode(PIN_LED_BATT,OUTPUT);
  digitalWrite(PIN_LED_BATT,HIGH);
  pinMode(PIN_LED_SUN,OUTPUT);
  digitalWrite(PIN_LED_SUN,HIGH);
  pinMode(PIN_LED_CPU,OUTPUT);
  digitalWrite(PIN_LED_CPU,HIGH);
  
  pinMode(PIN_BTN_PMP1,INPUT);
  pinMode(PIN_BTN_PMP2,INPUT);
  pinMode(PIN_BTN_ONOFF,INPUT);

  pinMode(PIN_DHT_DATA,INPUT_PULLUP);  

  pinMode(PIN_MES_POW_SUN,INPUT);
  pinMode(PIN_MES_POW_BATT,INPUT);
  
  digitalWrite(PIN_LED_PMP1,LOW);
  digitalWrite(PIN_LED_PMP2,LOW);
  digitalWrite(PIN_LED_BATT,LOW);
  digitalWrite(PIN_LED_SUN,LOW);
  digitalWrite(PIN_LED_CPU,LOW);

  Sleep::init();  

  dhtSensor.begin();

  init_autom();
  
  jard.init();
  cmds.init(&Serial,&jard);

  memoire_stats_inc(MEM_STATS_ADDR_TOT_BOOTS);
}

Analog anBatt(4);
Analog anSun(4);
void read_analog()
{
  unsigned short tmp=analogRead(PIN_MES_POW_BATT);
  unsigned short val=(unsigned short)((tmp*175L)/1023L);  
  anBatt.latch(val);

  tmp=analogRead(PIN_MES_POW_SUN);
  val=(unsigned short)((tmp*175L)/1023L);
  anSun.latch(val);
}

Analog anHum(4);
Analog anTemp(4);
void read_dht(void)
{
  float h = dhtSensor.readHumidity();
  if ( isnan(h) )
  {    
    anHum.reinit();
  }  
  else
  {
    anHum.latch(h);    
  }
  
  float t = dhtSensor.readTemperature();
  if ( isnan(t) )
  {    
    anTemp.reinit();
  }  
  else
  {
    anTemp.latch(t);
  }  
}


void loop()
{   
  latch_inputs();
  read_analog();
  read_dht();
    
  jard.setBatLevel(anBatt.get());
  jard.setSunLevel(anSun.get());
  jard.setTemp(anTemp.get());
  jard.setHum(anHum.get());
    
  mbs.start_latch();
  jard.loop();
  mbs.end_latch();
    
  apply_outputs();

  if ( (mbs.get(MB_SLEEP)==false) )
  {
    Sleep::enter_idle_sleep();
  }
  else
  {    
    Sleep::enter_big_sleep();
  }

  Sleep::tick();
}
