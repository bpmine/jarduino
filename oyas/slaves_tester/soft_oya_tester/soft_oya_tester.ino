#include <EEPROM.h>

#define SEG_A   (8)  /// Pin 8
#define SEG_B   (9)  /// Pin 9
#define SEG_C   (3)  /// Pin 3
#define SEG_D   (4)  /// Pin 4
#define SEG_E   (5)  /// Pin 5
#define SEG_F   (7)  /// Pin 7
#define SEG_G   (6)  /// Pin 6
#define SEG_DP  (2)  /// Pin 2

#define SEL     (A5)
#define ADDRESS (11)

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
unsigned long t0_sel_ms=0;
bool g_on=false;


/*unsigned char digit=0;

void apply_display(void)
{
  for (int i=0;i<NB_SEGS;i++)
  {
    if ( ( digit & (1<<i) ) == (1<<i) )
    {
      digitalWrite(list_segs[i],HIGH);
    }
    else
    {
      digitalWrite(list_segs[i],LOW);
    }
  }
}*/

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

  t0_sel_ms=0;
  g_addr=EEPROM.read(0);
  if ( (g_addr<1) || (g_addr>15) )
    g_addr=1;
  
  pinMode(ADDRESS,INPUT_PULLUP);
  pinMode(SEL,INPUT_PULLUP);
    
  display(0,false);  
  
  g_on=false;
}

void loop()
{
  bool sel=digitalRead(ADDRESS)==HIGH?false:true;
  bool fire=digitalRead(SEL)==HIGH?false:true;
  delay(1);
  sel=sel && (digitalRead(ADDRESS)==HIGH?false:true);
  fire=fire && (digitalRead(SEL)==HIGH?false:true);

  if (sel)
  {
    g_addr++;
    if (g_addr>15)
      g_addr=1;

    EEPROM.write(0,g_addr);
  }

  

  if (fire)
    g_on=true;
  else
    g_on=false;
    
  display_hex(g_addr,g_on);
  
  delay(200);
}
