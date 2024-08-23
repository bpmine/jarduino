#ifndef ASYNC_HEADER_INCLUDED
#define ASYNC_HEADER_INCLUDED

#include "prot.h"

class AsyncCmd
{
  private:
    unsigned char cmd;
    unsigned char addr;
    unsigned char val;
    bool sent;
    bool pending;
    bool acked;

    void reset(void)
    {
      this->cmd=0;
      this->addr=0;
      this->val=0;
      sent=false;
      pending=false;
      acked=false;
    }

    bool send_cmd(unsigned char cmd,unsigned char addr,unsigned char val=0)
    {
      if (pending==false)
      {
        reset();
        this->cmd=cmd;
        this->addr=addr;
        this->val=val;
        pending=true;

        return true;
      }

      return false;
    }

  public:
    AsyncCmd() {reset();};

    bool is_pending(void) {return pending;};
    unsigned char get_cmd(void) {return cmd;}
    unsigned char get_addr(void) {return addr;}

    bool send_raz_total(unsigned char addr) {return send_cmd(MSG_RAZ_TIME, addr);}
    bool send_raz_errs(unsigned char addr) {return send_cmd(MSG_RAZ_ERR, addr);}
    bool send_ping(unsigned char addr) {return send_cmd(MSG_PING, addr);}

    void set_sended(void)
    {
      if (pending==true)
      {
        if (cmd==MSG_PING)
        {
          sent=true;
        }
        else
        {
          reset();
        }
      }
      else
      {
        reset();
      }
    }
    void set_acked(void) {reset();}
    void set_tmt(void) {reset();}
};

AsyncCmd AsyncCmd;

#endif
