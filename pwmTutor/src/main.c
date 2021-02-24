/*
 * MSP430 Timer Tutorial Example Code 3
 * Anthony Scranney
 * www.Coder-Tronics.com
 * August 2014
 *
 * PWM example using both TimerA_0 and TimerA_1 to increase and decrease
 * the duty cycle, which in turn alters the brightness of the on board green LED
 */

#include <msp430g2253.h>


/*** Global Variable ***/
int IncDec_PWM = 1;


//variable PWM. 
//PWM controlled by timer0 (and compare regs 0,1). spits 2 pwm's in p1.0 (red, port1bit0) and p1.6
//variability controlled by timer1 and a software interrupt "Timer1_A0"
/*void initVariablePwm()
{
	// GPIO Set-Up 
    P1DIR |= BIT0 | BIT6;					// P1.0 & P1.6 set as output (Green LED)
    P1SEL |= BIT0 | BIT6;					// P1.0 & P1.6 selected Timer0_A Out1 output

	// Timer0_A Set-Up
    TA0CCR0 |= 1000;					// PWM period
    TA0CCR1 |= 1;					// TA0CCR1 PWM duty cycle
    TA0CCTL1 |= OUTMOD_7;			// TA0CCR1 output mode = reset/set
    TA0CTL |= TASSEL_2 + MC_1;		// SMCLK, Up Mode (Counts to TA0CCR0)

	// Timer1_A Set-Up
    TA1CCR0 |= 2000;					// Counter value
    TA1CCTL0 |= CCIE;				// Enable Timer1_A interrupts
    TA1CTL |= TASSEL_2 + MC_1          ;		// SMCLK, Up Mode (Counts to TA1CCR0)
}

#pragma vector=TIMER1_A0_VECTOR     // Timer1 A0 interrupt service routine
__interrupt void Timer1_A0 (void)
{

    TA0CCR1 += IncDec_PWM;			// Increase or decrease duty cycle

    if( TA0CCR1 > 998 || TA0CCR1 < 2 )	// Reverse direction if it falls within values
       IncDec_PWM = -IncDec_PWM;
}*/

//PWM by toggling output pin and proper use of the TAIV reg
/*void initTaivUse()
{
    P1DIR |= BIT0 + BIT6 + BIT7;
    P2DIR |= BIT0 + BIT6 + BIT7;
    //P2SEL |= BIT0;
    P1OUT = 0x0;
    P2OUT = 0x0;
    TA0CTL = TASSEL_2 + MC_2 + ID_1 + TAIE; //SMCLK , contmode, interrupt
    TA0CCTL0 = CCIE;
    //TA0CCTL1 = CCIE;
    //TA0CCTL2 = CCIE;
    TA0CCR0 = 3000;
    TA0CCR1 = 2000;
    TA0CCR2 = 1000;
    //TA0CCTL0 |= CCIE;
}

//Timer_A3 Interrupt vector (TAIV) handler
#pragma vector=TIMER0_A0_VECTOR
__interrupt void interTaivUse0(void)
{
    P2OUT ^= 0x01; 
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void interTaivUse1(void)
{
    //P1OUT ^= 0x01; 
    switch(TAIV)
    {
        case 2: break; //TACCR1 not used
        case 4: break; //TACCR2 not used
        case 10: P1OUT ^= BIT0; break; //TACCR0, overflow
    }
}*/


//slac485k.zip : ta01.c
/*void initSlacTa01()
{
  P1DIR |= BIT0 | BIT6;                            // P1.0 output
  P2DIR |= BIT0 | BIT1 | BIT2;                            // P2.0 output
  P1OUT = BIT6;
  TA0CCTL0 = CCIE;                             // CCR0 interrupt enabled
  TA0CCTL1 = CCIE;                             // CCR0 interrupt enabled
  TA0CCTL2 = CCIE;                             // CCR0 interrupt enabled
  TA0CCR0 = 0;
  TA0CTL = TASSEL_2 + MC_2 + TAIE;                  // SMCLK, contmode
}

// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
  P1OUT ^= BIT0;                            // Toggle P1.0
  P1OUT ^= BIT6;                            // Toggle P1.1
  TA0CCR0 += 50000;                            // Add Offset to CCR0
}
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1 (void)
{
  switch (TAIV)
  {
    case TA0IV_TACCR1:         //TACCR1
        P2OUT ^= BIT1;
        TA0CCR1 += 25000; 
    break;
    case TA0IV_TACCR2:         //TACCR2
        P2OUT ^= BIT2;
        TA0CCR2 += 12500;
        break;
    case TA0IV_TAIFG: P2OUT ^= BIT0;break;
  }
}*/

//variable PWM period 2-4msec. Duty Cycle 50% , so the pwm is high 1-2msec.
//this is driven by the high state of PWM using CCR1 (varies 1001 steps->1msec to 1999steps->2msec approx)
//then CCR0 which is the PWM period is simply double CCR1 (mult by2).
//the interrupt vector TIMER0_A1_VECTOR is enabled by TAIE , and the CCR1 interrupt is enabled by TA0CCTL1 |= CCIE;
//the whole example uses a single Timer (timer0), so Timer1 is free, but it is possible to use 2 timers.
//In fact the 1st attempt was based on initVariablePwm() (see above), where 2 timers are used.
void initVarPwmPeriod()
{
	// GPIO Set-Up 
    P1DIR |= BIT0 | BIT6;					// P1.0 & P1.6 set as output (Green LED)
    P1SEL |= BIT0 | BIT6;					// P1.0 & P1.6 selected Timer0_A Out1 output

	// Timer0_A Set-Up
    TA0CCR1 = 1000;					// TA0CCR1 PWM duty cycle "high" state
    TA0CCR0 = TA0CCR1 << 1;					// PWM period (is double  the high state)
    TA0CCTL1 |= OUTMOD_7 | CCIE;			// TA0CCR1 output mode = reset/set
    TA0CTL |= TASSEL_2 | MC_1 | TAIE;		// SMCLK, Up Mode (Counts to TA0CCR0)

	// Timer1_A Set-Up
    //TA1CCR0 |= 2000;					// Counter value
    //TA1CCTL0 |= CCIE;				// Enable Timer1_A interrupts
    //TA1CTL |= TASSEL_2 + MC_1          ;		// SMCLK, Up Mode (Counts to TA1CCR0)

    IncDec_PWM = 1;
}

#pragma vector=TIMER0_A1_VECTOR     // Timer1 A0 interrupt service routine
__interrupt void Timer0_A1 (void)
{
    switch(TA0IV)
    {
        case TA0IV_TACCR1: 
            TA0CCR1 += IncDec_PWM;			// Increase or decrease duty cycle
            TA0CCR0 = TA0CCR1 << 1; // * 2

            if( TA0CCR1 > 1999 || TA0CCR1 < 1001 )	// Reverse direction if it falls within values
            IncDec_PWM = -IncDec_PWM;
        break;
    }
}

int main(void)
{

	/*** Watchdog timer and clock Set-Up ***/
	WDTCTL = WDTPW + WDTHOLD;		// Stop watchdog timer
	DCOCTL = 0;             		// Select lowest DCOx and MODx
	BCSCTL1 = CALBC1_1MHZ;  		// Set range
	DCOCTL = CALDCO_1MHZ;   		// Set DCO step + modulation


    //initVariablePwm();
    //initTaivUse();
    //initSlacTa01();
    initVarPwmPeriod();

    _BIS_SR(LPM0_bits + GIE);		// Enter Low power mode 0 with interrupts enabled
}

