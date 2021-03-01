/*
* Copyright (C)
* Originally by: D. Dang, Texas Instruments Inc. December 2010
* Modified by: Alan Barr 2011
*/

/* Module provides basic bit-banged UART functionality.
 *
 * To use call uartInit() passing in a pointer to the memory to use for storing
 * rx'ed bytes as well as the size of the buffer. Call uartSetupPins() to then
 * configure the pins as required.
 *
 * After this tx'ing and rx'ing should be possible.
 *
 * The memory used to store received characters should be cleared as required,
 * which is done externally to this module. Also uartRxBufferIndex should be set
 * to 0 as appropriate, which is the index the next received character will be 
 * placed.
 *
 * If the receive buffer fills, the index will simply sit at the last position
 * in the buffer, constantly overwriting it. This could be problematic if
 * waiting on two characters, such as a newline and carriage return.
*/

#include "uart.h"

static int uartTxData;                                          // UART internal variable for TX
volatile static unsigned char gUartBufSize = 0;                 // Size of receive buffer
volatile static volatile char * gUartRxBuffer = 0;              // Pointer to start of receive buf
volatile unsigned int uartRxBufferIndex = 0;                    // External, zero elsewhere


/* Configure pins for duplex UART */
void uartSetupPins(void)
{   
    P1OUT &= ~(UART_TXD + UART_RXD);                            
    P1SEL |= UART_TXD + UART_RXD;                               // Timer function for TXD/RXD pins
    P1DIR |= UART_TXD;
    P1DIR &= ~UART_RXD;                                         // Set all pins but RXD to output
}


/* Configure Timer A for duplex UART and registers the memory to store
 * recieved characters. 
 * pBuffer - a pointer to the array where the received characters will go. 
 * sizeOfBuffer - the size of the created array. */
void uartInit(volatile char * pBuffer, volatile unsigned char sizeOfBuffer)
{
    __enable_interrupt();
    TACCTL0 = OUT;                                              // Set TXD Idle as Mark = '1'
    TACCTL1 = SCS + CM1 + CAP + CCIE;                           // Sync, Neg Edge, Capture, Int
    TACTL = TASSEL_2 + MC_2;                                    // SMCLK, start in continuous mode
    gUartRxBuffer = pBuffer;
    gUartBufSize = sizeOfBuffer;
}


/* Disable Timer A for duplex UART.*/
void uartDisable(void)
{
    TACCTL0 = 0x00;
    TACCTL1 = 0x00;
    TACTL = 0x00;
}


/* Print one byte over UART 
 * byte - character to transmit.*/
void uartTx(const char byte)
{
    while (TACCTL0 & CCIE);                                     // Ensure last char got TX'd
    TACCR0 = TAR;                                               // Current state of TA counter
    TACCR0 += UART_TBIT;                                        // One bit time till first bit
    TACCTL0 = OUTMOD0 + CCIE;                                   // Set TXD on EQU0, Int
    uartTxData = byte;                                          // Load global variable
    uartTxData |= 0x100;                                        // Add mark stop bit to TXData
    uartTxData <<= 1;                                           // Add space start bit
    _BIS_SR(LPM0_bits);                                         // Enter LMP0 for duration
}


/* Print a NULL terminated string over UART.
 * string - null terminated sting to transmit over UART. */
void uartPrint(const char * string)
{
    while (*string) 
    {
        uartTx(*string++);
    }
}


/* Receive interrupt handler */
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1_ISR(void)
//interrupt(TIMER0_A1_VECTOR) Timer0_A1_ISR(void)
{
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;                            // Seems to need to be unsigned 

    if (TA0IV & TA0IV_TACCR1)                                   
    {
        TACCR1 += UART_TBIT;                                    // Add Offset to CCRx
        if (TACCTL1 & CAP)                                      // Capture mode = start bit edge
        { 
            TACCTL1 &= ~CAP;                                    // Switch capture to compare mode
            TACCR1 += UART_TBIT_DIV_2;                          // Point CCRx to middle of D0
        }

        else 
        {
            rxData >>= 1;

            if (TACCTL1 & SCCI)                                 // Get bit waiting in receive latch
            {  
                rxData |= 0x80;
            }

            rxBitCnt--;

            if (rxBitCnt == 0)                                  // All bits RXed?
            {
                gUartRxBuffer[uartRxBufferIndex] = rxData;

                /* Ensure buffer is only incremented when not at the end */
                if (uartRxBufferIndex < (gUartBufSize - 1))
                {
                    uartRxBufferIndex++;
                }

                rxBitCnt = 8;                                   // Re-load bit counter
                TACCTL1 |= CAP;                                 // Switch compare to capture mode
                __bic_SR_register_on_exit(LPM0_bits);           // Clear LPM0 bits from 0(SR)
            }
        }
    }
}


/* Transmit interrupt handler */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void)
//interrupt(TIMER0_A0_VECTOR) Timer0_A0_ISR(void)
{
    static char txBitCnt = 10;

    TACCR0 += UART_TBIT;                                        // Add Offset to CCRx
    if (txBitCnt == 0)                                          // All bits TXed?
    {    
        TACCTL0 &= ~CCIE;                                       // All bits TXed, disable interrupt
        txBitCnt = 10;                                          // Re-load bit counter
        _BIC_SR_IRQ(LPM0_bits);                                 // Clear LPM0 bits
    }

    else 
    {
        if (uartTxData & 0x01) 
        {
            TACCTL0 &= ~OUTMOD2;                                // TX Mark '1'
        }
        else 
        {
            TACCTL0 |= OUTMOD2;                                 // TX Space '0'
        }
        uartTxData >>= 1;
        txBitCnt--;
    }
}      

