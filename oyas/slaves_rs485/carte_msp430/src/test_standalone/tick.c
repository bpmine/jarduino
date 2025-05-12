/**
 * @file tick.c
 * @brief Gestion du tick et des tempos
 */

#include <msp430.h>
#include "tick.h"

#define _TGL_TICK(mask)     g_tick_flgs = g_tick_flgs ^ (mask)
#define _TGL_TEMP(mask)     _tick_flgs_div = _tick_flgs_div ^ (mask)
#define _IS_TEMP(mask)      _tick_flgs_div & (mask)

volatile unsigned long g_tick_1ms=0;
volatile unsigned long g_tick_1s=0;
volatile unsigned short g_tick_flgs=0;

volatile unsigned short _count_til_250ms=0;
volatile unsigned short _tick_flgs_div=0;

volatile unsigned char g_tick_1ms_lock=0;
volatile unsigned long g_tick_1ms_latched=0;
volatile unsigned char g_tick_1s_lock=0;
volatile unsigned long g_tick_1s_latched=0;

#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A_ISR(void)
{
    g_tick_1ms++;
    _count_til_250ms++;

    _TGL_TICK(TICK_1MS);

    if (g_tick_1ms_lock==0)
        g_tick_1ms_latched=g_tick_1ms;

    if (g_tick_1s_lock==0)
        g_tick_1s_latched=g_tick_1s;

    if (_count_til_250ms >= 250)
    {
        _TGL_TICK(TICK_250MS);
        _count_til_250ms = 0;

        /// On propage les flg de ticks...
        unsigned short changed = TICK_500MS;
        int i;
        for (i=2; i<7; i++)
        {
            if (changed & (1<<i))
            {
                if (_IS_TEMP(1<<i))
                {
                    _TGL_TICK(1<<i);
                    changed |= (1<<(i+1));

                    if ( (1<<i) == TICK_1S)
                        g_tick_1s++;
                }
                _TGL_TEMP(1<<i);
            }
        }
    }
}

void tick_init(void)
{
    TACTL = TASSEL_2 | ID_3 | MC_1;  // SMCLK, divisé par 8, mode Up
    TACCR0 = 620;  // (1ms avec SMCLK = 1MHz et division par 8)
    TACCTL0 = CCIE; // Activation de l'interruption sur TACCR0

    g_tick_1ms=0;
    g_tick_1s=0;
    g_tick_flgs=0;

    _count_til_250ms=0;
    _tick_flgs_div=0;

    g_tick_1ms_lock=0;
    g_tick_1ms_latched=0;
    g_tick_1s_lock=0;
    g_tick_1s_latched=0;
}

unsigned long tick_get_ms(void)
{
    g_tick_1ms_lock=1;
    unsigned long l=g_tick_1ms_latched;
    g_tick_1ms_lock=0;

    return l;
}

unsigned long tick_get_s(void)
{
    g_tick_1s_lock=1;
    unsigned long l=g_tick_1s_latched;
    g_tick_1s_lock=0;

    return l;
}

unsigned long tick_delta_ms(unsigned long t0_ms)
{
    g_tick_1ms_lock=1;
    unsigned long t=g_tick_1ms_latched;
    g_tick_1ms_lock=0;
    if (t>=t0_ms)
        return t-t0_ms;
    else
        return (0xFFFFFFFF-t0_ms+1)+t;
}

unsigned long tick_delta_s(unsigned long t0_s)
{
    g_tick_1s_lock=1;
    unsigned long t=g_tick_1s_latched;
    g_tick_1s_lock=0;
    if (t>=t0_s)
        return t-t0_s;
    else
        return (0xFFFFFFFF-t0_s+1)+t;
}


void tick_delay_ms(unsigned long delay_ms)
{
    volatile unsigned long t0=tick_get_ms();
    volatile unsigned long delta_ms=0;
    while (delta_ms<delay_ms)
    {
        delta_ms=tick_delta_ms(t0);
    }
}

void tick_delay_s(unsigned long delay_s)
{
    volatile unsigned long t0=tick_get_s();
    volatile unsigned long delta_s=0;
    while (delta_s<delay_s)
    {
        delta_s=tick_delta_s(t0);
    }
}
