/*
*   Defines table of terminal settings sttyl supports and showSpeed()
*   function which maps B* constants from termios to output baud rate.
*
*   public interface :
*   const struct TC_Property PopertyTable[] : terminal settings table
*   void showSpeed(const struct termios *ttyp) : maps B* constants from
*                                          termios to output baud rate.
*/

#include <stdio.h>
#include "sttyl.h"

//table of properties sttyl supports 
//each array element represtents a row in the table
//each data member in a row represents column
//column 1 - property name
//column 2 - offset of group of settings represented as data member
//of termios struct to which the property belongs
//column 3 - index or bit mask of the property in the group
//NULL at the end represents end of the table   
const struct TC_Property PopertyTable[] = {
    {"intr",CCOFFSET,VINTR},
    {"erase",CCOFFSET,VERASE},
    {"kill",CCOFFSET,VKILL},
    {"start",CCOFFSET,VSTART},
    {"stop",CCOFFSET,VSTOP},
    {"werase",CCOFFSET,VWERASE},
    {"icrnl",IFOFFSET,ICRNL},
    {"olcuc",OFOFFSET,OLCUC},
    {"onlcr",OFOFFSET,ONLCR},
    {"isig",LFOFFSET,ISIG},
    {"icanon",LFOFFSET,ICANON},
    {"echo",LFOFFSET,ECHO},
    {"echoe",LFOFFSET,ECHOE},
    {NULL,0,0}
};

/*
*   prints output speed rate of a termios object
*   parameters:
*       const struct termios *ttyp - pointer to termios object 
*/
void showSpeed(const struct termios *ttyp)
{
    switch(cfgetospeed(ttyp)){
        case B50:
            printf("speed 50 baud; ");break;
        case B75:
            printf("speed 75 baud; ");break;
        case B110:
            printf("speed 110 baud; ");break;
        case B134:
            printf("speed 134 baud; ");break;
        case B150:
            printf("speed 150 baud; ");break;
        case B200:
            printf("speed 200 baud; ");break;
        case B300:
            printf("speed 300 baud; ");break;
        case B600:
            printf("speed 600 baud; ");break;
        case B1200:
            printf("speed 1200 baud; ");break;
        case B1800:
            printf("speed 1800 baud; ");break;
        case B2400:
            printf("speed 2400 baud; ");break;
        case B4800:
            printf("speed 4800 baud; ");break;
        case B9600:
            printf("speed 9600 baud; ");break;
        case B19200:
            printf("speed 19200 baud; ");break;
        case B38400:
            printf("speed 38400 baud; ");break;
        default:
            printf("Fast speed; ");
    }
}
