/**
 * @file jard_slave_rs485.ino
 * @brief Programme d'un esclave RS485 de gestion d'un OYA (ou d'une pompe)
*/
#include "client.h"
#include "globals.h"
#include "eep_slave.hpp" .
#include "slavearduino.hpp"
#include "timer.h"
#include "pins.h"

#include <avr/wdt.h>

//#define DEBUG_TRACE
//#define INIT_AND_SET_ADDR
//#define ENABLE_WDG

//#define NO_FLOW
//#define NO_ANALOG

#ifdef INIT_AND_SET_ADDR
  //#define INIT_ADDR 2
  #define INIT_ADDR 'Z'
  #define DEBUG_TRACE
#endif

bool g_on=false;
bool g_cpt_low=false;
bool g_cpt_high=false;
unsigned char g_mes_cv;
bool g_reset_because_of_wdg=false;

int g_flow_mLpMin=-1;
char g_temp_dg=0;
char g_hum_pc=0;
unsigned short g_total_s=0;
unsigned short g_errors=0;

#ifndef NO_FLOW
  #include "flow.hpp" 
#endif

#ifndef NO_ANALOG
  #include "analog.hpp" 
  Analog anMesV;
#endif

Timer tmrSec(1000,false);
Timer tmr500ms(500,false);
Timer tmr200ms(200,false);

bool flg200ms=false;
bool flg500ms=false;

unsigned char getSlaveAddress(void)
{
  unsigned char addr=0;
  if (digitalRead(PIN_ADDR_A1)==LOW)
   addr|=0x01;
  if (digitalRead(PIN_ADDR_A2)==LOW)
   addr|=0x02;
  if (digitalRead(PIN_ADDR_A3)==LOW)
   addr|=0x04;
  if (digitalRead(PIN_ADDR_A4)==LOW)
   addr|=0x08;

  return addr;
}

void setup() 
{
  #ifdef ENABLE_WDG 
    unsigned char mcusr=MCUSR;
    if (MCUSR & (1 << WDRF))
      g_reset_because_of_wdg=true;
      
    MCUSR = 0;
  #endif

  wdt_disable();
  
  pinMode(PIN_CMD_EV,OUTPUT);
  digitalWrite(PIN_CMD_EV,LOW);
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,LOW);

  /// @warning Les deux entrées LOW/HIGH n'ont pas de pullup externe sur les cartes Rev Ex. Donc il les faut en interne
  pinMode(PIN_CPT_LVL_LOW,INPUT_PULLUP);
  pinMode(PIN_CPT_LVL_HIGH,INPUT_PULLUP);
  pinMode(PIN_CPT_FLOW,INPUT);

  pinMode(PIN_MES_V,INPUT);

  pinMode(PIN_ADDR_A1,INPUT_PULLUP);
  pinMode(PIN_ADDR_A2,INPUT_PULLUP);
  pinMode(PIN_ADDR_A3,INPUT_PULLUP);
  pinMode(PIN_ADDR_A4,INPUT_PULLUP);

  pinMode(PIN_TX_EN,OUTPUT);
  digitalWrite(PIN_TX_EN,LOW);

  pinMode(PIN_DGB_SYNC,OUTPUT);
  digitalWrite(PIN_DGB_SYNC,LOW);

  pinMode(PIN_DGB_CMD,OUTPUT);
  digitalWrite(PIN_DGB_CMD,LOW);

  /// @remark On clignote 5 fois rapidement au boot
  for (int i=0;i<5;i++)
  {
    delay(100);
    digitalWrite(PIN_LED,HIGH);
    delay(100);
    digitalWrite(PIN_LED,LOW);
  }  
  digitalWrite(PIN_LED,LOW);
  tmrSec.start();

  g_cpt_low=false;
  g_cpt_high=false;
  g_on=false;
  g_flow_mLpMin=-1;
  
  g_total_s=0;
  g_errors=0;

  #ifndef NO_FLOW
    Flow.begin(PIN_CPT_FLOW);
  #endif

  #ifdef INIT_AND_SET_ADDR
    unsigned char tmp;
    Eep::readID(&tmp); /// Pour etre sur EEP bien init
    Eep::writeID(INIT_ADDR);
  #endif

  unsigned char addr=getSlaveAddress();
  unsigned char laddr=0xFF;
  do
  {
    delay(5);
    laddr=addr;
    addr=getSlaveAddress();    
  } while (addr!=laddr);
  
  if (addr==0)
  {
    /// Si le selecteur d'adresse est sur '0' (ou non monte), on lit l'adresse en EEPROM
    Eep::readID(&addr);
  }

  if (addr>=15)
    addr='Z';

  Slave.begin(addr);

  g_total_s=Eep::readTotalTimeCtr();
  g_errors=Eep::readTotalErrCtr();

  #ifdef DEBUG_TRACE
    Serial.print("Boot @");
    if (addr=='Z')
      Serial.println("Z");
    else
      Serial.println(addr);

    for (int i=0;i<500;i++)
    {
      delay(1);
      yield();
    }
    
    Serial.print("Total: ");
    Serial.print(g_total_s);
    Serial.println(" s");
    for (int i=0;i<500;i++)
    {
      delay(1);
      yield();
    }

    Serial.print("Errs: ");
    Serial.println(g_errors);
    for (int i=0;i<500;i++)
    {
      delay(1);
      yield();
    }
  #endif

  #ifdef INIT_AND_SET_ADDR
    Serial.print("FORCE ADDRESS INIT: ");
    Serial.println(INIT_ADDR);
    for (int i=0;i<500;i++)
    {
      delay(1);
      yield();
    }
  #endif 

  /// @remark Attendre au moins 1s apres le clignotement de boot
  while (tmrSec.tick()==false);
  
  /// @remark Si adresse Z, on bloque tout en clignotant lentement (1s)
  if (addr=='Z')
  {
    bool blk=false;
    while (1)
    {
      if (tmrSec.tick()==true)
        blk=!blk;
        
      digitalWrite(PIN_LED,blk?HIGH:LOW);      
      
      yield();
    }
  }

  #ifdef ENABLE_WDG
    cli();
    wdt_reset();
    wdt_enable(WDTO_500MS);
    sei();

    if (g_reset_because_of_wdg==true)
    {
      g_errors++;
      Eep::writeTotalErrCtr(g_errors);      
    }
  #endif

  flg200ms=false;
  flg500ms=false;

  //tmrSec.start();
  tmr500ms.start();
  tmr200ms.start();  
}

void serialEvent()
{
  Slave.onSerialEvent();
}

void loop() 
{
  #ifdef INIT_AND_SET_ADDR
    /// @remark S'allume brievement toutes les secondes si soft d'initialisation
    digitalWrite(PIN_LED,LOW);    
    delay(200);
    digitalWrite(PIN_LED,HIGH);
    delay(1000);
    return;
  #endif

  /// @remark Au repos, le niveau doit être high (capteur non branché)
  #ifndef NO_FLOW
    g_flow_mLpMin=Flow.getFlow();
  #endif
  g_cpt_low=digitalRead(PIN_CPT_LVL_LOW)==HIGH?true:false;
  g_cpt_high=digitalRead(PIN_CPT_LVL_HIGH)==HIGH?true:false;
  #ifndef NO_ANALOG
    anMesV.latch((unsigned short)analogRead(PIN_MES_V));  
  #endif  

  /// @remark Gestion de la comm de l'esclave. true a chaque fin de cycle / trame "S"
  if (Slave.loop()==true)
  {
     digitalWrite(PIN_DGB_SYNC,HIGH);

     g_hum_pc=-1;
     g_temp_dg=-127;
 
     #ifndef NO_ANALOG
  	   unsigned long v=(unsigned long)anMesV.get();    
       v=v*120/669;
       if (v>255)
        v=255;
       g_mes_cv=(unsigned char)v;
     #endif
     
     digitalWrite(PIN_DGB_SYNC,LOW);
  }  
 
  if (!Slave.isAlive())
  {
	  g_on=false;
  }
  else
  {
	  g_on=Slave.isCmd();
  }

  #ifndef NO_FLOW
    Flow.tick();
  #endif
  
  if (tmrSec.tick()==true)
  {    
    if (g_on==true)
    {
      g_total_s++;
      Eep::writeTotalTimeCtr(g_total_s);
    }
  }
  
  if (tmr200ms.tick()==true)
  {
    flg200ms=!flg200ms;
  }

  if (tmr500ms.tick()==true)
  {
    flg500ms=!flg500ms;
  }

  digitalWrite(PIN_CMD_EV,g_on==true?HIGH:LOW);

  /// @remark LED clignote tres vite si ON, moins vite si pas de Synchro et fixe si synchro ok
  if (Slave.isAlive()==true)
  {
    if (g_on==true)
    {
      digitalWrite(PIN_LED,flg200ms?HIGH:LOW);
    }
    else if (Slave.isSyncOk()==false)
    {
      digitalWrite(PIN_LED,flg500ms?HIGH:LOW);
    }
    else
    {
      digitalWrite(PIN_LED,HIGH);
    }
  }
  else
  {
    /// @remark LED eteinte si pas de reseau (cmd)    
    digitalWrite(PIN_LED,LOW);
  }

  #ifdef ENABLE_WDG
    wdt_reset();
  #endif
}

void reset_time(void)
{
  g_total_s=0;
  Eep::writeTotalTimeCtr(g_total_s);
}

void reset_errs(void)
{
  g_errors=0;
  Eep::writeTotalErrCtr(g_errors);
}
