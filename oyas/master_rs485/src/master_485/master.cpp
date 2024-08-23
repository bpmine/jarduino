/**
 * @file master.cpp
 * @brief IMPLEMENTATION - Gestion du maître des OYAs (RS485)
 * */
#include "master.h"
#include "prot.h"

#include "async.hpp"

void Master::init()
{
  pos=-1;
  pCurSlave=nullptr;

  list.init_all();
  nbcycles=0;
  tmrAnswer.stop();
  tmrWait.stop();  

  rxFrame.setReceiver(this);

  commands=0;
  raz_errs=0;
  raz_time=0;
  ponged=0;
  toping=0;
}
    
Master::Master()
{   
  init();
  eState = OFF;
  flgTrace=false;
}

Master::~Master()
{
}

void Master::start_cycle(void)
{
  if ( eState==IDLE )
  {
    pCurSlave=list.findFirstSlave(pos);
    if (pCurSlave!=nullptr)
    {
      eState=SEND;
      if (flgTrace)
        log("Start cycle");
    }
  }
}

void Master::set_config_slaves(unsigned short ens)
{
  list.config_slaves(ens&(0x3FFF));
}

void Master::set_commands(unsigned short cmds)
{
  /// @remark On ne commande que les esclaves actifs sur ce maitre. Les autres sont à 0 (sécurité).
  commands=(list.enabled_slaves() & cmds)<<1;
}

void Master::set_raz_time(unsigned short razs)
{
  raz_time|=(list.enabled_slaves() & razs)<<1;
}

void Master::set_raz_errs(unsigned short razs)
{
  raz_errs|=(list.enabled_slaves() & razs)<<1;
}

void Master::set_to_ping(unsigned short toping)
{
  this->toping=(list.enabled_slaves() & toping)<<1;
  this->ponged=0;
}

unsigned short Master::get_pong_states(void)
{
  return this->ponged;
}

void Master::setEnable(bool enable)
{
  if (enable==false)
  {
    eState=OFF;
    setPower(false);
    AsyncCmd.set_sended(); ///< Liberer la commande async
  }
  else if (eState==OFF)
  {
    init();
    eState=IDLE;
    setPower(true);
  }
}


void Master::fillSlaveObjFromSlaveFrame(Slave *pSlave,FrameSlave *pFrmSlave)
{
  pSlave->hum_pc=pFrmSlave->hum;
  pSlave->temp_dg=pFrmSlave->temp;
  pSlave->voltage=pFrmSlave->voltage;
  pSlave->total_slave_on_s=pFrmSlave->total_time_s;
  pSlave->total_slave_errs=pFrmSlave->total_errs;
  pSlave->last_slave_tick_ms=pFrmSlave->tick_ms;
  pSlave->cmd=pFrmSlave->cmd();
  pSlave->setOn(pFrmSlave->on());
}

bool Master::OnFrameReceive(FramePump *pump)
{
  Pump *pmp=list.getPump();
  if ( (pmp!=nullptr) && (pmp->addr==pump->addr) )
  {
    log("Pompe");

    fillSlaveObjFromSlaveFrame(pmp,pump);
    pmp->setCommOk(true);
    pmp->setFlow(pump->flow);

    return true;
  }

  return false;
}

bool Master::OnFrameReceive(FrameOya *oya)
{
  Oya *pObjOya=list.getOya(oya->addr);
  if ( pObjOya != nullptr )
  {
    log("Oya");

    fillSlaveObjFromSlaveFrame(pObjOya,oya);
    pObjOya->setCommOk(true);
    pObjOya->setHigh(oya->high());
    pObjOya->setLow(oya->low());

    return true;
  }

  return false;
}
   
bool Master::OnFrameReceive(FramePong *pong)
{
  if (AsyncCmd.is_pending())
  {
    if ( (AsyncCmd.get_cmd()==MSG_PING) && (pong->addr==AsyncCmd.get_addr()) )
    {
      log("Pong OK",pong->addr);
      AsyncCmd.set_acked();
      unsigned short msk=1<<pong->addr;
      ponged|=msk;
      return true;
    }
  }

  return false;
}

void Master::manage_asyncs(void)
{
  if (AsyncCmd.is_pending()==true)
    return;
  
  if (toping!=0)
  {
    for (int i=1;i<15;i++)
    {
      unsigned short msk=1<<i;
      if ( (toping&msk) == msk )
      {
        toping&=~msk;
        ponged&=~msk;
        AsyncCmd.send_ping(i);
        break;
      }
    }
  }
  else if (raz_time!=0)
  {
    for (int i=1;i<15;i++)
    {
      unsigned short msk=1<<i;
      if ( (raz_time&msk) == msk )
      {
        raz_time&=~msk;
        AsyncCmd.send_raz_total(i);
        break;
      }
    }
  }
  else if (raz_errs!=0)
  {
    for (int i=1;i<15;i++)
    {
      unsigned short msk=1<<i;
      if ( (raz_errs&msk) == msk )
      {
        raz_errs&=~msk;
        AsyncCmd.send_raz_errs(i);
        break;
      }
    }
  }
}

bool Master::loop(void)
{
  recv();
  
  switch (eState)
  {
    case OFF:
    {
      nbcycles=0;
      commands=0;
      list.init_all();
      break;
    }
    case IDLE:
    {
      tmrAnswer.start();
      break;
    }

    case SEND:
    {      
      if (pCurSlave==nullptr)
      {
    	  eState=IDLE;
      }
      else
      {
        eState = RECV;
        rxFrame.reset();
        tmrAnswer.start();
        FrameBuilder fb;
        if (pCurSlave != nullptr)
        {
          FrameCmd cmd;
          cmd.commands = commands;
          cmd.addr = pCurSlave->addr;
          fb.build(&cmd);
          sendBytes(fb.getBuffer(), fb.size());
        }
      }

      break;
    }
        
    case RECV:
    {
      if (tmrAnswer.tick()==true)
      {
        if (pCurSlave!=nullptr)
        {
          pCurSlave->setCommOk(false);
          log("Tmt slave ",pCurSlave->addr);
        }

        tmrWait.start();
        eState=WAIT;
      }

      break;
    }

    case WAIT:
    {
      if (tmrWait.tick()==true)
      	eState=NEXT;
      break;
    }

    case NEXT:
    {
      pCurSlave=list.findNextOya(pos);
      if (pCurSlave==nullptr)
      {
        if ( AsyncCmd.is_pending()==true )
          eState=ASYNC_SEND;
        else
          eState=END;
      }
      else
      {
        eState=SEND;
      }
          
      break;          
    }

    case ASYNC_SEND:
    {
      log("Async");
      FrameBuilder fb;

      switch (AsyncCmd.get_cmd())
      {
        case MSG_RAZ_TIME:
        {
          FrameRazT raz;
          raz.addr=AsyncCmd.get_addr();
          fb.build(&raz);
          sendBytes(fb.getBuffer(), fb.size());
          AsyncCmd.set_sended();
          eState = END;
          break;
        }
        case MSG_RAZ_ERR:
        {
          FrameRazE raz;
          raz.addr=AsyncCmd.get_addr();
          fb.build(&raz);
          sendBytes(fb.getBuffer(), fb.size());
          AsyncCmd.set_sended();
          eState = END;
          break;
        }
        case MSG_PING:
        {
          rxFrame.reset();

          FramePing ping;
          ping.addr=AsyncCmd.get_addr();
          ping.value=0xAA;
          fb.build(&ping);
          sendBytes(fb.getBuffer(), fb.size());
          AsyncCmd.set_sended();

          eState = ASYNC_RECV;
          tmrAnswer.start();
          break;
        }
        default:
        {
          eState = END;
          break;
        }
      }

      break;
    }

    case ASYNC_RECV:
    {
      if (AsyncCmd.is_pending()==true)
      {
        if (tmrAnswer.tick()==true)
        {
          log("Async Tmt slave ",AsyncCmd.get_addr());
          AsyncCmd.set_tmt();
          eState=END;
        }
      }
      else
      {
        eState=END;
      }

      break;
    }

    case END:
    {
      tmrAnswer.start();
      FrameBuilder fb;
      FrameCmd cmd(commands,'S');
      fb.build(&cmd);
      sendBytes(fb.getBuffer(),fb.size());
      eState=IDLE;
      nbcycles++;
      list.updCycleStats();

      if (flgTrace)
        log("SYNC");

      manage_asyncs();

      return true;
    }
  }

  return false;
}  

void Master::recv(void)
{
  while (available())
  {
    int b = readByte();
    if ((eState != RECV) && (eState != ASYNC_RECV))
      continue;

    if ((b > 0) && (b < 255))
    {
      E_FRAME_ERR res = rxFrame.recv(b);
      if ( (res == FRAME_OK) || (res == UNHANDLED_FRAME) )
      {
        eState = WAIT;
        tmrWait.start();
      }
    }
  }
}

bool Master::isRunning(void)
{
  return eState==OFF?false:true;
}

unsigned long Master::cycles(void)
{
  return nbcycles;
}

void Master::setTrace(bool flgEnabled)
{
  flgTrace=flgEnabled;
}

SlavesList * Master::getSlavesList(void)
{
  return &list;
}

void Master::set_pump(bool on)
{
  if (on==true)
    commands|=0x02;
  else
    commands&=~0x02;
}

void Master::set_oya(char addr,bool on)
{
  if ( (addr>1) && (addr<15) )
  {
    unsigned short mask=0x0001 << (addr);
    if (on==true)
      commands|=mask;
    else
      commands&=~mask;
  }
}

unsigned short Master::get_config_slaves(void)
{
  return list.enabled_slaves();
}
