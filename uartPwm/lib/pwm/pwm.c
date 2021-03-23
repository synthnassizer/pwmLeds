#include <msp430g2253.h>
#include "uart_hw.h"
#include "pwm.h"


//LEDs
#ifndef WITH_DBG_PINS
#define RED     (BIT0)
#define GREEN   (BIT2)
#define BLUE    (BIT4)
#else
#define RED     (BIT0 | BIT1)
#define GREEN   (BIT2 | BIT3)
#define BLUE    (BIT4 | BIT5)
#endif

#define OUTPORT P2OUT

#define REG_PWM_RED   TA0CCR0
#define REG_PWM_GREEN TA0CCR1
#define REG_PWM_BLUE  TA0CCR2


static unsigned short tick = 0u;



void initColorsTimer();
void initIntensityTimer();


void initPwm()
{
    initColorsTimer();
    initIntensityTimer();
}

void initColorsTimer()
{
    P2DIR |= RED | GREEN | BLUE;       // P1.0 output
    TA0CCTL0 |= CCIE;                  // CCR0 interrupt enabled
    TA0CCTL1 |= CCIE;                  // CCR1 interrupt enabled
    TA0CCTL2 |= CCIE;                  // CCR2 interrupt enabled
    REG_PWM_RED = 0x0;
    REG_PWM_GREEN = 0x0;
    REG_PWM_BLUE = 0x0;
    TA0CTL = TASSEL_2 | MC_2 | TAIE;   // SMCLK, contmode , Timer0A1 interrupt vector enabled
    OUTPORT = 0x0;
}

void initIntensityTimer()
{
    TA1CCR0 |= STEPS_PER_MSEC;         // Counter value
    TA1CCTL0 |= CCIE;                  // Enable Timer1_A interrupts
    TA1CTL |= SELECTED_INTENSITY_CLK | MC_1 | SELECTED_INTENSITY_DIVIDER;  // ACLK, Up to TA1CCR0 Mode, divider 8
}

void setSteps(unsigned short const red,
              unsigned short const green,
              unsigned short const blue)
{
    setRed(red);
    setGreen(green);
    setBlue(blue);
}
void setRed(unsigned short const red)
{
    REG_PWM_RED = red;
}
unsigned short getRed()
{
    return REG_PWM_RED;
}

void setGreen(unsigned short const green)
{
    REG_PWM_GREEN = green;
}
unsigned short getGreen()
{
    return REG_PWM_GREEN;
}

void setBlue(unsigned short const blue)
{
    REG_PWM_BLUE = blue;
}
unsigned short getBlue()
{
    return REG_PWM_BLUE;
}

void resetTick()
{
    tick = 0x0;
}
unsigned short getTick()
{
    return tick;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
    OUTPORT &= ~RED;                    //process Timer0 : TA0CCR0 / REG_PWM_RED
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1 (void)
{
    switch (TA0IV)
    {
    case TA0IV_TACCR1:                  //process Timer0 : TACCR1 / REG_PWM_GREEN
        OUTPORT &= ~GREEN;
    break;
    case TA0IV_TACCR2:                  //process Timer0 : TACCR2 / REG_PWM_BLUE
        OUTPORT &= ~BLUE;
        break;
    case TA0IV_TAIFG:                   //process when counter resets to 0 
        OUTPORT |= RED | GREEN | BLUE;  //reset all to high at the beginning
        break;
    }
}

#pragma vector=TIMER1_A0_VECTOR     // Timer1 A0 interrupt service routine
__interrupt void Timer1_A0 (void)
{
    //process Timer1 : TA1CCR0
    tick++; 

}
