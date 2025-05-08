#include <msp430.h> 

#define PIN_LED1 (0x08)

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

inline void micro_init(void)
{
    _DINT();                        ///< Disable interrupts
    WDTCTL = WDTPW+WDTHOLD;         ///< Disable watchdog

    P2DIR = PIN_LED1;               ///< Juste la LED en sortie
    P2SEL = 0x00;
    P2OUT = PIN_LED1;               ///< LED à 1 au démarrage

    //micro_set_internal_1MHz();
    micro_set_external_8MHz();
}


void _wait(unsigned long nb)
{
    unsigned long l;
    for (l=nb; l>0; l--)
    {
        __no_operation();
    }
}

/**
 * main.c
 */
int main(void)
{
    micro_init();
    while ( 1 )
    {
        P2OUT |= 0x08;
        _wait(1000000L);
        P2OUT &= ~0x08;
        _wait(1000000L);
    }
}
