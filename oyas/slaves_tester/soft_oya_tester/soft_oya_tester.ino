/**
 * @file soft_oya_tester.ino
 * 
 * @brief Testeur d'Oyas
 * 
 * Ce programme se comporte comme un maître RS485
 *   - Il envoie périodiquement:
 *      - Une trame de commande avec l'adresse de l'esclave sélectionné
 *      - une trame de sync (adresse 'S')
 *      
 *   - Lorsque le bouton FIRE est presse, alors la commande déclenche l'EV de l'OYA/POMPE correspondant à l'adresse sélectionnée
 *   - Les deux LEDs indiquent le niveau ou le débit selon si l'esclave a l'adresse 1 (pompe) ou non
 *   - Le bouton ADDRESS permet de changer l'adresse sélectionnée
*/
#include <EEPROM.h>

#include "timer.h"
#include "prot.h"
#include "frames.hpp"
#include "framebuilder.h"

#define SEG_A     (8)  /// Pin 8
#define SEG_B     (9)  /// Pin 9
#define SEG_C     (3)  /// Pin 3
#define SEG_D     (4)  /// Pin 4
#define SEG_E     (5)  /// Pin 5
#define SEG_F     (7)  /// Pin 7
#define SEG_G     (6)  /// Pin 6
#define SEG_DP    (2)  /// Pin 2

#define FIRE      (A5)
#define ADDRESS   (11)

#define LD_LEFT   (A1)
#define LD_RIGHT  (A2)

#define TX_EN     (10)

#define NB_SEGS (8)
const int list_segs[NB_SEGS]=
{
  SEG_A,
  SEG_B,
  SEG_C,
  SEG_D,
  SEG_E,
  SEG_F,
  SEG_G,
  SEG_DP
};

#define BIT_A   (0x01)
#define BIT_B   (0x02)
#define BIT_C   (0x04)
#define BIT_D   (0x08)
#define BIT_E   (0x10)
#define BIT_F   (0x20)
#define BIT_G   (0x40)
#define BIT_DP  (0x80)

unsigned char g_addr=1;
unsigned long t0_fire_ms=0;
bool g_on=false;

FrameBuilder builder;
Timer tmrCycle(100UL,false);
Timer tmrInput(250UL,false);
Timer tmrNoComm(5000UL);

void display(char val,bool dp)
{
  unsigned char ret=0;
  
  switch (val)
  {
    case '0':ret=0x3F;break;
    case '1':ret=BIT_B|BIT_C;break;
    case '2':ret=BIT_A|BIT_B|BIT_G|BIT_E|BIT_D;break;
    case '3':ret=BIT_A|BIT_B|BIT_C|BIT_D|BIT_G;break;
    case '4':ret=BIT_B|BIT_C|BIT_F|BIT_G;break;
    case '5':ret=BIT_A|BIT_F|BIT_G|BIT_C|BIT_D;break;
    case '6':ret=BIT_A|BIT_C|BIT_D|BIT_E|BIT_F|BIT_G;break;
    case '7':ret=BIT_A|BIT_B|BIT_C;break;
    case '8':ret=0x7F;break;
    case '9':ret=BIT_A|BIT_B|BIT_C|BIT_D|BIT_F|BIT_G;break;
    case 'A':ret=BIT_A|BIT_B|BIT_C|BIT_E|BIT_F|BIT_G;break;    
    case 'B':ret=BIT_C|BIT_D|BIT_E|BIT_F|BIT_G;break;
    case 'C':ret=BIT_A|BIT_D|BIT_E|BIT_F;break;
    case 'D':ret=BIT_B|BIT_C|BIT_D|BIT_E|BIT_G;break;
    case 'E':ret=BIT_A|BIT_D|BIT_E|BIT_F|BIT_G;break;
    case 'F':ret=BIT_A|BIT_E|BIT_F|BIT_G;break;
    default:ret=0;break;
  }

  if (dp)
    ret|=BIT_DP;

  for (int i=0;i<NB_SEGS;i++)
  {
    if ( ( ret & (1<<i) ) == (1<<i) )
    {
      digitalWrite(list_segs[i],HIGH);
    }
    else
    {
      digitalWrite(list_segs[i],LOW);
    }
  }
}

void display_hex(unsigned char val,bool dp)
{
  if ( ( val>0) && (val<=15) )
  {
    char c;
    
    if (val<10)
      c='0'+val;
    else
      c='A'+val-10;

    display(c,dp);
  }
  else
  {
    display(0,dp);
  }
}

void setup() 
{
  for (int i=0;i<NB_SEGS;i++)
  {
    pinMode(list_segs[i],OUTPUT);
    digitalWrite(list_segs[i],LOW);
  }

  t0_fire_ms=0;
  g_addr=EEPROM.read(0);
  if ( (g_addr<1) || (g_addr>15) )
    g_addr=1;
  
  pinMode(ADDRESS,INPUT_PULLUP);
  pinMode(FIRE,INPUT_PULLUP);
    
  pinMode(LD_RIGHT,OUTPUT);
  digitalWrite(LD_RIGHT,LOW);
  
  pinMode(LD_LEFT,OUTPUT);
  digitalWrite(LD_LEFT,LOW);
  
  pinMode(TX_EN,OUTPUT);
  digitalWrite(TX_EN,LOW);
  

  display(0,false);  
  
  g_on=false;  

  tmrCycle.start();
  tmrInput.start();
  tmrNoComm.start();
}

int g_sendSync=false;

void sendCommandFrame(unsigned char addr)
{
  unsigned short cmds=0;
  if (g_on==true)
  {
    cmds|=(0x01<<g_addr);
  }
  
  FrameCmd cmd(cmds,addr);
  unsigned char *pMsg=builder.build(&cmd);

  digitalWrite(TX_EN,HIGH);
  delay(2);
  Serial.write(pMsg,builder.size());
  delay(1);
  digitalWrite(TX_EN,LOW);
}

void loop()
{
  bool addr=digitalRead(ADDRESS)==HIGH?false:true;
  bool fire=digitalRead(FIRE)==HIGH?false:true;
  delay(1);
  addr=addr && (digitalRead(ADDRESS)==HIGH?false:true);
  fire=fire && (digitalRead(FIRE)==HIGH?false:true);

  if (tmrInput.tick()==true)
  {
    if (addr)
    {
      g_addr++;
      if (g_addr>15)
        g_addr=1;
  
      EEPROM.write(0,g_addr);
    }
  }

  if (fire)
    g_on=true;
  else
    g_on=false;
    
  display_hex(g_addr,g_on);
  
  /*if ( (blk--) < 0 )
  {
    digitalWrite(LD_LEFT,!digitalRead(LD_LEFT));
    digitalWrite(LD_RIGHT,!digitalRead(LD_RIGHT));
    blk=4;
  }*/

  if (tmrCycle.tick()==true)
  {
    if (g_sendSync==false)
    {
      sendCommandFrame(g_addr);
      g_sendSync=true;
    }
    else
    {
      sendCommandFrame(ADDR_SYNC);
      g_sendSync=false;
    }    
  }
  
  tmrNoComm.start();
}
