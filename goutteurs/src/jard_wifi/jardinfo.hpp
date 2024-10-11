#ifndef HEADER_INCLUDED_JARDINFO
#define HEADER_INCLUDED_JARDINFO

#include "pumpinfo.hpp"

class JardInfo
{
  public:
    unsigned short version_jarduino=0;
    unsigned short serial=0;
    unsigned short version_soft=0;
    
    unsigned short batt_dxV=0;
    unsigned short sun_dxV=0;
    unsigned short temp_deg=0;
    unsigned short hum_pc=0;
    
    unsigned short year=0;
    unsigned short month=0;
    unsigned short day=0;
    unsigned short hour=0;
    unsigned short minute=0;
    unsigned short second=0;

    PumpInfo pump1;
    PumpInfo pump2;
    
    JardInfo()
    {
    }

    void makeJson(char *strJson)
    {
      char strDate[40];
      sprintf(strDate,"%04d-%02d-%02dT%02d:%02d:%02dZ",year,month,day,hour,minute,second);
      
      strcpy(strJson,"");      
      strcat(strJson,"{");
      
      addJsonVarStr(strJson,"date",strDate);
      strcat(strJson,",");
      addJsonVarInt(strJson,"batt",batt_dxV);
      strcat(strJson,",");
      addJsonVarInt(strJson,"sun",sun_dxV);
      strcat(strJson,",");
      addJsonVarInt(strJson,"temp",temp_deg);
      strcat(strJson,",");
      addJsonVarInt(strJson,"hum",hum_pc);
      strcat(strJson,",");
      strcat(strJson,"{");
      strcat(strJson,"pump1:");
      addJsonPump(strJson,pump1);
      strcat(strJson,",");
      strcat(strJson,"pump2:");
      addJsonPump(strJson,pump2);
      strcat(strJson,"}");
      
      strcat(strJson,"}");
    }

    private:
      void addJsonVarInt(char *strBuffer,char *strName,int iVal)
      {
        char tmp[100];
        sprintf(tmp,"\"%s\":%d",strName,iVal);
        strcat(strBuffer,tmp);
      }
  
      void addJsonVarStr(char *strBuffer,char *strName,char *strVal)
      {
        char tmp[100];
        sprintf(tmp,"\"%s\":\"%s\"",strName,strVal);
        strcat(strBuffer,tmp);
      }

      void addJsonPump(char *strJson,PumpInfo &pump)
      {        
        strcat(strJson,"{");
        addJsonVarInt(strJson,"force",pump.flgForce);
        strcat(strJson,",");        
        addJsonVarInt(strJson,"enabled",pump.flgEnabled);
        strcat(strJson,",");        
        addJsonVarInt(strJson,"auto",pump.flgAuto);
        strcat(strJson,",");        
        addJsonVarInt(strJson,"remote",pump.flgRemote);
        strcat(strJson,",");        
        addJsonVarInt(strJson,"cmd_remote",pump.flgCmdRemote);
        strcat(strJson,"}");        
      }
};

#endif
