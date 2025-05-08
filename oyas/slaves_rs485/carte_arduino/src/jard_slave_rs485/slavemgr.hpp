#ifndef SLAVEMGR_HEADER_INCLUDED
#define SLAVEMGR_HEADER_INCLUDED

#include "frames.hpp"
#include "framebuilder.h"
#include "globals.h"
#include "timer.h"

#define	SLAVE_CMD_TIMEOUT_MS	  (4UL*1000UL)
#define SLAVE_SYNC_TIMEOUT_MS   (6UL*1000UL)
#define SERIAL_SPEED            (9600)

class SlaveMgr : IFrameReceiver
{
  private:
    Timer tmrCmd=Timer(SLAVE_CMD_TIMEOUT_MS,true);    ///< Detection timeout de commande (Pas de commande, il faut se mettre en secu)
    Timer tmrSync=Timer(SLAVE_SYNC_TIMEOUT_MS,true);  ///< Detection timeout de SYNC (Pas de remontee de datas vers le maitre)
  	bool flgCommOk;
  	bool flgSyncOk;
  	bool flgEndOfCycle;
  	unsigned char addr;
  	bool flgCmd;

    void FillInCommonFields(FrameSlave *pFrmSlave)
    {
  	  pFrmSlave->addr=addr;
  	  pFrmSlave->hum=g_hum_pc;
  	  pFrmSlave->temp=g_temp_dg;
      pFrmSlave->voltage=g_mes_cv;
  	  pFrmSlave->tick_ms=getTick()%0xFFFF;
  	  pFrmSlave->total_time_s=g_total_s;
  	  pFrmSlave->total_errs=g_errors;
  
  	  pFrmSlave->setCmd(flgCmd).setOn(g_on);
    }

  protected:
  	FrameBuilder rxFrame;
  
  	virtual void send(unsigned char *buffer,int size)=0;
  	virtual unsigned long getTick(void)=0;

  public:
	  SlaveMgr()
    {
  		flgEndOfCycle=false;
  		addr=0;
  		flgCommOk=false;
  		flgSyncOk=false;
  		flgCmd=false;
	  }

    virtual ~SlaveMgr()
    {
    }

    virtual void onSerialEvent(void)=0;
    virtual void onCmdReceived(bool flgCmd)=0;

    bool OnFrameReceive(FrameRazT* pRazT) override
    {
      if (addr==pRazT->addr)
      {
        reset_time();
        return true;
      }

      return false;
    }

    bool OnFrameReceive(FrameRazE* pRazE) override
    {
      if (addr==pRazE->addr)
      {
        reset_errs();
        return true;
      }

      return false;
    }

    bool OnFrameReceive(FramePing* pFrmPing) override
    {
    	if (addr==pFrmPing->addr)
    	{
    	  FrameBuilder fb;
    	  FramePong pong(addr,~pFrmPing->value);
    	  fb.build(&pong);
    	  send(fb.getBuffer(),fb.size());

    	  return true;
    	}

    	return false;
    }

    bool OnFrameReceive(FrameCmd* pFrmCmd) override
    {
    	/// On applique systematiquement les commandes
	  unsigned short mask = (1<<addr) & (0x7FFE);

	  if ((pFrmCmd->commands&mask)==mask)
	  {
	    flgCmd=true;
  	}
  	else
  	{
  	  flgCmd=false;
  	}

	  tmrCmd.start();
      flgCommOk=true;
      onCmdReceived(flgCmd);
  
  		/// On r�pond une trame si l'adresse de l'esclave est demandee
      if (addr==pFrmCmd->addr)
      {
        if (addr==1)
      	{
          FrameBuilder fb;
        	FramePump pump;
    
        	FillInCommonFields(&pump);
        	pump.flow=g_flow_mLpMin;
    
        	fb.build(&pump);
        	send(fb.getBuffer(),fb.size());
      	}
      	else if ( (addr>1) && (addr<15) )
      	{
        	FrameBuilder fb;
        	FrameOya oya;
    
        	FillInCommonFields(&oya);
        	oya.setLow(g_cpt_low).setHigh(g_cpt_high);
    
        	fb.build(&oya);
        	send(fb.getBuffer(),fb.size());
      	}
    	}
     	/// On finalise le cycle si l'adresse est celle de sync (initialement 'S')
     	else if (pFrmCmd->addr==ADDR_SYNC)
     	{
     	  tmrSync.start();
     	  flgSyncOk=true;
     	  flgEndOfCycle=true;
     	}
  
     	return true;
    }

    virtual void begin(unsigned char addr)
    {
     	if (addr<=15)
     	  this->addr=addr;
     	else
     	  this->addr=0;
  
     	flgCommOk=false;
     	flgSyncOk=false;
     	flgEndOfCycle=false;
     	flgCmd=false;
      rxFrame.setReceiver(this);
    }

    bool loop(void)
    {
      if (flgSyncOk==true)
      {
        if (tmrSync.tick()==true)
        {
          flgSyncOk=false;
        }
      }

      	if (flgCommOk==true)
      	{
      	  if (tmrCmd.tick()==true)
      	  {
            flgCommOk=false;
            return false;
      	  }
      	}

      	if (flgEndOfCycle==true)
      	{
      	  flgEndOfCycle=false;
      	  return true;
      	}

     	return false;
    }

    bool isAlive(void)
    {
    	return flgCommOk;
    }

    bool isSyncOk(void)
    {
      return flgSyncOk;
    }

    bool isCmd(void)
    {
    	return flgCmd;
    }
};

#endif
