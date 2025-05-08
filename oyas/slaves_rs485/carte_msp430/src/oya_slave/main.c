#include <msp430.h> 
#include "prot.h"
#include "tick.h"
#include "serial.h"
#include "frames.h"
#include "pins.h"

unsigned char g_slaveAddr=0;

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
    unsigned char tmp=P1IN & PIN_MASK_ADDR;
    unsigned char addr=tmp>>1;

    if ((tmp&PIN_ADDR_A4)==PIN_ADDR_A4)
        addr|=0x08;
    else
        addr&=~0x08;

    //return addr;
    return 1;
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

    P3SEL &= ~BIT3;
    P3DIR |= BIT3;
    P3OUT &= ~BIT3;

    _EINT();                    ///< Enable interrupts
}



void frames_on_receive_sync(void)
{

}

void frames_on_receive_cmds(unsigned short cmds,unsigned char addr)
{

}

void frames_on_receive_ping(unsigned char val)
{

}


/**
 * main.c
 */
int main(void)
{
    micro_init();
    int done=1;

    g_slaveAddr=read_address();
    while ( (g_slaveAddr==0) || (g_slaveAddr==15) )
    {
        if ( (g_tick_flgs&TICK_2S) && (g_tick_flgs&TICK_1S) )
        {
            P2OUT |= 0x08;
        }
        else
        {
            P2OUT &= ~0x08;
        }
        g_slaveAddr=read_address();
    }

    int i;
    for (i=0;i<10;i++)
    {
        P2OUT |= 0x08;
        tick_delay_ms(400);
        P2OUT &= ~0x08;
        tick_delay_ms(500);
    }

	while (1)
	{
	    if (g_tick_flgs&TICK_1S)
	    {
	        P3OUT |= BIT3;
	        P2OUT |= 0x08;

	        if (done==1)
	        {
	            done=0;
	            serial_send("ABC",3);
	        }
	    }
	    else
	    {
	        P3OUT &= ~BIT3;
	        P2OUT &= ~0x08;
	        done=1;
	    }

	    serial_tick();
	}
}
