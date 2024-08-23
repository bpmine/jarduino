/**
 * @file wificomm.h
 * @brief EN-TETE - GESTION DE LA COMMUNICATION WIFI
 **/
#ifndef COMM_HEADER_INCLUDED
#define COMM_HEADER_INCLUDED

#define COMM_BUFFER_MAX_SIZE  (255)
#define TIMEOUT_WIFI_COMM_MS  (5000UL)
#define TIMEOUT_WIFI_ACK_MS   (5000UL)
#define PERIOD_SEND_DATAS_MS  (10000UL)

#include "timer.h"

class SlavesList;
class Slave;
class Pump;
class Oya;
class HardwareSerial;
class Data;
class WifiComm
{
  private:
    HardwareSerial* pStr;
    unsigned char buffer[COMM_BUFFER_MAX_SIZE];
    int pos;
    Timer tmrAlive= Timer(TIMEOUT_WIFI_COMM_MS);
    Timer tmrRemoteActive= Timer(TIMEOUT_WIFI_COMM_MS);
    bool flgRemoteActive;
    bool flgAlive;
    unsigned short commands;

    Timer tmrSendAck= Timer(TIMEOUT_WIFI_ACK_MS);
    Timer tmrSendData= Timer(PERIOD_SEND_DATAS_MS,false);

    void pubDataInfo(void);

    void execCommands(unsigned short cmds,bool ctrl);
    void recv(void);
    void sendTask(void);

  public:
    WifiComm();

    void begin(HardwareSerial* pSerial);
    void setPower(bool on);
    void loop(void);

    bool isRemoteActive(void);
    bool isAlive(void);
    unsigned short getCommands(void);

    void sendData(void);
};

extern WifiComm Comm;

#endif
