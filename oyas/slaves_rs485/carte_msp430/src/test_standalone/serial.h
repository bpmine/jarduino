/** @file serial.h
 * @brief Gestion du port serie
 */
#ifndef SERIAL_H_
#define SERIAL_H_

extern void serial_init(int speed);

extern void serial_send(unsigned char *datas,unsigned char nb);
extern void serial_tick(void);

extern void serial_send_oya(unsigned char status,unsigned short tick_ms,unsigned short volt,unsigned short time_s,unsigned short errs);
extern void serial_send_pump(unsigned char status,unsigned short tick_ms,unsigned short volt,unsigned short time_s,unsigned short errs,unsigned short flow);
extern void serial_send_pong(unsigned char val);

#endif
