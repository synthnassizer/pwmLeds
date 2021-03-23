#include "animations.h"
#include <msp430g2253.h>
#include "uart_hw.h"
#include "pwm.h"

#define FADE_PERIOD_BOUND_OFFSET (2) //just an offset from the fade period boundaries [0, stepPeriod]

typedef enum Mode
{
    manual = 0,
    strobo = 1,     //cold white light and jumping between 0->MAX intensity
    colorFlow = 2,  //constant intensity, interpolate through colours
    fade = 4        //fade in to one colour, then fade out, then change colour, repeat
} eMode;

typedef struct sMAXRGB //pwm steps of each RGB value
{
    unsigned int r;   //steps for red led (0..65535)
    unsigned int g;   //steps for green led (0..65535)
    unsigned int b;   //steps for blue led (0..65535)
} tMAXRGB;

static tRGB pwm = { 0 };
static tRGB fadeSlope = { 0 };
static unsigned short maxIntensity = 0xFFFF; //works as max rgb
//static tRGB maxPwm = { 0 }; //max RGB per color. depends on maxIntensity
static unsigned short stepPeriod = 0x0;
static eMode opmode = manual;
static unsigned short prevTick = 0;


short unsigned int msecToPwmSteps(unsigned int msec);
void calcSlopes(unsigned short const maxInt, unsigned short const stepPer, tRGB const * const pwms);
void animStrobo();
void animFade(unsigned short const delta);

void setMaxIntensity(unsigned short const value)
{
    maxIntensity = value;
    calcSlopes(maxIntensity, stepPeriod, &pwm);
}
unsigned short getMaxIntensity()
{
    return maxIntensity;
}

void setSpeed(unsigned short const msec)
{
    stepPeriod = msecToPwmSteps(msec);
}
unsigned short getSpeed()
{
    return (stepPeriod * 1000 / ACLK_POST_DIV);
}

void setColors(unsigned short const red,
               unsigned short const green,
               unsigned short const blue)
{
    pwm.r = red;
    pwm.g = green;
    pwm.b = blue;

    if (manual == opmode)
    {
        setSteps(red, green, blue);
    }
    else //for all other modes
    {
        calcSlopes(maxIntensity, stepPeriod, &pwm);
    }
}
tRGB const * getColorsPtr()
{
    return &pwm;
}

unsigned short msecToPwmSteps(unsigned int msec)
{
    unsigned int stepsForMSec = 0u;

    if (MAX_FADE_PERIOD < msec) 
    {
        return 0xFFFF;
    }
    else
    {
        stepsForMSec = msec * ACLK_POST_DIV;
        return (unsigned short)(stepsForMSec / 1000);
    }
}

void calcSlopes(unsigned short const maxInt, unsigned short const stepPer, tRGB const * const pwms)
{
    tMAXRGB maxPwm = { 0 }; 
    maxPwm.r = maxInt * pwms->r;
    maxPwm.g = maxInt * pwms->g;
    maxPwm.b = maxInt * pwms->b;
    fadeSlope.r = (unsigned short)(maxPwm.r / stepPer);
    fadeSlope.g = (unsigned short)(maxPwm.g / stepPer);
    fadeSlope.b = (unsigned short)(maxPwm.b / stepPer);
}

void setMode(unsigned short const mode,
             unsigned short const speed,
             unsigned short const maxInt) //TODO pass values
{
    opmode = mode;
    maxIntensity = maxInt;

    switch (opmode)
    {
        case strobo:
            stepPeriod = msecToPwmSteps(speed);
            break;
        case fade:
            stepPeriod = msecToPwmSteps(speed);
            calcSlopes(maxIntensity, stepPeriod, &pwm);
            break;
        case colorFlow : break;
        case manual : //set colour
            setSteps(pwm.r, pwm.g, pwm.b);
        default: break;
    }
}
unsigned short getMode()
{
    return (unsigned short)opmode;
}

void animFade(unsigned short const delta)
{
    unsigned short red   = getRed();
    unsigned short green = getGreen();
    unsigned short blue  = getBlue();

    red   += fadeSlope.r * delta;
    green += fadeSlope.g * delta;
    blue  += fadeSlope.b * delta;

    setSteps(red, green, blue);
}

void animStrobo()
{
    if (0x0 != getRed())
    {
        setSteps(pwm.r, pwm.g, pwm.b);
    }
    else
    {
        setSteps(0,0,0); //black - all LEDs off
    }
}

void performAnimation()
{
    int delta = 0;
    unsigned short currTick = getTick();

    if (prevTick != currTick)
    {
        delta = currTick - prevTick;
        if (delta < 0)
        {
            /* wrap around so the current value + 1, holds the correct delta   */
            delta = 1 + currTick;
        }

        switch (opmode)
        {
            case strobo: animStrobo(); break;
            case fade:
                animFade((unsigned short)delta);

                if ((currTick > (stepPeriod - FADE_PERIOD_BOUND_OFFSET)) || (currTick < FADE_PERIOD_BOUND_OFFSET))
                {
                    fadeSlope.r = -fadeSlope.r;
                    fadeSlope.g = -fadeSlope.g;
                    fadeSlope.b = -fadeSlope.b;
                    resetTick();
                    currTick = getTick();
                }
                break;
            case colorFlow : break;
            case manual:
            default: break;
        }

        prevTick = currTick;
    }
}
