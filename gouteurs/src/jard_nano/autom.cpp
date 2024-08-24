#include "autom.h"
#include <stdio.h>
    
#define MAX_MB_SIZE (8)

#define SIZE_MBS_INPUTS (2)
unsigned char mbs_inputs_last[SIZE_MBS_INPUTS];
unsigned char mbs_inputs_cur[SIZE_MBS_INPUTS];
unsigned char mbs_inputs_rising[SIZE_MBS_INPUTS];
unsigned char mbs_inputs_falling[SIZE_MBS_INPUTS];
MB mbs_inputs;

#define SIZE_MBS_OUTPUTS (2)
unsigned char mbs_outputs_last[SIZE_MBS_INPUTS];
unsigned char mbs_outputs_cur[SIZE_MBS_INPUTS];
unsigned char mbs_outputs_rising[SIZE_MBS_INPUTS];
unsigned char mbs_outputs_falling[SIZE_MBS_INPUTS];
MB mbs_outputs;

#define SIZE_MBS (6)
unsigned char mbs_last[SIZE_MBS];
unsigned char mbs_cur[SIZE_MBS];
unsigned char mbs_rising[SIZE_MBS];
unsigned char mbs_falling[SIZE_MBS];
MB mbs;
  
Analog::Analog()
{
  m_bSize=MAX_ANALOG_VALS;
  m_bCur=0;
  for (int i=0;i<MAX_ANALOG_VALS;i++)
  {
    m_vals[i]=0;
  }
}

Analog::Analog(unsigned char bSize)
{
  m_bSize=bSize;
  if (m_bSize==0)
    m_bSize=1;
  
  m_bCur=0;
  for (int i=0;i<MAX_ANALOG_VALS;i++)
  {
    m_vals[i]=0;
  }
}

void Analog::latch(unsigned short usValue)
{
  m_vals[m_bCur]=usValue;
  m_bCur++;
  if (m_bCur>=m_bSize)
    m_bCur=0;
}

void Analog::reinit(void)
{
  m_bCur=0;
  for (int i=0;i<MAX_ANALOG_VALS;i++)
  {
    m_vals[i]=0;
  }  
}

unsigned short Analog::get(void)
{
  unsigned long ul=0;
  for (int i=0;i<m_bSize;i++)
  {
    ul+=m_vals[i];
  }

  return ul/m_bSize;
}


    
MB::MB()
{
  m_bSize=0;
  m_pLast=NULL;
  m_pCur=NULL;
  m_pRising=NULL;
  m_pFalling=NULL;
}

void MB::init(unsigned char *pLast,unsigned char *pCur,unsigned char *pRising,unsigned char *pFalling,unsigned char i_bSize)
{
  if ( ( (i_bSize%8) !=0 ) && (i_bSize>MAX_MB_SIZE) )
  {
    m_bSize=0;
    m_pLast=NULL;
    m_pCur=NULL;
    m_pRising=NULL;
    m_pFalling=NULL;    

    return;
  }  
  
  m_bSize=i_bSize;
  m_pLast=pLast;
  m_pCur=pCur;
  m_pRising=pRising;
  m_pFalling=pFalling;  

  reinit();
}

void MB::reinit(void)
{
  if (m_bSize == 0 )
    return;

  for (int i=0;i<m_bSize;i++)
  {
    m_pLast[i]=0;
    m_pCur[i]=0;
    m_pRising[i]=0;
    m_pFalling[i]=0;
  }
}

void MB::reset(unsigned char i_bNum)
{
  if (i_bNum<m_bSize*8)
  {
    unsigned char nByte=i_bNum/8;
    unsigned char nBit=i_bNum%8;
    
    m_pCur[nByte]&=~(1<<nBit);
  }
}
    
void MB::set(unsigned char i_bNum)
{
  if (i_bNum<m_bSize*8)
  {
    unsigned char nByte=i_bNum/8;
    unsigned char nBit=i_bNum%8;
    
    m_pCur[nByte]|=(1<<nBit);
  }  
}

void MB::fromMB(unsigned char i_bNum,MB &src,unsigned char i_bNumSrc)
{
  bool flg=src.get(i_bNumSrc);
  fromBool(i_bNum,flg);
}

void MB::fromBool(unsigned char i_bNum,bool flg)
{
  if (flg==true)
    set(i_bNum);
  else
    reset(i_bNum);  
}

void MB::start_latch(void)
{
  if (m_bSize == 0 )
    return;

  for (int i=0;i<m_bSize;i++)
  {
    m_pLast[i]=m_pCur[i];
  }  
}

void MB::end_latch(void)
{
  unsigned char tmp[MAX_MB_SIZE];
  
  if (m_bSize == 0 )
    return;

  for (int i=0;i<m_bSize;i++)
  {
    tmp[i]=m_pLast[i]^m_pCur[i];
    m_pRising[i]=m_pCur[i]&tmp[i];
    m_pFalling[i]=(~m_pCur[i])&tmp[i];
  }    
}

bool MB::get(unsigned char i_bNum)
{
  if (i_bNum<m_bSize*8)
  {    
    unsigned char nByte=i_bNum/8;
    unsigned char nBit=i_bNum%8;

    unsigned char mask=(1<<nBit);    
    
    return ( (m_pCur[nByte]&mask) == mask)?true:false;
  }  
}

bool MB::getRising(unsigned char i_bNum)
{
  if (i_bNum<m_bSize*8)
  {    
    unsigned char nByte=i_bNum/8;
    unsigned char nBit=i_bNum%8;

    unsigned char mask=(1<<nBit);    
    
    return ( (m_pRising[nByte]&mask) == mask)?true:false;
  }  
}

bool MB::getFalling(unsigned char i_bNum)
{
  if (i_bNum<m_bSize*8)
  {    
    unsigned char nByte=i_bNum/8;
    unsigned char nBit=i_bNum%8;

    unsigned char mask=(1<<nBit);    
    
    return ( (m_pFalling[nByte]&mask) == mask)?true:false;
  }    
}

void MB::getStateStr(char *o_strState,int i_maxSize)
{
  if (i_maxSize<100)
    return;

  sprintf(o_strState,"%02x -> %02x / %02x %02x",m_pLast[0],m_pCur[0],m_pRising[0],m_pFalling[0]);
}

void init_autom(void)
{
  mbs_inputs.init(mbs_inputs_last,mbs_inputs_cur,mbs_inputs_rising,mbs_inputs_falling,SIZE_MBS_INPUTS);
  mbs_outputs.init(mbs_outputs_last,mbs_outputs_cur,mbs_outputs_rising,mbs_outputs_falling,SIZE_MBS_OUTPUTS);
  mbs.init(mbs_last,mbs_cur,mbs_rising,mbs_falling,SIZE_MBS);  
}
