/**
 * Copyright (C) PlatformIO <contact@platformio.org>
 * See LICENSE for details.
 */

/*#include <msp430g2553.h>

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;

    // make the LED pin an output for P1.0
    P1DIR |= 0x41;

    volatile int i;

    P1OUT = 0x40;

    while (1)
    {
        for (i = 0; i < 10000; i++);

        // toggle the LEDs
        P1OUT ^= 0x41;
    }

    return 0;
}*/

#define TIME430_CLOCK_FREQ 16 //MHz

#include "cli.h"

static void commandParser(char * stringToParse, tCommandStruct * commandStructPtr);

int main(void)
{
    volatile char uartRxBuffer[UARTBUFSIZE];

    WDTCTL = WDTPW + WDTHOLD;               // Stop WDT

    TIME430_CALIBRATE_CLOCK();

    P1DIR = LED1 + LED2;                    // LED pins outputs and high
    P1OUT = LED1 + LED2;

    uartInit(uartRxBuffer, UARTBUFSIZE);

    memClear((char *)uartRxBuffer, UARTBUFSIZE);
    PRINT("Cli Started.\n\r");
    cliHelp();
    PRINT(PROMPT);

    /* Loop constantly checks UART buffer for a newline or carriage return.
     * If it finds one it calls the parser to check if it was a valid command.
     * The uartBuffer is handled here for overflow. 
     */
    while(1)
    {
        if(uartRxBufferIndex >= UARTBUFSIZE)
        {
            uartRxBufferIndex = 0;
            memClear((char*)uartRxBuffer, sizeof(uartRxBuffer));
            PRINT(PROMPT);
        }

        /* If last char received was a newline, parse */
        else if(uartRxBuffer[uartRxBufferIndex -1] == '\n' || 
                uartRxBuffer[uartRxBufferIndex -1] == '\r')
        {
            uartRxBuffer[uartRxBufferIndex - 1] = '\0';
            commandParser((char *)uartRxBuffer, (tCommandStruct *)commandList);
            uartRxBufferIndex = 0;
            memClear((char *)uartRxBuffer, UARTBUFSIZE);
            PRINT(PROMPT);
        }
    }

    return 0;
}

/* Function runs through all commands in argument of tCommandStruct comparing with
 * stringToParse. If a command matches, the corresponding function pointer is called
 */
static void commandParser(char * stringToParse, tCommandStruct * commandStructPtr)
{
    int ctr;

    for(ctr = 0; ctr < COMMANDLISTSIZE; ctr++)
    {
        if (stringCompare(commandStructPtr[ctr].commandString, stringToParse) >= 0)
        {
            commandStructPtr[ctr].function();
        }
    }
}
