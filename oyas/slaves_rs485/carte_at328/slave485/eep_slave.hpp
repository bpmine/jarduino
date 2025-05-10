#ifndef EEP_SLAVE_HEADER_INCLUDED
#define EEP_SLAVE_HEADER_INCLUDED

#include <EEPROM.h>


#define EEP_ADDR_NODE_M1      (0)
#define EEP_ADDR_NODE_ADDR    (1)
#define EEP_ADDR_TIME_CTR     (2)
#define EEP_ADDR_ERR_CTR      (4)
#define EEP_ADDR_NODE_M2      (6)

class Eep
{
  private:
    static void eep_write_us(int addr,unsigned short value)
    {
      uint8_t a=(value>>8)&0xFF;
      uint8_t b=value&0xFF;
    
      EEPROM.write(addr,a);
      EEPROM.write(addr+1,b);
    }
    
    static unsigned short eep_read_us(int addr)
    {
      uint8_t a=EEPROM.read(addr);
      uint8_t b=EEPROM.read(addr+1);
    
      unsigned short us= ((uint32_t)a<<8) | (uint32_t)b;
    
      return us;
    }
  
  public:
    static bool readID(uint8_t *pAddr)
    {
      uint8_t m1=EEPROM.read(EEP_ADDR_NODE_M1);
      uint8_t m2=EEPROM.read(EEP_ADDR_NODE_M2);

      *pAddr='Z';
      if ( (m1==0x55) && (m2==0xAA) )
      {
        *pAddr=EEPROM.read(EEP_ADDR_NODE_ADDR);
        return true;        
      }

      writeID('Z');
      writeTotalTimeCtr(0);
      writeTotalErrCtr(0);

      return false;
    }

    static void writeID(uint8_t addr)
    {
      EEPROM.write(EEP_ADDR_NODE_M1,0x55);
      EEPROM.write(EEP_ADDR_NODE_ADDR,addr);
      EEPROM.write(EEP_ADDR_NODE_M2,0xAA);
    }

    static uint16_t readTotalTimeCtr(void)
    {
      return Eep::eep_read_us(EEP_ADDR_TIME_CTR);
    }

    static void writeTotalTimeCtr(uint16_t tps_s)
    {
      Eep::eep_write_us(EEP_ADDR_TIME_CTR,tps_s);    
    }

    static uint16_t readTotalErrCtr(void)
    {
      return Eep::eep_read_us(EEP_ADDR_ERR_CTR);
    }

    static void writeTotalErrCtr(uint16_t tps_s)
    {
      Eep::eep_write_us(EEP_ADDR_ERR_CTR,tps_s);
    }
};

#endif
