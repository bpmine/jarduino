#ifndef GPIO_H
#define GPIO_H

#include "sam.h"
#include <stdint.h>

typedef uint8_t gpio_id_t;

typedef struct
{
    uint8_t port;   /* 0 = PORTA, 1 = PORTB */
    uint8_t pin;    /* 0..31 */
} gpio_desc_t;


/* =========================
 * IDs GPIO génériques
 * ========================= */
#define PA00   0
#define PA01   1
#define PA02   2
#define PA03   3
#define PA04   4
#define PA05   5
#define PA06   6
#define PA07   7
#define PA08   8
#define PA09   9
#define PA10   10
#define PA11   11
#define PA12   12
#define PA13   13
#define PA14   14
#define PA15   15
#define PA16   16
#define PA17   17
#define PA18   18
#define PA19   19
#define PA20   20
#define PA21   21
#define PA22   22
#define PA23   23
#define PA24   24
#define PA25   25
#define PA26   26
#define PA27   27
#define PA28   28
#define PA29   29
#define PA30   30
#define PA31   31

#define PB00   32
#define PB01   33
#define PB02   34
#define PB03   35
#define PB04   36
#define PB05   37
#define PB06   38
#define PB07   39
#define PB08   40
#define PB09   41
#define PB10   42
#define PB11   43
#define PB12   44
#define PB13   45
#define PB14   46
#define PB15   47
#define PB16   48
#define PB17   49
#define PB18   50
#define PB19   51
#define PB20   52
#define PB21   53
#define PB22   54
#define PB23   55
#define PB24   56
#define PB25   57
#define PB26   58
#define PB27   59
#define PB28   60
#define PB29   61
#define PB30   62
#define PB31   63

#define GPIO_COUNT 64

#define GPIO_PMUX_A 0
#define GPIO_PMUX_B 1
#define GPIO_PMUX_C 2
#define GPIO_PMUX_D 3
#define GPIO_PMUX_E 4
#define GPIO_PMUX_F 5
#define GPIO_PMUX_G 6
#define GPIO_PMUX_H 7

extern const gpio_desc_t g_gpio_table[GPIO_COUNT];

void gpio_mode_input(gpio_id_t id);
void gpio_mode_output(gpio_id_t id);

void gpio_set(gpio_id_t id);
void gpio_clear(gpio_id_t id);
void gpio_toggle(gpio_id_t id);
uint8_t gpio_read(gpio_id_t id);

void gpio_pullup(gpio_id_t id);
void gpio_input_plain(gpio_id_t id);

void gpio_output_low(gpio_id_t id);
void gpio_output_high(gpio_id_t id);

void gpio_set_pmux(gpio_id_t id, uint8_t function);
void gpio_configure_uart(gpio_id_t tx, gpio_id_t rx,uint8_t pmux);

#endif