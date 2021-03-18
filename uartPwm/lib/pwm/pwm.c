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

#define REG_PWM_RED   TA0CCR0
#define REG_PWM_GREEN TA0CCR1
#define REG_PWM_BLUE  TA0CCR2


#define ACLK            (TASSEL_1)  //AUX CLK
#define SMCLK           (TASSEL_2)  //Sub Main CLK
#define SELECTED_INTENSITY_CLK (ACLK)
#if (SELECTED_INTENSITY_CLK==ACLK)
#define CLK_FREQ_HZ  32767 //Hz
#else
#error "code has only been designed for use with ACLK, at the moment"
#endif

#define SELECTED_INTENSITY_DIVIDER        (ID_3) //divider 8
#define ACLK_POST_DIV (CLK_FREQ_HZ / (1 << ((TA1CTL & SELECTED_INTENSITY_DIVIDER) >> 6))) //clk freq after divider
#define MAX_FADE_PERIOD (0xFFFF * 1000 / ACLK_POST_DIV) //results to 16003msec, for ACLK and ID_3 divider
#define STEPS_PER_MSEC (0xFFFF / MAX_FADE_PERIOD) //results to 4steps/msec (4.096 but truncated)

typedef enum Mode
{
    manual = 0,
    strobo = 1,     //cold white light and jumping between 0->MAX intensity
    colorFlow = 2,  //constant intensity, interpolate through colours
    fade = 4        //fade in to one colour, then fade out, then change colour, repeat
} eMode;

typedef struct sPWMRGB //pwm steps of each RGB value
{
    unsigned short r;   //steps for red led (0..65535)
    unsigned short g;   //steps for green led (0..65535)
    unsigned short b;   //steps for blue led (0..65535)
} tPWMRGB;

static tPWMRGB pwm = { 0 };
static unsigned short maxIntensity = 0xFFFF; //works as max rgb
static int step = 0x0; //works when needing to fade intensity over time
static eMode opmode = manual;




void initColorsTimer();
void initIntensityTimer();
short unsigned int msecToPwmSteps(unsigned int msec);
unsigned short adjustColorByInt(unsigned short const color, unsigned short const intensity);




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
    REG_PWM_RED = pwm.r;
    REG_PWM_GREEN = pwm.g;
    REG_PWM_BLUE = pwm.b;
    TA0CTL = TASSEL_2 | MC_2 | TAIE;   // SMCLK, contmode , Timer0A1 interrupt vector enabled
    OUTPORT = 0x0;
}

void initIntensityTimer()
{
    TA1CCR0 |= STEPS_PER_MSEC;         // Counter value
    TA1CCTL0 |= CCIE;                  // Enable Timer1_A interrupts
    TA1CTL |= SELECTED_INTENSITY_CLK | MC_1 | SELECTED_INTENSITY_DIVIDER;  // ACLK, Up to TA1CCR0 Mode, divider 8
}


void setMaxIntensity(unsigned short const value)
{
    maxIntensity = value;
    WATCH("0x%x", maxIntensity);
}
unsigned short getMaxIntensity()
{
    return maxIntensity;
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
    pwm.r = red;
    REG_PWM_RED = pwm.r;
    WATCH("0x%x", pwm.r);
}
unsigned short getRed()
{
    return pwm.r;
}

void setGreen(unsigned short const green)
{
    pwm.g = green;
    REG_PWM_GREEN = pwm.g;
    WATCH("0x%x",pwm.g);
}
unsigned short getGreen()
{
    return pwm.g;
}

void setBlue(unsigned short const blue)
{
    pwm.b = blue;
    REG_PWM_BLUE = pwm.b;
    WATCH("0x%x",pwm.b);
}
unsigned short getBlue()
{
    return pwm.b;
}


unsigned short msecToPwmSteps(unsigned int msec)
{
    unsigned int fadePeriod = TA1CCR0 / ACLK_POST_DIV;
    unsigned int stepsForMSec = 0u;

    //some very high value just below 16k msec,
    //which is the theoretical upper time to ladder 0->65k with a AUX_CLK (32767 ? Hz) clock
    //usually the clock will be somewhat ofset from this exact value though
    if (MAX_FADE_PERIOD < fadePeriod) 
    {
        return 0xFFFF;
    }
    else
    {
        stepsForMSec = msec * ACLK_POST_DIV;
        return (unsigned short)(stepsForMSec / 1000);
    }
}

unsigned short adjustColorByInt(unsigned short const color, unsigned short const intensity)
{
    unsigned int bigNum = 0u;
    bigNum = color * intensity;             // can get > MAX_INT32.
    return (unsigned short)(bigNum >> 16);    // i.e. div by 0xFFFF;
}

//#define MIN_PWM
void setMode(unsigned short const mode, unsigned short speed) //TODO pass values
{
    eMode const opmode = mode;

    switch (opmode)
    {
        case fade:
            intensity = 0x0;
        case strobo:
            TA1CCR0 = msecToPwmSteps(speed); //TODO
            break;
        case manual : //set colour
        default: break;
    }
}
unsigned short getMode()
{
    return (unsigned short)opmode;
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
    switch (opmode)
    {
        case strobo:
            if (0x0 != REG_PWM_RED)
            {
                //set to white
                REG_PWM_RED = pwm.r;
                REG_PWM_GREEN = pwm.g;
                REG_PWM_BLUE = pwm.b;
            }
            else
            {
                REG_PWM_RED = 0x0;
                REG_PWM_GREEN = 0x0;
                REG_PWM_BLUE = 0x0;
            }
            break;
        case fade:
            REG_PWM_RED += step;
            green = REG_PWM_GREEN;
            blue  = REG_PWM_BLUE;

            if (((((maxIntensity * pwm.r) >> 16) - 2) < REG_PWM_RED) || (1 > REG_PWM_RED))
                
            TA1CCR0 += IncDec_PWM;			// Increase or decrease duty cycle

            if( TA0CCR0 > 998 || TA0CCR0 < 2 )	// Reverse direction if it falls within values
            IncDec_PWM = -IncDec_PWM;
            break;
        case manual:
        default: break;
    }

}*/
