#include "sam.h"

void clock_init(void)
{
	/* Activer OSC8M (normalement déjà actif au reset) */
	SYSCTRL->OSC8M.bit.ENABLE = 1;

	/* Prescaler = 1 ? 8 MHz */
	SYSCTRL->OSC8M.bit.PRESC = 0;

	/* Attendre stabilité */
	while (!SYSCTRL->PCLKSR.bit.OSC8MRDY);
}

void port_init(void)
{
	/* Activer le port */
	PM->APBBMASK.reg |= PM_APBBMASK_PORT;

	/* PA20 en sortie */
	PORT->Group[0].DIRSET.reg = (1 << 20);

	/* Mettre à 1 immédiatement */
	PORT->Group[0].OUTSET.reg = (1 << 20);
	
	/* Désactiver le multiplexage (mettre en GPIO pur) */
	PORT->Group[0].PINCFG[20].bit.PMUXEN = 0;

	/* Optionnel mais propre */
	PORT->Group[0].PINCFG[20].bit.INEN = 0;
	PORT->Group[0].PINCFG[20].bit.PULLEN = 0;	
}

void delay(void)
{
	/* Délai approximatif ~500 ms à 8 MHz */
	for (volatile uint32_t i = 0; i < 200000; i++)
	{
		__asm__("nop");
	}
}

int main(void)
{
	SystemInit();   // init CMSIS de base

	clock_init();
	port_init();

	while (1)
	{
		PORT->Group[0].OUTTGL.reg = (1 << 20); // toggle PA20
		delay();
	}
}