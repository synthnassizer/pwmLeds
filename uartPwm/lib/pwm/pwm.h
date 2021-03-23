#ifndef PWM_H
#define PWM_H

#define ACLK            (TASSEL_1)  //AUX CLK
#define SMCLK           (TASSEL_2)  //Sub Main CLK
#define SELECTED_INTENSITY_CLK (ACLK)
#if (SELECTED_INTENSITY_CLK==ACLK)
#define CLK_FREQ_HZ  32767 //Hz
#else
#error "code has only been designed for use with ACLK, at the moment"
#endif

#define SELECTED_INTENSITY_DIVIDER        (ID_3) //divider 8
#define ACLK_POST_DIV (CLK_FREQ_HZ / (1 << ((TA1CTL & SELECTED_INTENSITY_DIVIDER) >> 6))) //clk freq after divider, units Hz
#define MAX_FADE_PERIOD (0xFFFF * 1000 / ACLK_POST_DIV) //results to 16003msec, for ACLK and ID_3 divider
#define STEPS_PER_MSEC (0xFFFF / MAX_FADE_PERIOD) //results to 4steps/msec (4.096 but truncated)




void initPwm();

void setSteps(unsigned short const red,
              unsigned short const green,
              unsigned short const blue);
void setRed(unsigned short const red);
unsigned short getRed();
void setGreen(unsigned short const green);
unsigned short getGreen();
void setBlue(unsigned short const blue);
unsigned short getBlue();

void resetTick();
unsigned short getTick();


#endif /* PWM_H */
