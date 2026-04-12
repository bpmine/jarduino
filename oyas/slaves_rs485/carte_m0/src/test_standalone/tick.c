/**
 * @file tick.c
 * @brief Gestion du tick et des tempos pour ATSAMD21G18A
 */

#include "sam.h"
#include "tick.h"

#define _TGL_TICK(mask)     (g_tick_flgs ^= (mask))
#define _TGL_TEMP(mask)     (_tick_flgs_div ^= (mask))
#define _IS_TEMP(mask)      (_tick_flgs_div & (mask))

volatile unsigned long g_tick_1ms = 0;
volatile unsigned long g_tick_1s = 0;
volatile unsigned short g_tick_flgs = 0;

volatile unsigned short _count_til_250ms = 0;
volatile unsigned short _tick_flgs_div = 0;

volatile unsigned char g_tick_1ms_lock = 0;
volatile unsigned long g_tick_1ms_latched = 0;
volatile unsigned char g_tick_1s_lock = 0;
volatile unsigned long g_tick_1s_latched = 0;


static inline void tick_tc3_wait_sync(void)
{
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY)
    {
    }
}


void TC3_Handler(void)
{
    if (TC3->COUNT16.INTFLAG.bit.MC0)
    {
        TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;

        g_tick_1ms++;
        _count_til_250ms++;

        _TGL_TICK(TICK_1MS);

        if (g_tick_1ms_lock == 0)
            g_tick_1ms_latched = g_tick_1ms;

        if (g_tick_1s_lock == 0)
            g_tick_1s_latched = g_tick_1s;

        if (_count_til_250ms >= 250)
        {
            _TGL_TICK(TICK_250MS);
            _count_til_250ms = 0;

            /* On propage les flg de ticks... */
            unsigned short changed = TICK_500MS;
            int i;
            for (i = 2; i < 7; i++)
            {
                if (changed & (1 << i))
                {
                    if (_IS_TEMP(1 << i))
                    {
                        _TGL_TICK(1 << i);
                        changed |= (1 << (i + 1));

                        if ((1 << i) == TICK_1S)
                            g_tick_1s++;
                    }
                    _TGL_TEMP(1 << i);
                }
            }
        }
    }
}


void tick_init(void)
{
    /* Horloge périphérique TC3 */
    PM->APBCMASK.reg |= PM_APBCMASK_TC3;

    /* GCLK0 vers TC3/TC4 */
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TCC2_TC3 |
                        GCLK_CLKCTRL_GEN_GCLK0 |
                        GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
    }

    /* Reset TC3 */
    TC3->COUNT16.CTRLA.bit.SWRST = 1;
    tick_tc3_wait_sync();

    /* Mode 16 bits, prescaler /64, match frequency */
    TC3->COUNT16.CTRLA.reg =
        TC_CTRLA_MODE_COUNT16 |
        TC_CTRLA_PRESCALER_DIV64 |
        TC_CTRLA_WAVEGEN_MFRQ;
    tick_tc3_wait_sync();

    /* 
     * 8 MHz / 64 = 125 kHz
     * 1 ms => 125 coups
     * En MFRQ, période = CC0
     */
    TC3->COUNT16.CC[0].reg = 124;
    tick_tc3_wait_sync();

    /* Interruption sur compare match canal 0 */
    TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC0;

    NVIC_ClearPendingIRQ(TC3_IRQn);
    NVIC_EnableIRQ(TC3_IRQn);

    /* Init variables */
    g_tick_1ms = 0;
    g_tick_1s = 0;
    g_tick_flgs = 0;

    _count_til_250ms = 0;
    _tick_flgs_div = 0;

    g_tick_1ms_lock = 0;
    g_tick_1ms_latched = 0;
    g_tick_1s_lock = 0;
    g_tick_1s_latched = 0;

    /* Start timer */
    TC3->COUNT16.CTRLA.bit.ENABLE = 1;
    tick_tc3_wait_sync();
}


unsigned long tick_get_ms(void)
{
    g_tick_1ms_lock = 1;
    unsigned long l = g_tick_1ms_latched;
    g_tick_1ms_lock = 0;

    return l;
}


unsigned long tick_get_s(void)
{
    g_tick_1s_lock = 1;
    unsigned long l = g_tick_1s_latched;
    g_tick_1s_lock = 0;

    return l;
}


unsigned long tick_delta_ms(unsigned long t0_ms)
{
    g_tick_1ms_lock = 1;
    unsigned long t = g_tick_1ms_latched;
    g_tick_1ms_lock = 0;

    if (t >= t0_ms)
        return t - t0_ms;
    else
        return (0xFFFFFFFFUL - t0_ms + 1UL) + t;
}


unsigned long tick_delta_s(unsigned long t0_s)
{
    g_tick_1s_lock = 1;
    unsigned long t = g_tick_1s_latched;
    g_tick_1s_lock = 0;

    if (t >= t0_s)
        return t - t0_s;
    else
        return (0xFFFFFFFFUL - t0_s + 1UL) + t;
}


void tick_delay_ms(unsigned long delay_ms)
{
    volatile unsigned long t0 = tick_get_ms();
    volatile unsigned long delta_ms = 0;

    while (delta_ms < delay_ms)
    {
        delta_ms = tick_delta_ms(t0);
    }
}


void tick_delay_s(unsigned long delay_s)
{
    volatile unsigned long t0 = tick_get_s();
    volatile unsigned long delta_s = 0;

    while (delta_s < delay_s)
    {
        delta_s = tick_delta_s(t0);
    }
}