/*
 * Alan Barr
 * 2013
 *
 * Based on example code by:
 * D. Dang
 * Texas Instruments Inc.
 * February 2011
 */

#include "uart_hw.h"
#include "time430.h"
#include "legacymsp430.h"
#include "msp430.h"
volatile unsigned int uartRxBufferIndex = 0;                    // External, zero elsewhere

volatile static unsigned char gUartBufSize = 0;                 // Size of receive buffer
volatile static volatile char * gUartRxBuffer = 0;              // Pointer to start of receive buf

volatile static const char * uartTxString = 0;


void uartInit(volatile char * pBuffer, volatile unsigned char sizeOfBuffer)
{
    gUartRxBuffer = pBuffer;
    gUartBufSize = sizeOfBuffer;

    UCA0CTL1 |= UCSWRST;

    P1SEL |= BIT1 | BIT2;
    P1SEL2 |= BIT1 | BIT2;

    UCA0CTL1 |= UCSSEL_2;                     // SMCLK

    UCA0BR0 = 0xFF & (unsigned short)(TIME430_CLOCK_HZ / UART_HW_BAUD);
    UCA0BR1 = 0xFF & ((unsigned short)(TIME430_CLOCK_HZ / UART_HW_BAUD) >> 8);

    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1

    UCA0CTL1 &= ~UCSWRST;

    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
    __enable_interrupt();
}


int putchar(int c)
{
    while ( ( UC0IFG & UCA0TXIFG ) == 0 );
    UCA0TXBUF = (char)c;
    return 0;
}

void uartPrint(const char *string)
{
    while (IE2 & UCA0TXIE)
    {
        /*TODO No fancy queues here. Blocks if busy.*/
    }
    uartTxString = string;
    IE2 |= UCA0TXIE; 
}

void uartDisable(void)
{
    UCA0CTL1 |= UCSWRST;
    P1SEL &= ~(BIT1 | BIT2);
    P1SEL2 &= ~(BIT1 | BIT2);
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI_TX_ISR(void)
//interrupt(USCIAB0TX_VECTOR) USCI_TX_ISR(void)
{
    if (uartTxString != 0 && *uartTxString != 0)
    {
        UCA0TXBUF = *uartTxString;
        uartTxString++;
    }
    else
    {
        IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
    }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI_RX_ISR(void)
//interrupt(USCIAB0RX_VECTOR) USCI_RX_ISR(void)
{
    if (uartRxBufferIndex < gUartBufSize)
    {
        gUartRxBuffer[uartRxBufferIndex] = UCA0RXBUF;
        //UCA0TXBUF = gUartRxBuffer[uartRxBufferIndex]; //XXX Debug echo. Not smart.
        uartRxBufferIndex++;
    }
}

