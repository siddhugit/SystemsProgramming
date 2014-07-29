#ifndef STTYL_H__
#define STTYL_H__

/*
*   Contains:
*   Definition of struct for row of property table
*   Definition of offset of termios data members
*/

#include <stddef.h> //offsetof(3)
#include <termios.h> //struct termios

//offset of iflags member in struct termios 
#define IFOFFSET offsetof(struct termios, c_iflag)
//offset of oflags member in struct termios
#define OFOFFSET offsetof(struct termios, c_oflag)
//offset of lflags member in struct termios
#define LFOFFSET offsetof(struct termios, c_lflag)
//offset of cc member in struct termios
#define CCOFFSET offsetof(struct termios, c_cc)

//struct represents row type 
struct TC_Property
{
    //property name e.g. kill,erase,echo etc
    const char* propName;
    //offset of data members which represent group of flags 
    //or cntrl character array in struct termios
    size_t offset;   
    //index in case of cntrol character type
    //bit mask in case of flag type
    tcflag_t propIndex;
};

void showSpeed(const struct termios *ttyp);

#endif //STTYL_H__
