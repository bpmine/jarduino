/**
* @file framebuilder.cpp
*
* @brief IMPLEMENTATION - Gestion d'une trame RS485 - Octet par octet (reception) ou par valeurs (emission)
*/
#include "framebuilder.h"
#include "frames.hpp"

#include <string.h>
#include <stdio.h>

unsigned char FrameBuilder::hex_val(unsigned char hex)
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

unsigned char FrameBuilder::decode_hex_byte(unsigned char a, unsigned char b)
{
  return hex_val(a)*16+hex_val(b);
}

char FrameBuilder::tohexchar(unsigned char b)
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

void FrameBuilder::reset(void)
{
  cs=0;
  memset(buffer,0, MAX_BUFFER_SIZE);
  pos=0;
  cs_enabled=false;  
}

FrameBuilder::FrameBuilder(void)
{
	reset();
	pReceiver = NULL;
}

void FrameBuilder::enableCS(void)
{
  cs_enabled=true;
}

void FrameBuilder::disableCS(void)
{
  cs_enabled=false;
}

void FrameBuilder::pack(unsigned char b)
{
	if (pos < MAX_BUFFER_SIZE)
	{
		buffer[pos++] = b;
		if (cs_enabled)
			cs += b;
	}
}

void FrameBuilder::pack_byte(unsigned char b)
{
	pack(tohexchar((b >> 4) & 0x0F));
	pack(tohexchar(b & 0x0F));	
}

void FrameBuilder::pack_sbyte(char b)
{
	unsigned char uc = (unsigned char)b;
	pack_byte(uc);
}

void FrameBuilder::pack_word(unsigned short word)
{
	pack(tohexchar((word >> 12) & 0x0F));
	pack(tohexchar((word >> 8) & 0x0F));
	pack(tohexchar((word >> 4) & 0x0F));
	pack(tohexchar(word & 0x0F));
}

void FrameBuilder::pack_cs(void)
{
  disableCS();
  pack_byte(cs);
}

void FrameBuilder::calc_size(void)
{
  disableCS();
  unsigned char tmp = pos;
  unsigned char sz = pos - 4;
  pos = 1;  
  pack_byte(sz);
  pos = tmp;
}

void FrameBuilder::start_build(char msg)
{
  reset();
  pack(SOH);
  pack_byte(0);   ///< Size
  enableCS();
  pack(msg);
}

void FrameBuilder::end_build(void)
{
  pack_cs();
  pack(STX);
  calc_size();
}

unsigned char* FrameBuilder::build(FramePing *pFramePing)
{
  start_build(MSG_PING);
  pack_byte(pFramePing->addr);
  pack_byte(pFramePing->value);
  end_build();

	return buffer;
}

unsigned char* FrameBuilder::build(FramePong *pFramePong)
{
  start_build(MSG_PONG);
  pack_byte(pFramePong->addr);
  pack_byte(pFramePong->value);
  end_build();

	return buffer;
}

unsigned char* FrameBuilder::build(FrameRazT *pFrameRazT)
{
  start_build(MSG_RAZ_TIME);
  pack_byte(pFrameRazT->addr);
  end_build();

  return buffer;
}

unsigned char* FrameBuilder::build(FrameRazE *pFrameRazE)
{
  start_build(MSG_RAZ_ERR);
  pack_byte(pFrameRazE->addr);
  end_build();

  return buffer;
}

unsigned char* FrameBuilder::build(FrameCmd *pFrameCmd)
{
  start_build(MSG_CMD);
  pack_word(pFrameCmd->commands);
  pack_byte(pFrameCmd->addr);
  end_build();

	return buffer;
}

unsigned char* FrameBuilder::build(FrameOya *pFrameOya)
{
	start_build(MSG_OYA);
	pack_byte(pFrameOya->addr);
	pack_byte(pFrameOya->status);
	pack_word(pFrameOya->tick_ms);
	pack_sbyte(pFrameOya->temp);
	pack_sbyte(pFrameOya->hum);
	pack_byte(pFrameOya->voltage);
	pack_word(pFrameOya->total_time_s);
	pack_word(pFrameOya->total_errs);
	end_build();

	return buffer;
}

unsigned char * FrameBuilder::build(FramePump *pFramePump)
{
	start_build(MSG_PUMP);
	pack_byte(pFramePump->addr);
	pack_byte(pFramePump->status);
	pack_word(pFramePump->tick_ms);
	pack_sbyte(pFramePump->temp);
	pack_sbyte(pFramePump->hum);
	pack_byte(pFramePump->voltage);
	pack_word(pFramePump->total_time_s);
	pack_word(pFramePump->total_errs);
	pack_word(pFramePump->flow);
	end_build();

	return buffer;
}

E_FRAME_ERR FrameBuilder::recv(unsigned char b)
{
  if ( (pos==0) && (b==SOH) )
  {
    ///@remark Synchro trame recue, on commence l'analyse de la trame
		pack(b);
		return PENDING;
  }
  else if ( (pos==1) || (pos==2) )
  {
    ///@remark Reception de la taille (octets pos 1 et 2)
	  if (b == SOH)
	  {
      ///@remark Recuperation si SOH recu   
		  reset();        
			pack(b);
      return PENDING;
    }
    else if (b == STX)
    {
      ///@remark Fin en erreur si caractere de fin
      reset();
		  return BAD_FRAME;
	  }
	  else
	  {
      ///@remark Reception taille
		  pack(b);      
		  return PENDING;
	  }
  }
  else if ((pos>2) && (pos< MAX_BUFFER_SIZE))
  {
    ///@reception du reste (jusqu'au maximum autorise)
  	pack(b);
  	if (b==STX)
  	{
  		unsigned char sz = decode_hex_byte(buffer[1], buffer[2]);
  		if (sz != pos - 4)
  		{
  			reset();
  			return BAD_FRAME;
  		}
  		else
  		{
  			unsigned char cs = decode_hex_byte(buffer[pos-3], buffer[pos-2]);
  			unsigned char cs_calc = 0;
  			for (int i = 0; i < sz-2;i++)
  			{
  				cs_calc += buffer[3 + i];
  			}
  
  			if (cs_calc != cs)
  			{
  				reset();
  				return BAD_CS;
  			}
  			else
  			{ 
          //Serial.println(cs_calc,HEX);
  				bool ret=OnFrameDecode();
  				reset();
  				return ret==true?FRAME_OK:UNHANDLED_FRAME;
  			}
  		}
  	}
  	else if (b == SOH)
  	{
  		reset();
  		pack(b);
  		return PENDING;
  	}
  
  	return PENDING;
  }
  else
  {
	  reset();
	  return WAITING;
  }
}

unsigned char FrameBuilder::size(void)
{
  return pos;
}

unsigned char* FrameBuilder::getBuffer(void)
{
	return buffer;
}

int FrameBuilder::getMaxBufferSize(void)
{
	return MAX_BUFFER_SIZE;
}


int FrameBuilder::first_read(void)
{
	return 3;
}

bool FrameBuilder::read_char(char* c, int& cur)
{
	*c = 0;
	if (cur >= pos)
		return false;
	else
	{
		*c=buffer[cur++];
		return true;
	}
}

bool FrameBuilder::read_byte(unsigned char* uc, int& cur)
{
	*uc = 0;

	if (cur+1 >= pos)
		return false;
	else
	{
		unsigned char a= buffer[cur++];
		unsigned char b = buffer[cur++];
		*uc=decode_hex_byte(a, b);

		return true;
	}
}

bool FrameBuilder::read_sbyte(char* c, int& cur)
{
	unsigned char uc;
	bool ret = read_byte(&uc, cur);
	if (ret == true)
	{
		*c = (char)uc;
		return true;
	}
	else
		return false;
}

bool FrameBuilder::read_word(unsigned short* us, int& cur)
{
	*us = 0;

	if (cur + 3 >= pos)
		return false;
	else
	{
		unsigned char a = buffer[cur++];
		unsigned char b = buffer[cur++];
		unsigned char c = buffer[cur++];
		unsigned char d = buffer[cur++];
		unsigned char uc = decode_hex_byte(a, b);
		*us |= ((uc << 8) & 0xFF00);
		uc = decode_hex_byte(c, d);
		*us |= (uc & 0x00FF);

		return true;
	}
}


void FrameBuilder::setReceiver(IFrameReceiver *pReceiver)
{
  this->pReceiver=pReceiver;
}

bool FrameBuilder::OnFrameDecode(void)
{
  if (pReceiver==nullptr)
	  return false;

  char msg;
  int cur=first_read();
  if (read_char(&msg, cur)==false)
	  return false;

  if ( msg==MSG_CMD )
  {
		FrameCmd cmd;
		if (read_word(&cmd.commands,cur)==false)
			return false;
		if (read_byte(&cmd.addr,cur)==false)
			return false;

		return pReceiver->OnFrameReceive(&cmd);
  }
  else if ( msg==MSG_OYA )
  {
		FrameOya oya;
		if (read_byte(&oya.addr,cur)==false)
			return false;
		if (read_byte(&oya.status,cur)==false)
			return false;
		if (read_word(&oya.tick_ms,cur)==false)
			return false;
		if (read_sbyte(&oya.temp, cur)==false)
			return false;
		if (read_sbyte(&oya.hum, cur)==false)
			return false;
		if (read_byte(&oya.voltage,cur)==false)
			return false;
		if (read_word(&oya.total_time_s,cur)==false)
			return false;
		if (read_word(&oya.total_errs,cur)==false)
			return false;

		return pReceiver->OnFrameReceive(&oya);
  }
  else if ( msg==MSG_PUMP )
  {
		FramePump pump;
		if (read_byte(&pump.addr,cur)==false)
			return false;
		if (read_byte(&pump.status,cur)==false)
			return false;
		if (read_word(&pump.tick_ms,cur)==false)
			return false;
		if (read_sbyte(&pump.temp, cur)==false)
			return false;
		if (read_sbyte(&pump.hum, cur)==false)
			return false;
		if (read_byte(&pump.voltage,cur)==false)
			return false;
		if (read_word(&pump.total_time_s,cur)==false)
			return false;
		if (read_word(&pump.total_errs,cur)==false)
			return false;
		if (read_word(&pump.flow,cur)==false)
			return false;

		return pReceiver->OnFrameReceive(&pump);
  }
  else if ( msg==MSG_PING )
  {
		FramePing ping;
		if (read_byte(&ping.addr,cur)==false)
			return false;
		if (read_byte(&ping.value,cur)==false)
			return false;

		return pReceiver->OnFrameReceive(&ping);
	}
	else if ( msg==MSG_PONG )
	{
		FramePong pong;
		if (read_byte(&pong.addr,cur)==false)
			return false;
		if (read_byte(&pong.value,cur)==false)
			return false;

		return pReceiver->OnFrameReceive(&pong);
  }
	else if (msg==MSG_RAZ_TIME)
	{
	  FrameRazT razT;
    if (read_byte(&razT.addr,cur)==false)
      return false;

    return pReceiver->OnFrameReceive(&razT);
	}
  else if (msg==MSG_RAZ_ERR)
  {
    FrameRazE razE;
    if (read_byte(&razE.addr,cur)==false)
      return false;

    return pReceiver->OnFrameReceive(&razE);
  } 

  return false;
}
