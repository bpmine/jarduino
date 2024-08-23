/**
 * @file wificomm.h
 * @brief EN-TETE - GESTION DE LA COMMUNICATION WIFI/MAITRE OYA
 **/
#ifndef COMM_HEADER_INCLUDED
#define COMM_HEADER_INCLUDED

#define COMM_BUFFER_MAX_SIZE    (255)
#define TIMEOUT_MQTT            (8000)
#define TIMEOUT_MASTER          (800)

#include "timer.h"

class HardwareSerial;
class WifiComm
{
  private:
    HardwareSerial* pStr;
    
    unsigned char buffer[COMM_BUFFER_MAX_SIZE];
    int pos;
   
    Timer tmrMqtt= Timer(TIMEOUT_MQTT);
    Timer tmrMaster= Timer(TIMEOUT_MASTER);
    bool flgMqtt_ok;
    bool flgMaster_ok;

    void OnRecvFromMaster(char *msg);
    void recv_management(void);    
    
  public:
    WifiComm();

    void begin(HardwareSerial* pSerial);
    void loop(void);

    void sendMsgToMaster(const char *msg);

    bool isMqttOk(void);
    bool isMasterOk(void);
};

extern WifiComm Comm;

#endif
