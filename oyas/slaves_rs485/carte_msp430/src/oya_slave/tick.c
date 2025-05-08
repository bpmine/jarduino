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

#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A_ISR(void)
{
    g_tick_1ms++;
    _count_til_250ms++;

    _TGL_TICK(TICK_1MS);

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
}

unsigned long tick_get_ms(void)
{
    return g_tick_1ms;
}

unsigned long tick_get_s(void)
{
    return g_tick_1s;
}

unsigned long tick_delta_ms(unsigned long t0_ms)
{
    unsigned long t=g_tick_1ms;
    if (t>t0_ms)
        return t-t0_ms;
    else
        return 0xFFFFFFFF-t0_ms+t;
}

unsigned long tick_delta_s(unsigned long t0_s)
{
    unsigned long t=g_tick_1s;
    if (t>t0_s)
        return t-t0_s;
    else
        return 0xFFFFFFFF-t0_s+t;
}


void tick_delay_ms(unsigned long delay_ms)
{
    volatile unsigned long t0=tick_get_ms();
    while (tick_delta_ms(t0) < delay_ms);
}

