#ifndef _UART_HW_H_
#define _UART_HW_H_

#include <msp430g2253.h>

#define UART_TXD        BIT1                                // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD        BIT2                                // RXD on P1.2 (Timer0_A.CCI1A)
#define PRINT           uartPrint

extern volatile unsigned int uartRxBufferIndex;

void uartSetupPins(void);
void uartInit(volatile char * pBuffer, volatile unsigned char sizeOfBuffer);
void uartTx(const char byte);
void uartPrint(const char *string);
void uartDisable(void);

#define UART_HW_BAUD 9600

#endif /*_UART_HW_H_*/
