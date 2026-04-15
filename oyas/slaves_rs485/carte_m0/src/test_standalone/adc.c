#include "adc.h"
#include "gpio.h"
#include "pins.h"

#include "sam.h"

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
	
	ret = (unsigned short)((val * 120UL) / 652UL);

	return ret;
}
