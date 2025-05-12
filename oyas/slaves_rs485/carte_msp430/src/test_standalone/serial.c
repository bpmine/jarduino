#include <msp430.h>

#include "serial.h"
#include "pins.h"

#define OUT_UTXD0       0x10    // P3.4
#define IN_URXD0        0x20    // P3.5

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


T_FRAME tx;

unsigned char _serial_ctrl=0;
unsigned char _addr=0;
unsigned char _flgPending=0;

void serial_send(unsigned char *datas,unsigned char nb)
{
    if (_serial_ctrl&SER_CT_TX)
        return;

    if (nb==0)
        return;

    P3OUT|=PIN_TX_EN;
    _flgPending=1;
    _delay_cycles(10);

    int i;
    for (i=0;i<nb;i++)
    {
        tx.data[i]=datas[i];
    }
    tx.pos=1;
    tx.size=nb;
    _serial_ctrl|=SER_CT_TX;
    U0TXBUF=datas[0];
    IE2 |= UTXIE0;
}

void serial_tick(void)
{
    if (_flgPending==2)
    {
        if (U0TCTL & TXEPT)
        {
            _delay_cycles(10);
            P3OUT&=~PIN_TX_EN;
            _flgPending=0;
        }
    }
}

#pragma vector=USART0TX_VECTOR
__interrupt void USART0_TX_ISR(void)
{
    if (_serial_ctrl & SER_CT_TX)
    {
        if (tx.pos<tx.size)
        {
            U0TXBUF = tx.data[tx.pos];
            tx.pos++;
        }
        else
        {
            tx.size=0;
            tx.pos=0;
            _serial_ctrl &= ~SER_CT_TX;
            IE2 &= ~UTXIE0;
            _flgPending=2;
        }
    }
}

#pragma vector=USART0RX_VECTOR
__interrupt void USART0_RX_ISR(void)
{
    unsigned char dta=U0RXBUF;
}

void serial_init(int speed)
{
    UCTL0 = SWRST;
    UCTL0 |= CHAR ;                  ///< 8-bit character, no parity, 2 bit stop
    UTCTL0 = SSEL1|SSEL0;            ///< SMCLK
    URCTL0 = 0;            ///<
    UBR00 = 0x41;                    ///< Baud rate 9600
    UBR10 = 0x03;                    ///< Baud rate 9600
    UMCTL0 = 0x00;                   ///< no modulation
    ME2 = UTXE0|URXE0;               ///< Enabled USART0 TXD/RXD
    P3DIR |= 0x10;                   ///< P3.4 output direction
    P3DIR &= ~0x20;                  ///< P3.5 input direction
    P3DIR|=PIN_TX_EN;                ///< P3.6 en sortie
    P3SEL |= IN_URXD0 | OUT_UTXD0;   ///< P3.4 and P3.5 = USART0 TXD/RXD

    int i;
    for (i=0;i<MAX_BUFFER_SIZE;i++)
    {
        tx.data[i]=0;
    }
    tx.size=0;
    tx.pos=0;

    _serial_ctrl=0;
    _addr=0;
    _flgPending=0;

    UCTL0 &= ~SWRST;
    IFG2 &= ~(URXIFG0 | UTXIFG0);

    IE2 |= URXIE0;
}

