/**
 * @file slaves.hpp
 * @brief Representation des etats des esclaves (Pompe et Oyas)
*/
#ifndef SLAVES_HEADER_INCLUDED
#define SLAVES_HEADER_INCLUDED

class Slave
{
  public:
    char addr;		///< Adresse de l'esclave (1 -> 14. 1 est la pompe.)
    bool on;		  ///< Etat reel de l'esclave ON/OFF
    int temp_dg;	///< Temperature remontee
    int hum_pc;		///< Humidite remontee
    int voltage;  ///< Tension

    bool comm_ok;	///< Communication ok (ou pas)
    
    unsigned long cycles_since_off;
    unsigned long cycles_since_on;
    unsigned long cycles_since_nok;
    unsigned long cycles_since_ok;
    unsigned long cycles_errors;

    bool cmd;		///< Derniere commande envoyee a l'esclave

    unsigned short last_slave_tick_ms;
    unsigned short total_slave_on_s;
    unsigned short total_slave_errs;

    virtual void init(void)
    {
      on=false;
      temp_dg=-1;
      hum_pc=-1;
      comm_ok=false;
      
      cycles_since_off=0;
      cycles_since_on=0;
      cycles_since_nok=0;
      cycles_since_ok=0;
      cycles_errors=0;

      cmd=false;

      last_slave_tick_ms=0;
      total_slave_on_s=0;
      total_slave_errs=0;
    }

    void init(char addr)
    {
      this->addr=addr;
      init();
    }

    Slave(char addr)
    {
      init(addr);
    }

    virtual ~Slave()
    {
    }

    void setOn(bool flgOn)
    {
      if ( on != flgOn )
      {
        cycles_since_on=0;
        cycles_since_off=0;
      }
      else if ( on == true )
      {
        cycles_since_on++;
      }
      else if ( on == false )
      {
        cycles_since_off++;
      }

      on=flgOn;
    }

    void setCommOk(bool flgCommOk)
    {
      if ( comm_ok != flgCommOk )
      {
        cycles_since_on=0;
        cycles_since_off=0;
        cycles_since_nok=0;
        cycles_since_ok=0;
      }
      else if ( comm_ok == true )
      {
        cycles_since_ok++;
      }
      else if ( comm_ok == false )
      {
        cycles_since_off++;
        cycles_since_nok++;
      }

      comm_ok=flgCommOk;
    }

    void updCycleStats(void)
    {
      if (comm_ok==true)
      {
        if (on==false)
          cycles_since_off++;
        else
          cycles_since_on++;
          
        cycles_since_nok=0;
        cycles_since_ok++;
      }
      else
      {
        cycles_since_off=0;
        cycles_since_on=0;
        cycles_since_ok=0;
        cycles_since_nok++;
        cycles_errors++;
      }      
    }

    void razErrors(void)
    {
      cycles_errors=0;
    }
};

class Pump : public Slave
{
  public:
    int flow;

    virtual void init(void) override
	  {
      Slave::init();
      flow=0;
	  }

    Pump(char addr):Slave(addr) {flow=0;}
    ~Pump() {}

    void setFlow(int flow) {this->flow=flow;}
};


class Oya : public Slave
{
  public:
    bool high;
    bool low;

    virtual void init() override
	  {
      Slave::init();
      high=false;
      low=false;
	  }

    Oya(char addr) : Slave(addr) {high=false;low=false;}
    ~Oya() {}

    void setHigh(bool high) {this->high=high;}
    void setLow(bool low) {this->low=low;}
};

#define NUM_SLAVES_MAX	(14)
class SlavesList
{
  private:
    Slave *slaves[NUM_SLAVES_MAX];

    unsigned short flgEnabledSlaves;

  public:
    SlavesList()
    {
      flgEnabledSlaves=0;
      for (int i=0;i<NUM_SLAVES_MAX;i++)
      {
    	if (i==0)
    	  slaves[i]=(Slave *)new Pump(1);
    	else
    	  slaves[i]=(Slave *)new Oya(1+i);
      }
    }

    ~SlavesList()
    {
      for (int i = 0; i < NUM_SLAVES_MAX; i++)
        delete slaves[i];
    }

    void updCycleStats(void)
    {
      for (int i=0;i<NUM_SLAVES_MAX;i++)
      {
        unsigned short msk=1<<i;
        if ((flgEnabledSlaves&msk)==msk)
          slaves[i]->updCycleStats();
      }
    }

    void config_slaves(unsigned short ens)
    {
      flgEnabledSlaves=ens;
    }

    unsigned short enabled_slaves(void)
    {
      return flgEnabledSlaves;
    }

    void init_all(void)
    {
      for (int i=0;i<NUM_SLAVES_MAX;i++)
    	slaves[i]->init();
    }

    Slave *getSlave(unsigned char addr)
    {
      if ((addr<1) || (addr>NUM_SLAVES_MAX) )
    	  return nullptr;

      unsigned short mask=0x01<< (addr-1);

      if ((mask&flgEnabledSlaves)==mask)
    	return slaves[addr-1];
      else
    	return nullptr;
    }

    Pump *getPump(void)
    {
      return (Pump *)slaves[0];
    }

    Oya *getOya(char addr)
    {
      if ( (addr<2) || (addr>NUM_SLAVES_MAX-1) )
    	  return nullptr;

      return (Oya *)getSlave(addr);
    }

    Slave * findFirstSlave(int &pos)
    {
      pos=-1;
      return findNextSlave(pos);
    }

    Slave * findNextSlave(int &pos)
    {
      if (pos<-1)
    	  return nullptr;

      while (pos<NUM_SLAVES_MAX)
      {
    	  pos++;
        unsigned short mask=0x01<< pos;
        if ((mask&flgEnabledSlaves)==mask)
          return slaves[pos];
      }

      return nullptr;
    }

    Oya * findFirstOya(int &pos)
    {
      pos=0;
      return findNextOya(pos);
    }

    Oya * findNextOya(int &pos)
    {
      if (pos<0)
    	  return nullptr;

      return (Oya *)findNextSlave(pos);
    }
};

#endif
