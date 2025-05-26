#include <msp430.h>

#include "serial.h"
#include "frames.h"
#include "prot.h"

#include "pins.h"
#include "tick.h"

#define OUT_UTXD0       0x10    // P3.4
#define IN_URXD0        0x20    // P3.5

T_FRAME rx;
T_FRAME tx;

#define TX_IDLE     (0)
#define TX_TOSEND   (1)
#define TX_SENDING  (2)
#define TX_LAST     (3)

volatile unsigned char _serial_tx_state=0;

void serial_send(unsigned char *datas,unsigned char nb)
{
    if (_serial_tx_state!=TX_IDLE)
        return;

    if (nb==0)
        return;

    int i;
    for (i=0;i<nb;i++)
    {
        tx.data[i]=datas[i];
    }
    tx.pos=1;
    tx.size=nb;

    _serial_tx_state=TX_TOSEND;
}

void serial_tick(void)
{
    switch (_serial_tx_state)
    {
        case TX_TOSEND:
        {
            tick_delay_ms(5);
            P3OUT|=PIN_TX_EN;
            tick_delay_ms(5);
            _serial_tx_state=TX_SENDING;
            U0TXBUF=tx.data[0];
            IE2 |= UTXIE0;
            break;
        }
        case TX_SENDING:
        {
            break;
        }
        case TX_LAST:
        {
            while (!(U0TCTL & TXEPT));
            tick_delay_ms(2);
            P3OUT&=~PIN_TX_EN;
            _serial_tx_state=TX_IDLE;
            break;
        }

    }
}

#pragma vector=USART0TX_VECTOR
__interrupt void USART0_TX_ISR(void)
{
    switch (_serial_tx_state)
    {
        case TX_SENDING:
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
                _serial_tx_state=TX_LAST;
            }
            break;
        }
        default:
        {
            tx.size=0;
            tx.pos=0;
            IE2 &= ~UTXIE0;
            break;
        }
    }
}

#pragma vector=USART0RX_VECTOR
__interrupt void USART0_RX_ISR(void)
{
    unsigned char dta=U0RXBUF;
    frames_on_receive_byte(&rx,dta);
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

    frames_init(&tx);
    frames_init(&rx);
    _serial_tx_state=TX_IDLE;

    UCTL0 &= ~SWRST;
    IFG2 &= ~(URXIFG0 | UTXIFG0);

    IE2 |= URXIE0;
}

void serial_send_oya(unsigned char addr,unsigned char status,unsigned short tick_ms,unsigned short volt,unsigned short time_s,unsigned short errs)
{
    frames_build_oya(&tx,addr,status,tick_ms,volt,time_s,errs);
    serial_send(tx.data,tx.size);
}

void serial_send_pump(unsigned char addr,unsigned char status,unsigned short tick_ms,unsigned short volt,unsigned short time_s,unsigned short errs,unsigned short flow)
{
    frames_build_pump(&tx,addr,status,tick_ms,volt,time_s,errs,flow);
    serial_send(tx.data,tx.size);
}

void serial_send_pong(unsigned char val)
{
    frames_build_pong(&tx,val);
    serial_send(tx.data,tx.size);
}
