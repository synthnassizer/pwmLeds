/*
** Author: Alan Barr 
** Created: 2011
*/
/* Builds with mspgcc */
/* This file contains the commands which can be ran over UART */
//#include "cli.h"

#include <msp430g2553.h>
#include "commandFunctions.h"
#include "uart_hw.h"
#include "pwm.h"
#include "stringFunctions.h"
#include <string.h>
#include <stdlib.h>

#define COMMANDLISTSIZE 9

typedef void (*tFunctionNoArg)(void);
typedef void (*tFunctionArg)(char *);

typedef union uFunc
{
    tFunctionNoArg funcNoArg;
    tFunctionArg funcArg;
} tFunc;

typedef struct sCommandStruct
{
    char * commandString;
    tFunc function;
} tCommandStruct;


void setPwm(char * rgbStr);

const tCommandStruct commandList[COMMANDLISTSIZE]= 
{
    {"led one on", &(led1On)},
    {"led two on", &(led2On)},
    {"led one off", &(led1Off)},
    {"led two off", &(led2Off)},
    {"led one toggle", &(led1Toggle)},
    {"led two toggle", &(led2Toggle)},
    {"help", &(cliHelp)},
    {"pwm", .function.funcArg = &(setPwm)},
    {"reset", &(reset)}
};

void setPwm(char * rgbStr)
{
    unsigned short red;
    unsigned short green;
    unsigned short blue;

    char * rest = rgbStr;

    red   = (unsigned short) strtoul(rest, &rest, 0);
    green = (unsigned short) strtoul(rest, &rest, 0);
    blue  = (unsigned short) strtoul(rest, &rest, 0);

    setSteps(red,green,blue);
}

void led1On(void)
{
    P1OUT |= LED1;
}

void led2On(void)
{
    P1OUT |= LED2;
}

void led1Off(void)
{
    P1OUT &= ~LED1;
}

void led2Off(void)
{
    P1OUT &= ~LED2;
}

void led1Toggle(void)
{
    P1OUT ^= LED1;
}

void led2Toggle(void)
{
    P1OUT ^= LED2;
}

/* Resets mcu by writing to Watchdog Register without specifying password. */
void reset(void)
{
    WDTCTL = 0x00;
}

void cliHelp(void)
{
    int ctr;
    PRINT("Options:" NLRF);

    for(ctr=0;ctr<(sizeof(commandList)/sizeof(commandList[0]) - 1);ctr++)
    {
        PRINT(commandList[ctr].commandString);
        PRINT(NLRF);
    }
}


/* Function runs through all commands in argument of tCommandStruct comparing with
 * stringToParse. If a command matches, the corresponding function pointer is called
 */
void commandParser(char * stringToParse)
{
    int ctr = 0;
    int cmpRes = 0;
    char * rgbStr = NULL;

    for(ctr = 0; ctr < COMMANDLISTSIZE; ctr++)
    {
        cmpRes = stringCompare(commandList[ctr].commandString, stringToParse);
        switch (cmpRes)
        {
            case 0:
                commandList[ctr].function.funcNoArg();
                return;
            case 1:
                rgbStr = strchr(stringToParse, ' '); //skip cmd
                if (rgbStr)
                    commandList[ctr].function.funcArg(rgbStr);
                return;
            default: break;
        }
    }


}


