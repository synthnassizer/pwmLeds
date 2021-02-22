/**
 * Copyright (C) PlatformIO <contact@platformio.org>
 * See LICENSE for details.
 */

#include <msp430g2553.h>

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
}
