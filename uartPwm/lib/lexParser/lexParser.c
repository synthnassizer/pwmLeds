#include "lexParser.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "uart_hw.h"
#include "pwm.h"
#include <stdio.h>

#define DELIM " \n\r\t"


#define KEYWORDS_NUM 9
enum eKeywords { set , get , color , all , pwm , mode , red , green , blue };
const char * keywordStrs[KEYWORDS_NUM] = { "set", "get" , "color" , "all" , "pwm" , "mode" , "red" , "green" , "blue" };


typedef void (*setFunc)(unsigned short);
int setValue(char ** rest, setFunc setFn);

int findKeyword(char const * s);
int parseValue(char ** s, unsigned short * value);
int parseTripleValue(char ** s, unsigned short * value1, unsigned short * value2, unsigned short * value3);
int parseSetCmd(char ** rest);
int parseGetCmd(char ** rest);
int parseSetColor(char ** rest);
int parseSetMode(char ** rest);
int parseSetRed(char ** rest);
int parseSetGreen(char ** rest);
int parseSetBlue(char ** rest);



int findKeyword(char const * s)
{
    unsigned int i = 0;
    for(i = 0; i < (sizeof(keywordStrs)/sizeof(keywordStrs[0])); i++)
    {
        if (0 == strcmp(s, keywordStrs[i]))
        {
            return (int)i;
        }
    }

    //printf("keyword '%s' not identified." NLRF,s);
    return -1;
}

int parseToken(char * s, char ** rest)
{
    char const * next = strtok_r(s, DELIM, rest);

    if (next && (0 < strlen(next)))
    {
        //TODO does not account for when "color" word is ommited".
        return findKeyword(next);
    }

    return -2; //TODO End of stream missing
}

int parseValue(char ** rest, unsigned short * value)
{
    char * const current = *rest;
    *value = (unsigned short)strtoul(current, rest, 0);
    if (current != *rest )
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int parseTripleValue(char ** s, unsigned short * value1, unsigned short * value2, unsigned short * value3)
{
    if (*s && (0 < strlen(*s)))
    {
        *value1 = (unsigned short)strtoul(*s, s, 0);
        if (*s && (0 < strlen(*s)))
        {
            *value2 = (unsigned short)strtoul(*s, s, 0);
            if (*s && (0 < strlen(*s)))
            {
                *value3 = (unsigned short)strtoul(*s, s, 0);
                return 0;
            }
        }
    }

    return -1;
}

int parseCmd(char * s)
{
    int ret = -1;
    char * rest = NULL;
    int eCmdIdx = parseToken(s, &rest);
    //char err[100];

    switch ((enum eKeywords)eCmdIdx)
    {
        case set : ret = parseSetCmd(&rest); break;
        case get : ret = parseGetCmd(&rest); break;
        default:
            printf("Unexpected cmd '%s'." NLRF,s); //s is not the whole string because of strtok
            break;
    }

    if (0 <= ret)
    {
        eCmdIdx = parseToken(NULL, &rest);
        if (-2 != eCmdIdx)
        {
            printf("Cmd succeeded but with a subset of given args in '%s'" NLRF,s);
            ret = 0;
        }
        printf("OK" NLRF);
    }
    return ret;
}

int parseSetCmd(char ** rest)
{
    int ret = -1;
    char const * current = *rest;
    enum eKeywords eCmdIdx = parseToken(NULL, rest);

    switch (eCmdIdx)
    {
        case all : 
        case pwm : 
        case color : ret = parseSetColor(rest); break;
        case mode : ret = parseSetMode(rest); break;
        case red : ret = parseSetRed(rest); break;
        case green : ret = parseSetGreen(rest); break;
        case blue : ret = parseSetBlue(rest); break;
        default: printf("Unexpected set cmd identifier '%s'." NLRF,current);
            break;
    }

    return ret;
}

int parseGetCmd(char ** rest)
{
    int ret = 0;
    char const * current = *rest;
    int eCmdIdx = parseToken(NULL, rest);

    switch ((enum eKeywords)eCmdIdx)
    {
        case color :
            printf("R 0x%x , G 0x%x , B 0x%x" NLRF, getRed(), getGreen(), getBlue());
            break;
        case mode :  printf("Opmode %d" NLRF,getMode()); break;
        case red :   printf("R 0x%x" NLRF,getRed()); break;
        case green : printf("G 0x%x" NLRF,getGreen()); break;
        case blue :  printf("B  0x%x" NLRF,getBlue()); break;
        default: 
            printf("Unexpected get cmd identifier '%s'." NLRF,current);
            ret = -1;
            break;
    }

    return ret;
}

int parseSetColor(char ** rest)
{
    unsigned short red = 0;
    unsigned short green = 0;
    unsigned short blue = 0;
    int ret = parseTripleValue(rest, &red, &green, &blue);

    if (-1 != ret)
    {
        setSteps(red, green, blue);
    }
    else
    {
        printf("Failed to parse all 3 RGB values in '%s'" NLRF,*rest);
    }

    return ret;
}

int setValue(char ** rest, setFunc setFn)
{
    unsigned short value = 0;
    int ret = parseValue(rest, &value);

    if ((-1 != ret) && setFn)
    {
        setFn(value);
    }
    else
    {
        printf("Failed to parse value in '%s'" NLRF,*rest);
    }

    return ret;
}

int parseSetMode(char ** rest)
{
    return setValue(rest, &setMode);
}

int parseSetRed(char ** rest)
{
    return setValue(rest, &setRed);
}

int parseSetGreen(char ** rest)
{
    return setValue(rest, &setGreen);
}

int parseSetBlue(char ** rest)
{
    return setValue(rest, &setBlue);
}

