#include "pwm.h"
#include <msp430g2253.h>
#include "uart_hw.h"


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

static unsigned short stepsRed = 0;     //steps for red led (0..65535)
static unsigned short stepsGreen = 0;   //steps for green led (0..65535)
static unsigned short stepsBlue = 0;    //steps for blue led (0..65535)
typedef enum Mode
{
    manual,
    flash,
    colorFlow,
    colorCamel //TODO think of more
} eMode;
static eMode opmode = manual;


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
    TA0CCR0 = stepsRed;
    TA0CCR1 = stepsGreen;
    TA0CCR2 = stepsBlue;
    TA0CTL = TASSEL_2 + MC_2 + TAIE;   // SMCLK, contmode , Timer0A1 interrupt vector enabled
    OUTPORT = 0x0;
}

void initIntensityTimer()
{
    
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
    stepsRed = red;
    TA0CCR0 = stepsRed;
    WATCH("%x",stepsRed);
}
unsigned short getRed()
{
    return stepsRed;
}

void setGreen(unsigned short const green)
{
    stepsGreen = green;
    TA0CCR1 = stepsGreen;
    WATCH("%x",stepsGreen);
}
unsigned short getGreen()
{
    return stepsGreen;
}

void setBlue(unsigned short const blue)
{
    stepsBlue = blue;
    TA0CCR2 = stepsBlue;
    WATCH("%x",stepsBlue);
}
unsigned short getBlue()
{
    return stepsBlue;
}

void setMode(unsigned short const mode)
{
    (void)mode; //TODO
}
unsigned short getMode()
{
    return (unsigned short)opmode;
}



#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    OUTPORT &= ~RED;                    //process Timer0 TA0CCR0
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1 (void)
{
    switch (TA0IV)
    {
    case TA0IV_TACCR1:                  //process Timer0 TACCR1
        OUTPORT &= ~GREEN;
    break;
    case TA0IV_TACCR2:                  //process Timer0 TACCR2
        OUTPORT &= ~BLUE;
        break;
    case TA0IV_TAIFG:                   //process when counter resets to 0 
        OUTPORT |= RED | GREEN | BLUE;  //reset all to high at the beginning
        break;
    }
}
