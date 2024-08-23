/**
 * @file api.h
 * @brief EN-TETE - API du maître des OYAs
 * */
#ifndef APP_HEADER_INCLUDED
#define APP_HEADER_INCLUDED

class Pump;
class Oya;
class Slave;
class SlavesList;
class Data;

extern void api_trace(bool en);

extern void api_master(bool on);
extern Pump *api_get_pump(void);
extern Oya *api_find_first_oya(int &pos);
extern Oya *api_find_next_oya(int &pos);
extern Oya *api_get_oya(int addr);

extern void api_set_pompe(bool on);
extern void api_set_oya(char addr,bool on);
extern void api_set_commands(unsigned short cmds);

extern void api_get_date_hour(int &day,int &month,int &year,int &hour,int &minute,int &second);
extern void api_get_date_hour(int &day,int &month,int &year,int &hour,int &minute,int &second,int &dow);
extern void api_set_date(int day,int month,int year);
extern void api_set_hour(int hour,int minute,int second);

extern void api_set_slaves_config(unsigned short config);
extern unsigned short api_get_slaves_config(void);

extern void api_raz_all_time(void);
extern void api_raz_time(unsigned char addr);
extern void api_raz_all_errs(void);
extern void api_ping(unsigned char addr);
extern unsigned short api_get_pong(void);

extern void api_latch_data(Data *pData);

#endif
