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

#define IS_TICK_250MS   ( (g_tick_flgs&TICK_250MS) == TICK_250MS )
#define IS_TICK_500MS   ( (g_tick_flgs&TICK_500MS) == TICK_500MS )
#define IS_TICK_1S      ( (g_tick_flgs&TICK_1S) == TICK_1S )
#define IS_TICK_2S      ( (g_tick_flgs&TICK_2S) == TICK_2S )
#define IS_TICK_4S      ( (g_tick_flgs&TICK_4S) == TICK_4S )

#define IS_RISE_250MS   ( (g_edges_flgs&TICK_250MS) == TICK_250MS )
#define IS_RISE_500MS   ( (g_edges_flgs&TICK_500MS) == TICK_500MS )
#define IS_RISE_1S      ( (g_edges_flgs&TICK_1S) == TICK_1S )
#define IS_RISE_2S      ( (g_edges_flgs&TICK_2S) == TICK_2S )
#define IS_RISE_4S      ( (g_edges_flgs&TICK_4S) == TICK_4S )

extern volatile unsigned short g_tick_flgs;
extern volatile unsigned short g_edges_flgs;

extern void tick_init(void);

extern unsigned long tick_get_ms(void);
extern unsigned long tick_get_s(void);
extern unsigned long tick_delta_ms(unsigned long t0_ms);
extern unsigned long tick_delta_s(unsigned long t0_s);

extern void tick_delay_ms(unsigned long delay_ms);
extern void tick_delay_s(unsigned long delay_s);

extern void tick_cycle(void);

#endif
