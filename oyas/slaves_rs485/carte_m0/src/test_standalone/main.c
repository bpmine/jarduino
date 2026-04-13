#include "sam.h"

#include "tick.h"
#include "serial.h"
#include "pins.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>

volatile unsigned char g_slaveAddr=0;
volatile unsigned char g_High=0;
volatile unsigned char g_Low=0;
volatile unsigned char g_cmd=0;

volatile unsigned char g_cycle_test=0;
volatile unsigned short g_power_dv=0;

char msg[70]="";



inline void micro_set_internal_8MHz(void)
{
	/* Activer OSC8M (normalement déjà actif au reset) */
	SYSCTRL->OSC8M.bit.ENABLE = 1;

	/* Prescaler = 1 ? 8 MHz */
	SYSCTRL->OSC8M.bit.PRESC = 0;

	/* Attendre stabilité */
	while (!SYSCTRL->PCLKSR.bit.OSC8MRDY);
}

inline unsigned char read_address(void)
{
    unsigned char a1 = gpio_read(PIN_ADDR_A1) ? 0 : 1;
    unsigned char a2 = gpio_read(PIN_ADDR_A2) ? 0 : 1;
    unsigned char a3 = gpio_read(PIN_ADDR_A3) ? 0 : 1;
    unsigned char a4 = gpio_read(PIN_ADDR_A4) ? 0 : 1;

    unsigned char addr = 0;

    addr |= (a1 << 0);
    addr |= (a2 << 1);
    addr |= (a3 << 2);
    addr |= (a4 << 3);

    return addr;
}

static inline void adc_wait_sync(void)
{
	while (ADC->STATUS.bit.SYNCBUSY)
	{
	}
}

void adc_init(void)
{
    /* PIN_MES_V doit être une broche analogique valide */
    gpio_mode_input(PIN_MES_V);
    gpio_set_pmux(PIN_MES_V, GPIO_PMUX_B);

    PM->APBCMASK.reg |= PM_APBCMASK_ADC;

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC |
    GCLK_CLKCTRL_GEN_GCLK0 |
    GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
    }

    ADC->CTRLA.bit.SWRST = 1;
    adc_wait_sync();

    ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;
    adc_wait_sync();

    ADC->AVGCTRL.reg = 0;
    adc_wait_sync();

    ADC->SAMPCTRL.reg = 5;
    adc_wait_sync();

    ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV64 |
    ADC_CTRLB_RESSEL_10BIT;
    adc_wait_sync();

    ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND |
    ADC_INPUTCTRL_MUXPOS(PIN_MES_V_ADC_MUXPOS);
    adc_wait_sync();

    ADC->CTRLA.bit.ENABLE = 1;
    adc_wait_sync();
}


unsigned short adc_voltage_read(void)
{
    unsigned short ret = 0;
    unsigned short val = 0;

    ADC->SWTRIG.bit.START = 1;
    while (!ADC->INTFLAG.bit.RESRDY)
    {
    }

    val = (unsigned short)ADC->RESULT.reg;
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

    /* Conserver la formule métier existante temporairement */
    //ret = (unsigned short)((val * 120UL) / 778UL);
	ret=val;

    return ret;
}

inline static void micro_init(void)
{
    __disable_irq();

    /* Sorties */
    gpio_output_low(PIN_TX_EN);
    gpio_output_low(PIN_CMD_EV);

    /* Entrées capteurs */
    gpio_input_plain(PIN_CPT_LVL_HIGH);
    gpio_input_plain(PIN_CPT_LVL_LOW);

    /* Entrées adresse */
    gpio_pullup(PIN_ADDR_A1);
    gpio_pullup(PIN_ADDR_A2);
    gpio_pullup(PIN_ADDR_A3);
    gpio_pullup(PIN_ADDR_A4);

    /* LED à 1 au démarrage */
    gpio_output_high(PIN_LED1);

    micro_set_internal_8MHz();

    serial_init(9600);
    tick_init();

    adc_init();

    __enable_irq();
}

void _boot_blink(void)
{
    int i;
    for (i = 0; i < 5; i++)
    {
	    gpio_set(PIN_LED1);
	    tick_delay_ms(150);
	    gpio_clear(PIN_LED1);
	    tick_delay_ms(150);
    }

    gpio_set(PIN_LED1);
    tick_delay_ms(500);
}

void ustoa(char *buffer,unsigned short value)
{
    char temp[7]=""; // Contient la chaîne dans l'ordre de l'evaluation
    int i=0;
    int j=0;

    if (value == 0 )
    {
        buffer[0] = '0';
        buffer[1] = 0;
        return;
    }

    if (value>999)
        value=999;

    while (value > 0)
    {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }

    // inverser la chaîne
    for ( j = 0; j < i; j++)
        buffer[j] = temp[i - j - 1];

    buffer[i] = 0;
}


/**
 * main.c
 */
int main(void)
 {
    micro_init();

    g_High=0;
    g_Low=0;
    g_cmd=0;
    g_power_dv=0;

    g_slaveAddr=read_address();
    while ( (g_slaveAddr==0) || (g_slaveAddr==15) )
    {
        if ( (g_tick_flgs&TICK_2S) && (g_tick_flgs&TICK_1S) )
            gpio_set(PIN_LED1);
        else
            gpio_clear(PIN_LED1);

        g_slaveAddr=read_address();
    }

    _boot_blink();

	while (1)
	{
        g_High = gpio_read(PIN_CPT_LVL_HIGH) ? 1 : 0;
        g_Low  = gpio_read(PIN_CPT_LVL_LOW)  ? 1 : 0;
        g_power_dv=adc_voltage_read();

        g_cmd=!g_High;

        if (IS_TICK_2S)
	    {
	        if (g_cycle_test==1)
	        {
	            char x[2];

	            g_slaveAddr=read_address();
	            if (g_slaveAddr<10)
	            {
	                x[0]='0'+g_slaveAddr;
	            }
	            else
	            {
	                x[0]='A'+g_slaveAddr-10;
	            }
	            x[1]=0;

	            strcpy(msg,"Addr: ");
	            strcat(msg,x);
	            strcat(msg,"\n\r");

	            strcat(msg,"L: ");
	            if (g_Low==1) strcat(msg,"1"); else strcat(msg,"0");
                strcat(msg,"\n\r");

	            strcat(msg,"H: ");
	            if (g_High==1) strcat(msg,"1"); else strcat(msg,"0");
                strcat(msg,"\n\r");

	            strcat(msg,"C: ");
	            if (g_cmd==1) strcat(msg,"1"); else strcat(msg,"0");
                strcat(msg,"\n\r");

                char strVal[10]="";
                ustoa(strVal,g_power_dv);
                strcat(msg,"V: ");
                strcat(msg,strVal);
                strcat(msg,"\n\r");

	            strcat(msg,"_____\n\r");
	            serial_send((unsigned char *)msg,strlen(msg));

	            g_cycle_test=0;
	        }
	    }
	    else
	    {
	        g_cycle_test=1;
	    }

        if (g_High)
        {
            gpio_set(PIN_LED1);
        }
        else if (g_Low)
        {
            if (IS_TICK_250MS)
                gpio_set(PIN_LED1);
            else
                gpio_clear(PIN_LED1);
        }
        else
        {
            gpio_clear(PIN_LED1);
        }

        if (g_cmd==1)
		{
            gpio_set(PIN_CMD_EV);
		}
        else
		{
            gpio_clear(PIN_CMD_EV);
		}

	    serial_tick();
	}
}
