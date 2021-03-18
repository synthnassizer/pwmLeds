#ifndef PWM_H_
#define PWM_H_

void initPwm();

void setMaxIntensity(unsigned short const value);
unsigned short getMaxIntensity();
void setSteps(unsigned short const red,
              unsigned short const green,
              unsigned short const blue);
void setRed(unsigned short const red);
unsigned short getRed();
void setGreen(unsigned short const green);
unsigned short getGreen();
void setBlue(unsigned short const blue);
unsigned short getBlue();

void setMode(unsigned short const mode, unsigned short speed);
unsigned short getMode();

#endif /* PWM_H_ */
