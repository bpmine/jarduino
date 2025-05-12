#include <msp430.h> 
#include "tick.h"
#include "serial.h"
#include "pins.h"
#include <stdio.h>
#include <string.h>

volatile unsigned char g_slaveAddr=0;
volatile unsigned char g_High=0;
volatile unsigned char g_Low=0;
volatile unsigned char g_cmd=0;

volatile unsigned char g_cycle_test=0;

inline void micro_set_internal_1MHz(void)
{
    // Clock configuration
    DCOCTL = DCO2|DCO1|DCO0;              /// 7th frequency for DCO (1MHz)
    BCSCTL1 = XT2OFF|RSEL2|RSEL1|RSEL0|XTS;      /// 7th nominal frequency, XT2 off
    BCSCTL2 = 0;
}

inline void micro_set_external_8MHz(void)
{
    BCSCTL1 = XT2OFF | XTS;             ///< Activation de l'horloge externe (XTS)

    // Délai d’attente pour la stabilisation de l'oscillateur
    do {
        IFG1 &= ~OFIFG;       // Effacer le flag de faute d'oscillateur
        __delay_cycles(10000);
    } while (IFG1 & OFIFG);   // Attendre que le flag reste à 0

    BCSCTL2 = SELM_2 | SELS | DIVM_0 | DIVS_0;
}

inline unsigned char read_address(void)
{
    unsigned char tmp=(~(P1IN & PIN_MASK_ADDR)&PIN_MASK_ADDR);
    unsigned char addr=tmp>>1;

    if ((tmp&PIN_ADDR_A4)==PIN_ADDR_A4)
        addr|=0x08;
    else
        addr&=~0x08;

    return addr;
}

inline void micro_init(void)
{
    _DINT();                        ///< Disable interrupts
    WDTCTL = WDTPW+WDTHOLD;         ///< Disable watchdog

    P3DIR = PIN_TX_EN|PIN_CMD_EV;   ///< Low, High en entree et Ev et TX_EN en sortie
    P3SEL = 0x00;                   ///< Normal I/O
    P3OUT = 0x00;                   ///< Port 3 set to 0

    P1DIR = 0xF0;                   ///< A1, A2, A3 et A4 en entree, le reste en sortie
    P1SEL = 0x00;                   ///< Normal I/O
    P1OUT = 0x00;                   ///< Tout à 0

    P2DIR = PIN_LED1;               ///< Juste la LED en sortie
    P2SEL = 0x00;
    P2OUT = PIN_LED1;               ///< LED à 1 au démarrage

    //micro_set_internal_1MHz();
    micro_set_external_8MHz();

    serial_init(9600);
    tick_init();

    _EINT();                    ///< Enable interrupts
}

void _boot_blink(void)
{
    int i;
    for (i=0;i<5;i++)
    {
        P2OUT |= PIN_LED1;
        tick_delay_ms(150);
        P2OUT &= ~PIN_LED1;
        tick_delay_ms(150);
    }

    P2OUT |= PIN_LED1;
    tick_delay_ms(500);
}

char msg[50]="";

/**
 * main.c
 */
int main(void)
 {
    micro_init();

    g_High=0;
    g_Low=0;
    g_cmd=0;


    g_slaveAddr=read_address();
    while ( (g_slaveAddr==0) || (g_slaveAddr==15) )
    {
        if ( (g_tick_flgs&TICK_2S) && (g_tick_flgs&TICK_1S) )
        {
            P2OUT |= PIN_LED1;
        }
        else
        {
            P2OUT &= ~PIN_LED1;
        }
        g_slaveAddr=read_address();
    }

    _boot_blink();

	while (1)
	{
        g_High=(P3IN&PIN_CPT_LVL_HIGH)==PIN_CPT_LVL_HIGH?0:1;
        g_Low=(P3IN&PIN_CPT_LVL_LOW)==PIN_CPT_LVL_LOW?0:1;

        if (g_High==0)
            g_cmd=0;

        if (g_tick_flgs&TICK_1S)
	    {
	        g_cmd=g_High;

	        P2OUT |= PIN_LED1;

	        g_slaveAddr=read_address();

	        if (g_cycle_test==1)
	        {
	            char x[2];

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

	            strcat(msg,"_____\n\r");
	            serial_send((unsigned char *)msg,strlen(msg));

	            g_cycle_test=0;
	        }
	    }
	    else
	    {
	        P2OUT &= ~PIN_LED1;
	        g_cycle_test=1;
	    }

        if (g_cmd==1)
            P3OUT|=PIN_CMD_EV;
        else
            P3OUT &= ~PIN_CMD_EV;

	    serial_tick();
	}
}
