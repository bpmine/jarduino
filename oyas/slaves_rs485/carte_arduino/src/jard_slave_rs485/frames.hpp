#ifndef FRAMES_HEADER_INCLUDED
#define FRAMES_HEADER_INCLUDED

#include "prot.h"

class Frame
{
public:
   char id_msg;

   Frame()
   {
      id_msg = 0;
   }
   virtual ~Frame()
   {    
   }
};


class FrameSlave : Frame
{
public:
   unsigned char addr;
   unsigned char status;
   unsigned short tick_ms;
   char temp;
   char hum;
   unsigned char voltage;
   unsigned short total_time_s;
   unsigned short total_errs;

   bool cmd()
   {
      return (status & STATUS_CMD) == STATUS_CMD? true: false;
   }

   bool on()
   {
      return (status & STATUS_ON) == STATUS_ON ? true : false;
   }

   FrameSlave& setCmd(bool cmd)
   {
      if (cmd == true)
         status |= STATUS_CMD;
      else
         status &= ~STATUS_CMD;

      return *this;
   }

   FrameSlave& setOn(bool on)
   {
      if (on == true)
         status |= STATUS_ON;
      else
         status &= ~STATUS_ON;

      return *this;
   }

   FrameSlave() : Frame()
   {
      addr = 0;
      status = 0;
      tick_ms = 0;
      temp = 0;
      hum = 0;
      voltage = 0;
      total_time_s = 0;
      total_errs = 0;
   }

   FrameSlave(
       char addr,
       unsigned char status,
       unsigned short tick_ms,
       char temp,
       char hum,
       unsigned char voltage,
       unsigned short total_time_s,
       unsigned short total_errs
   ) : Frame()
   {
       this->addr = addr;
       this->status = status;
       this->tick_ms = tick_ms;
       this->temp = temp;
       this->hum = hum;
       this->voltage = voltage;
       this->total_time_s = total_time_s;
       this->total_errs = total_errs;
   }

   ~FrameSlave()
   {    
   }
};

class FramePump : public FrameSlave
{
public:
   unsigned short flow;

   FramePump() : FrameSlave()
   {
      flow = 0;
   }

   FramePump(
       char addr,
       unsigned short tick_ms,
       char temp,
       char hum,
       unsigned char voltage,
       unsigned short total_time_s,
       unsigned short total_errs,
       bool cmd,
       bool on,
       unsigned short flow
   ) : FrameSlave(addr,0,tick_ms,temp,hum,voltage,total_time_s,total_errs)
   {
       this->setCmd(cmd);
       this->setOn(on);
       this->flow = flow;
   }

   ~FramePump()
   {    
   }
};

class FrameOya : public FrameSlave
{
public:
   bool low()
   {
      return (status & STATUS_LVL_LOW)== STATUS_LVL_LOW ? true:false;
   }
   bool high()
   {
      return (status & STATUS_LVL_HIGH) == STATUS_LVL_HIGH ? true : false;
   }

   FrameOya& setHigh(bool high)
   {
      if (high == true)
         status |= STATUS_LVL_HIGH;
      else
         status &= ~STATUS_LVL_HIGH;

      return *this;
   }

   FrameOya& setLow(bool low)
   {
      if (low == true)
         status |= STATUS_LVL_LOW;
      else
         status &= ~STATUS_LVL_LOW;

      return *this;
   }

   FrameOya() : FrameSlave()
   {
   }

   FrameOya(
       unsigned char addr,
       unsigned short tick_ms,
       char temp,
       char hum,
       unsigned char voltage,
       unsigned short total_time_s,
       unsigned short total_errs,
       bool cmd,
       bool on,
       bool low,
       bool high
   ) : FrameSlave(addr, 0, tick_ms, temp, hum, voltage, total_time_s, total_errs)
   {
       this->setCmd(cmd);
       this->setOn(on);
       this->setLow(low);
       this->setHigh(high);
   }

   ~FrameOya()
   {    
   }
};

class FrameCmd : public Frame
{
public:
   unsigned short commands;
   unsigned char addr;

   FrameCmd() : Frame()
   {
      commands = 0;
      addr = 0;
   }

   FrameCmd(unsigned short cmds,char addr) : Frame()
   {
       this->commands = cmds;
       this->addr = addr;
   }

   ~FrameCmd()
   {    
   }
};

class FramePing : public Frame
{
public:
   unsigned char addr;
   unsigned char value;

   FramePing() : Frame()
   {
      value = 0;
      addr = 0;
   }

   FramePing(char addr,unsigned char value) : Frame()
   {
       this->value=value;
       this->addr=addr;
   }

   ~FramePing()
   {    
   }
};

class FramePong : public Frame
{
public:
   unsigned char addr;
   unsigned char value;

   FramePong() : Frame()
   {
      value = 0;
      addr = 0;
   }

   FramePong(char addr, unsigned char value) : Frame()
   {
       this->value = value;
       this->addr = addr;
   }

   ~FramePong()
   {    
   }
};

class FrameRazT : public Frame
{
public:
   unsigned char addr;

   FrameRazT() : Frame()
   {
      addr = 0;
   }

   FrameRazT(char addr) : Frame()
   {
       this->addr = addr;
   }

   ~FrameRazT()
   {    
   }
};

class FrameRazE : public Frame
{
public:
   unsigned char addr;

   FrameRazE() : Frame()
   {
      addr = 0;
   }

   FrameRazE(char addr) : Frame()
   {
       this->addr = addr;
   }

   ~FrameRazE()
   {    
   }
};

class IFrameReceiver
{
public:
   virtual ~IFrameReceiver() {}
   virtual bool OnFrameReceive(FrameCmd *) {return false;}
   virtual bool OnFrameReceive(FramePump *) {return false;}
   virtual bool OnFrameReceive(FrameOya *) {return false;}
   virtual bool OnFrameReceive(FramePing *) {return false;}
   virtual bool OnFrameReceive(FramePong *) {return false;}
   virtual bool OnFrameReceive(FrameRazT *) {return false;}
   virtual bool OnFrameReceive(FrameRazE *) {return false;}
};

#endif
