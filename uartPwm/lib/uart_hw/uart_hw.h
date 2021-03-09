#ifndef _UART_HW_H_
#define _UART_HW_H_

#include <msp430g2253.h>
#include <stdio.h>



static const char *COLORS[] = { "\033[1;31m", "\033[1;32m", "\033[1;33m", "\033[1;34m", "\033[1;35m" };
#define TRACE      do { printf("%s %s:%d: \033[1;37m %s \033[0m\n\r", COLORS[__COUNTER__%5],__FILE__, __LINE__ , __PRETTY_FUNCTION__ );} while(0)
#define WATCH(t,x) do { printf("%s %s:%d: \033[1;37m" #x "=" t "\033[0m\n\r", COLORS[__COUNTER__%5], __FILE__, __LINE__ , (x));} while(0)


#define UART_TXD        BIT1                                // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD        BIT2                                // RXD on P1.2 (Timer0_A.CCI1A)
#define PRINT           uartPrint

#define UARTBUFSIZE     (64)
#define NLRF            "\n\r"
#define PROMPT          NLRF "$ "

extern volatile unsigned int uartRxBufferIndex;

void uartSetupPins(void);
void uartInit(volatile char * pBuffer, volatile unsigned char sizeOfBuffer);
void uartTx(const char byte);
int putchar(int c);
void uartPrint(const char *string);
void uartDisable(void);

#define UART_HW_BAUD 9600

#endif /*_UART_HW_H_*/
