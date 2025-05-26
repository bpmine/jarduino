/** @file frames.c
 * @brief Protocol frames management
 */

#include "frames.h"
#include "prot.h"

static unsigned char _frames_hex_val(unsigned char hex);
static unsigned char _frames_decode_hex_byte(unsigned char a, unsigned char b);
static char _frames_tohexchar(unsigned char b);

static void _frames_enableCS(T_FRAME *pFrm);
static void _frames_disableCS(T_FRAME *pFrm);

static void _frames_pack(T_FRAME *pFrm,unsigned char b);
static void _frames_pack_byte(T_FRAME *pFrm,unsigned char b);
static void _frames_pack_sbyte(T_FRAME *pFrm,char b);
static void _frames_pack_word(T_FRAME *pFrm,unsigned short word);
static void _frames_pack_cs(T_FRAME *pFrm);
static void _frames_calc_size(T_FRAME *pFrm);

static void _frames_reset(T_FRAME *pFrm);
static void _frames_start_build(T_FRAME *pFrm,char msg);
static void _frames_end_build(T_FRAME *pFrm);

static void _frames_pack_common(T_FRAME *pFrm,unsigned char addr,unsigned char status,unsigned short tick_ms,unsigned char volt,unsigned short time_s,unsigned short errs);
static unsigned char _frames_on_receive_frame(T_FRAME *pFrame);


static inline unsigned char _frames_hex_val(unsigned char hex)
{
  if ( (hex>='0') && (hex<='9') )
  {
    return hex-'0';
  }
  else if ( (hex>='A') && (hex<='F') )
  {
    return (hex-'A')+10;
  }
  else
  {
    return 0;
  }
}

static inline unsigned char _frames_decode_hex_byte(unsigned char a, unsigned char b)
{
  return _frames_hex_val(a)*16 + _frames_hex_val(b);
}

static inline char _frames_tohexchar(unsigned char b)
{
    b = b & 0xF;
    if (b <= 9)
    {
        return '0' + b;
    }
    else if ((b >= 0x0A) && (b <= 0x0F))
    {
        return 'A' + (b - 10);
    }
    else
    {
        return '0';
    }
}

static inline void _frames_enableCS(T_FRAME *pFrm)
{
    pFrm->cs_enabled=1;
}

static inline void _frames_disableCS(T_FRAME *pFrm)
{
    pFrm->cs_enabled=0;
}

static inline void _frames_pack(T_FRAME *pFrm,unsigned char b)
{
    if (pFrm->pos < MAX_BUFFER_SIZE)
    {
        pFrm->data[pFrm->pos++] = b;

        if (pFrm->cs_enabled)
            pFrm->cs += b;
    }
}

static inline void _frames_pack_byte(T_FRAME *pFrm,unsigned char b)
{
    _frames_pack(pFrm,_frames_tohexchar((b >> 4) & 0x0F));
    _frames_pack(pFrm,_frames_tohexchar(b & 0x0F));
}

static inline void _frames_pack_sbyte(T_FRAME *pFrm,char b)
{
    unsigned char uc = (unsigned char)b;
    _frames_pack_byte(pFrm,uc);
}

static inline void _frames_pack_word(T_FRAME *pFrm,unsigned short word)
{
    _frames_pack(pFrm,_frames_tohexchar((word >> 12) & 0x0F));
    _frames_pack(pFrm,_frames_tohexchar((word >> 8) & 0x0F));
    _frames_pack(pFrm,_frames_tohexchar((word >> 4) & 0x0F));
    _frames_pack(pFrm,_frames_tohexchar(word & 0x0F));
}

static inline void _frames_pack_cs(T_FRAME *pFrm)
{
    _frames_disableCS(pFrm);
    _frames_pack_byte(pFrm,pFrm->cs);
}

static inline void _frames_calc_size(T_FRAME *pFrm)
{
  _frames_disableCS(pFrm);
  unsigned char tmp = pFrm->pos;
  unsigned char sz = pFrm->pos - 4;
  pFrm->pos = 1;
  _frames_pack_byte(pFrm,sz);
  pFrm->pos = tmp;
}

static inline void _frames_reset(T_FRAME *pFrm)
{
    int i;
    for (i=0;i<MAX_BUFFER_SIZE;i++)
        pFrm->data[i]=0;

    pFrm->pos=0;
    pFrm->cs_enabled=0;
    pFrm->cs=0;
    pFrm->size=0;
}

static inline void _frames_start_build(T_FRAME *pFrm,char msg)
{
  _frames_reset(pFrm);
  _frames_pack(pFrm,SOH);
  _frames_pack_byte(pFrm,0);   ///< Size
  _frames_enableCS(pFrm);
  _frames_pack(pFrm,msg);
}

static inline void _frames_end_build(T_FRAME *pFrm)
{
  _frames_pack_cs(pFrm);
  _frames_pack(pFrm,STX);
  _frames_calc_size(pFrm);

  pFrm->size=pFrm->pos;
}

static void _frames_pack_common(T_FRAME *pFrm,unsigned char addr,unsigned char status,unsigned short tick_ms,unsigned char volt,unsigned short time_s,unsigned short errs)
{
   _frames_pack_byte(pFrm,addr);
   _frames_pack_byte(pFrm,status);
   _frames_pack_word(pFrm,tick_ms);
   _frames_pack_sbyte(pFrm,0);
   _frames_pack_sbyte(pFrm,0);
   _frames_pack_byte(pFrm,volt);
   _frames_pack_word(pFrm,time_s);
   _frames_pack_word(pFrm,errs);
}

static unsigned char _frames_read_char(T_FRAME *pFrm, char* c, int *pCur)
{
    *c = 0;
    if ( (*pCur) >= pFrm->size )
        return 0;
    else
    {
        *c=pFrm->data[(*pCur)++];
        return 1;
    }
}


static unsigned char _frames_read_byte(T_FRAME *pFrm, unsigned char* uc, int *pCur)
{
    *uc = 0;

    if ( (*pCur)+1 >= pFrm->size )
        return 0;
    else
    {
        unsigned char a= pFrm->data[(*pCur)++];
        unsigned char b = pFrm->data[(*pCur)++];
        *uc=_frames_decode_hex_byte(a, b);

        return 1;
    }
}

static unsigned char _frames_read_word(T_FRAME *pFrm, unsigned short* us, int *pCur)
{
    *us = 0;

    if ((*pCur) + 3 >= pFrm->size)
        return 0;
    else
    {
        unsigned char a = pFrm->data[(*pCur)++];
        unsigned char b = pFrm->data[(*pCur)++];
        unsigned char c = pFrm->data[(*pCur)++];
        unsigned char d = pFrm->data[(*pCur)++];
        unsigned char uc = _frames_decode_hex_byte(a, b);
        (*us) |= ((uc << 8) & 0xFF00);
        uc = _frames_decode_hex_byte(c, d);
        (*us) |= (uc & 0x00FF);

        return 1;
    }
}

unsigned char _frames_on_receive_frame(T_FRAME *pFrame)
{
    int cur=1;
    char msg;
    unsigned char len;

    if (_frames_read_byte(pFrame,&len,&cur)!=1)
        return 0;

    if (_frames_read_char(pFrame,&msg, &cur)!=1)
        return 0;

    switch (msg)
    {
        case MSG_CMD:
        {
            unsigned short cmds;
            unsigned char addr;

            if (_frames_read_word(pFrame,&cmds, &cur)!=1)
                return 0;

            if (_frames_read_byte(pFrame,&addr, &cur)!=1)
                return 0;

            if (addr!=ADDR_SYNC)
                frames_on_receive_cmds(cmds,addr);
            else
                frames_on_receive_sync();

            break;
        }

        case MSG_PING:
        {
            unsigned char val;

            if (_frames_read_byte(pFrame,&val, &cur)!=1)
                return 0;

            frames_on_receive_ping(val);

            break;
        }
    }

    return 1;
}


void frames_init(T_FRAME *pFrame)
{
    int i;
    for (i=0;i<MAX_BUFFER_SIZE;i++)
    {
        pFrame->data[i]=0;
    }
    pFrame->size=0;
    pFrame->pos=0;
}

void frames_build_oya(T_FRAME *pFrame,unsigned char addr,unsigned char status,unsigned short tick_ms,unsigned char volt,unsigned short time_s,unsigned short errs)
{
    _frames_start_build(pFrame,MSG_OYA);
    _frames_pack_common(pFrame,addr,status,tick_ms,volt,time_s,errs);
    _frames_end_build(pFrame);
}


extern void frames_build_pump(T_FRAME *pFrame,unsigned char addr,unsigned char status,unsigned short tick_ms,unsigned char volt,unsigned short time_s,unsigned short errs,unsigned short flow)
{
    _frames_start_build(pFrame,MSG_OYA);
    _frames_pack_common(pFrame,addr,status,tick_ms,volt,time_s,errs);
    _frames_pack_word(pFrame,flow);
    _frames_end_build(pFrame);
}

void frames_build_pong(T_FRAME *pFrame,unsigned char val)
{
    _frames_start_build(pFrame,MSG_OYA);
    _frames_pack_byte(pFrame,val);
    _frames_end_build(pFrame);
}

E_FRAME_ERR frames_on_receive_byte(T_FRAME *pFrame,unsigned char b)
{
    if ( (pFrame->pos==0) && (b==SOH) )
    {
      ///@remark Synchro trame recue, on commence l'analyse de la trame
      _frames_pack(pFrame, b);
      return PENDING;
    }
    else if ( (pFrame->pos == 1) || (pFrame->pos == 2) )
    {
      ///@remark Reception de la taille (octets pos 1 et 2)
      if (b == SOH)
      {
        ///@remark Recuperation si SOH recu
        _frames_reset(pFrame);
        _frames_pack(pFrame,b);
        return PENDING;
      }
      else if (b == STX)
      {
        ///@remark Fin en erreur si caractere de fin
        _frames_reset(pFrame);
        return BAD_FRAME;
      }
      else
      {
        ///@remark Reception taille
        _frames_pack(pFrame,b);
        return PENDING;
      }
    }
    else if ((pFrame->pos > 2) && (pFrame->pos < MAX_BUFFER_SIZE))
    {
      ///@reception du reste (jusqu'au maximum autorise)
      _frames_pack(pFrame,b);
      if (b==STX)
      {
          unsigned char sz = _frames_decode_hex_byte(pFrame->data[1], pFrame->data[2]);
          if (sz != pFrame->pos - 4)
          {
              _frames_reset(pFrame);
              return BAD_FRAME;
          }
          else
          {
              unsigned char cs = _frames_decode_hex_byte(pFrame->data[pFrame->pos-3], pFrame->data[pFrame->pos-2]);
              unsigned char cs_calc = 0;
              int i;
              for (i = 0; i < sz-2;i++)
              {
                  cs_calc += pFrame->data[3 + i];
              }

              if (cs_calc != cs)
              {
                  _frames_reset(pFrame);
                  return BAD_CS;
              }
              else
              {
                  //Serial.println(cs_calc,HEX);
                  pFrame->size=pFrame->pos;
                  unsigned char ret=_frames_on_receive_frame(pFrame);
                  _frames_reset(pFrame);
                  return ret==1?FRAME_OK:UNHANDLED_FRAME;
              }
          }
      }
      else if (b == SOH)
      {
          _frames_reset(pFrame);
          _frames_pack(pFrame,b);
          return PENDING;
      }

      return PENDING;
    }
    else
    {
      _frames_reset(pFrame);
      return WAITING;
    }
}
