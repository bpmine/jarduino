#include "sam.h"

#include "serial.h"
#include "pins.h"
#include "gpio.h"

#define SER_CT_TX   (0x01)

#define MAX_BUFFER_SIZE (50)

typedef struct
{
    unsigned char data[MAX_BUFFER_SIZE];
    unsigned char size;
    unsigned char pos;
    unsigned char cs;
    unsigned char cs_enabled;
} T_FRAME;

static T_FRAME tx;

static unsigned char _serial_ctrl = 0;
static unsigned char _addr = 0;
static unsigned char _flgPending = 0;


/* =========================================================
 * Helpers SERCOM USART
 * ========================================================= */
static inline void serial_wait_sync(void)
{
    while (SERCOM1->USART.SYNCBUSY.reg)
    {
    }
}

static inline void serial_wait_reset(void)
{
    while (SERCOM1->USART.CTRLA.bit.SWRST || SERCOM1->USART.SYNCBUSY.bit.SWRST)
    {
    }
}

static inline uint8_t serial_tx_complete(void)
{
    return (SERCOM1->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_TXC) ? 1u : 0u;
}

static uint16_t serial_calc_baud(uint32_t fref, uint32_t baud)
{
    uint64_t tmp;

    if (baud == 0)
        baud = 9600;

    tmp = 65536ULL - ((16ULL * 65536ULL * (uint64_t)baud) / (uint64_t)fref);

    if (tmp > 65535ULL)
        tmp = 65535ULL;

    return (uint16_t)tmp;
}


/* =========================================================
 * API
 * ========================================================= */
void serial_send(unsigned char *datas, unsigned char nb)
{
    if (_serial_ctrl & SER_CT_TX)
        return;

    if (nb == 0)
        return;

    if (nb > MAX_BUFFER_SIZE)
        nb = MAX_BUFFER_SIZE;

    gpio_set(PIN_TX_EN);
    _flgPending = 1;

    for (int i = 0; i < nb; i++)
    {
        tx.data[i] = datas[i];
    }

    tx.pos = 1;
    tx.size = nb;
    _serial_ctrl |= SER_CT_TX;

    /* Nettoyer TXC avant de lancer une nouvelle trame */
    SERCOM1->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_TXC;

    /* Envoi du premier octet */
    while (!(SERCOM1->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE))
    {
    }
    SERCOM1->USART.DATA.reg = datas[0];

    /* Activer interruption Data Register Empty */
    SERCOM1->USART.INTENSET.reg = SERCOM_USART_INTENSET_DRE;
}


void serial_tick(void)
{
    if (_flgPending == 2)
    {
        if (serial_tx_complete())
        {
            gpio_clear(PIN_TX_EN);
            _flgPending = 0;

            /* Acquitter TXC */
            SERCOM1->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_TXC;
        }
    }
}


/* =========================================================
 * ISR SERCOM1
 * ========================================================= */
void SERCOM1_Handler(void)
{
    uint8_t flags = SERCOM1->USART.INTFLAG.reg;

    /* RX reçu */
    if (flags & SERCOM_USART_INTFLAG_RXC)
    {
        volatile unsigned char dta = (unsigned char)SERCOM1->USART.DATA.reg;
        (void)dta;
    }

    /* TX DRE */
    if ((flags & SERCOM_USART_INTFLAG_DRE) && (_serial_ctrl & SER_CT_TX))
    {
        if (tx.pos < tx.size)
        {
            SERCOM1->USART.DATA.reg = tx.data[tx.pos];
            tx.pos++;
        }
        else
        {
            tx.size = 0;
            tx.pos = 0;
            _serial_ctrl &= (unsigned char)(~SER_CT_TX);

            /* Stop interruption DRE */
            SERCOM1->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;

            /* On attend maintenant la vraie fin physique d'émission */
            _flgPending = 2;
        }
    }
}


/* =========================================================
 * Init
 * ========================================================= */
void serial_init(int speed)
{
    uint16_t baud_reg;

    if (speed <= 0)
	{
        speed = 9600;
	}

	gpio_configure_uart(PIN_UART_TX, PIN_UART_RX,GPIO_PMUX_SERCOM);

    /* Horloge bus SERCOM1 */
    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;

    /* GCLK0 vers SERCOM1 core */
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_SERCOM1_CORE |
                        GCLK_CLKCTRL_GEN_GCLK0 |
                        GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
    }

    /* Reset */
    SERCOM1->USART.CTRLA.bit.SWRST = 1;
    serial_wait_reset();

    /* USART asynchrone
       RX sur PAD[1], TX sur PAD[0] */
    SERCOM1->USART.CTRLA.reg =
        SERCOM_USART_CTRLA_MODE_USART_INT_CLK |
        SERCOM_USART_CTRLA_DORD |
        SERCOM_USART_CTRLA_RXPO(1) |
        SERCOM_USART_CTRLA_TXPO(0);

    /* 8 bits, no parity, 1 stop, RX/TX enable */
    SERCOM1->USART.CTRLB.reg =
        SERCOM_USART_CTRLB_CHSIZE(0) |
        SERCOM_USART_CTRLB_RXEN |
        SERCOM_USART_CTRLB_TXEN;
    serial_wait_sync();

    /* Baudrate pour GCLK0 = 8 MHz */    
	//baud_reg = serial_calc_baud(8000000UL, (uint32_t)speed);
	SERCOM1->USART.BAUD.reg = 64277;
    //SERCOM1->USART.BAUD.reg = baud_reg;
    serial_wait_sync();

    /* Init buffer/flags */
    for (int i = 0; i < MAX_BUFFER_SIZE; i++)
    {
        tx.data[i] = 0;
    }

    tx.size = 0;
    tx.pos = 0;
    tx.cs = 0;
    tx.cs_enabled = 0;

    _serial_ctrl = 0;
    _addr = 0;
    _flgPending = 0;

    /* Clear flags */
    SERCOM1->USART.INTFLAG.reg =
        SERCOM_USART_INTFLAG_RXC |
        SERCOM_USART_INTFLAG_TXC |
        SERCOM_USART_INTFLAG_DRE;

    /* Activer interruption RX */
    SERCOM1->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;

    /* Enable USART */
    SERCOM1->USART.CTRLA.bit.ENABLE = 1;
    serial_wait_sync();

    NVIC_ClearPendingIRQ(SERCOM1_IRQn);
    NVIC_EnableIRQ(SERCOM1_IRQn);
}