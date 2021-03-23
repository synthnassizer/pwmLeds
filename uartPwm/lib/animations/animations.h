#ifndef ANIMATIONS_H
#define ANIMATIONS_H


typedef struct sRGB //pwm steps of each RGB value
{
    unsigned short r;   //steps for red led (0..65535)
    unsigned short g;   //steps for green led (0..65535)
    unsigned short b;   //steps for blue led (0..65535)
} tRGB;

void setMaxIntensity(unsigned short const value);
unsigned short getMaxIntensity();

void setSpeed(unsigned short const msec);
unsigned short getSpeed(); //in msec

void setMode(unsigned short const mode,
             unsigned short const speed,
             unsigned short const maxInt);
unsigned short getMode();


void setColors(unsigned short const red,
               unsigned short const green,
               unsigned short const blue);
tRGB const * getColorsPtr();

void performAnimation();

#endif //ANIMATIONS_H
