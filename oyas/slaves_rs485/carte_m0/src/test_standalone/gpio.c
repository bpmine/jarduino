#include "gpio.h"

const gpio_desc_t g_gpio_table[GPIO_COUNT] =
{
	{0,  0}, {0,  1}, {0,  2}, {0,  3}, {0,  4}, {0,  5}, {0,  6}, {0,  7},
	{0,  8}, {0,  9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15},
	{0, 16}, {0, 17}, {0, 18}, {0, 19}, {0, 20}, {0, 21}, {0, 22}, {0, 23},
	{0, 24}, {0, 25}, {0, 26}, {0, 27}, {0, 28}, {0, 29}, {0, 30}, {0, 31},

	{1,  0}, {1,  1}, {1,  2}, {1,  3}, {1,  4}, {1,  5}, {1,  6}, {1,  7},
	{1,  8}, {1,  9}, {1, 10}, {1, 11}, {1, 12}, {1, 13}, {1, 14}, {1, 15},
	{1, 16}, {1, 17}, {1, 18}, {1, 19}, {1, 20}, {1, 21}, {1, 22}, {1, 23},
	{1, 24}, {1, 25}, {1, 26}, {1, 27}, {1, 28}, {1, 29}, {1, 30}, {1, 31}
};

static inline PortGroup *gpio_group(gpio_id_t id)
{
	return &PORT->Group[g_gpio_table[id].port];
}

static inline uint32_t gpio_mask(gpio_id_t id)
{
	return (1u << g_gpio_table[id].pin);
}

void gpio_mode_input(gpio_id_t id)
{
	gpio_group(id)->DIRCLR.reg = gpio_mask(id);
	gpio_group(id)->PINCFG[g_gpio_table[id].pin].bit.INEN = 1;
}

void gpio_mode_output(gpio_id_t id)
{
	gpio_group(id)->DIRSET.reg = gpio_mask(id);
}

void gpio_set(gpio_id_t id)
{
	gpio_group(id)->OUTSET.reg = gpio_mask(id);
}

void gpio_clear(gpio_id_t id)
{
	gpio_group(id)->OUTCLR.reg = gpio_mask(id);
}

void gpio_toggle(gpio_id_t id)
{
	gpio_group(id)->OUTTGL.reg = gpio_mask(id);
}

uint8_t gpio_read(gpio_id_t id)
{
	return (gpio_group(id)->IN.reg & gpio_mask(id)) ? 1u : 0u;
}

void gpio_pullup(gpio_id_t id)
{
	gpio_mode_input(id);
	gpio_group(id)->PINCFG[g_gpio_table[id].pin].bit.PULLEN = 1;
	gpio_group(id)->OUTSET.reg = gpio_mask(id);
}

void gpio_input_plain(gpio_id_t id)
{
	gpio_mode_input(id);
	gpio_group(id)->PINCFG[g_gpio_table[id].pin].bit.PULLEN = 0;
}

void gpio_output_low(gpio_id_t id)
{
	gpio_clear(id);
	gpio_mode_output(id);
}

void gpio_output_high(gpio_id_t id)
{
	gpio_set(id);
	gpio_mode_output(id);
}

void gpio_set_pmux(gpio_id_t id, uint8_t function)
{
	const gpio_desc_t *d = &g_gpio_table[id];

	uint8_t pin = d->pin;
	uint8_t port = d->port;

	uint8_t index = pin >> 1;  // PMUX index (2 pins par registre)

	/* Activer le multiplexage */
	PORT->Group[port].PINCFG[pin].bit.PMUXEN = 1;

	/* E ou O selon pin paire/impaire */
	if ((pin & 1u) == 0)
	{
		/* Pin paire ? PMUXE */
		PORT->Group[port].PMUX[index].bit.PMUXE = function;
	}
	else
	{
		/* Pin impaire ? PMUXO */
		PORT->Group[port].PMUX[index].bit.PMUXO = function;
	}
}

void gpio_configure_uart(gpio_id_t tx, gpio_id_t rx,uint8_t pmux)
{
	
    /* TX en sortie, RX en entrée */
    gpio_mode_output(tx);
    gpio_mode_input(rx);

    /* PMUX vers SERCOM */
    gpio_set_pmux(tx, pmux);
    gpio_set_pmux(rx, pmux);	
}