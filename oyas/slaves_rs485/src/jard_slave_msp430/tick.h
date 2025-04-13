/**
 * @file tick.h
 * @brief Gestion du tick et des tempos
 */
#ifndef TICK_H_
#define TICK_H_

#define TICK_1MS        (0x0001)
#define TICK_250MS      (0x0002)
#define TICK_500MS      (0x0004)
#define TICK_1S         (0x0008)
#define TICK_2S         (0x0010)
#define TICK_4S         (0x0020)
#define TICK_8S         (0x0040)
#define TICK_16S        (0x0080)

extern volatile unsigned long g_tick_1ms;
extern volatile unsigned long g_tick_1s;

extern volatile unsigned short g_tick_flgs;

extern void tick_init(void);

extern unsigned long tick_get_ms(void);
extern unsigned long tick_get_s(void);
extern unsigned long tick_delta_ms(unsigned long t0_ms);
extern unsigned long tick_delta_s(unsigned long t0_s);

extern void tick_delay_ms(unsigned long delay_ms);

#endif
