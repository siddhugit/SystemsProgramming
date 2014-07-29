/*
*   Purpose : Contanis main() of the program. main() also receives command line
*   arguments. If run without command line arguments, the program displays
*   terminal settings from a local termios object filled by tcgetatt(2). 
*   If arguments given it sets them to the local termios object.
*   If any invalid argument is encountered in the process, program reports 
*   error and exits with failure exit status. If all arguments are good then
*   local termios object overwrites terminal settings by calling tcsetatt(2).
*
*   public interface : 
*   int main(int argc, char** argv)
*
*   Requirement: sttyl [SPECIALCHAR CHAR] [ [-]FLAG]
*   o No command line input  
*       o display terminal settings
*   o Command line input
*       o SPECIALCHAR = erase,kil,intr 
*       o CHAR is new terminal setting character for SPECIALCHAR
*       o if '-' given turns off FLAG, otherwise turns it on
*/

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sttyl.h"

//sttyl uses this macro to represent off state of a flag bit
#define STATE_OFF 0
//sttyl uses this macro to represent on state of a flag bit
#define STATE_ON 1

//mask to get first 7 bits of control characters  
#define CTRLCHARMASK 0x7f
//invalid argument error message
#define INVALIDARGMSG "Invalid Argument"
//error message for missing value in command line for cc array type
#define NOARGMSG "missing argument"

//PopertyTable is defined in TCProperty.c
extern const struct TC_Property PopertyTable[];

/*
*   prints "arg,msg" on stderr and exits with EXIT_FAILURE
*   parameters:
*       o const char* arg - first argument of error message
*       o const char* msg - second argument of error message
*   Return : void
*/
void sttylErr(const char* arg,const char* msg)
{
    fprintf(stderr,"%s : %s\n",arg,msg);
    exit(EXIT_FAILURE);
}

/*
*   converts control characters in ascii range to printable format,
*   otherwise reports NA.
*   parameters:
*       o cc_t c - control character to convert
*       o char* buff - fills buff with printable string
*   Return : void
*/ 
void asciiToStr(cc_t c,char* buff)
{
    int i = 0;//index for buff array
    
    if(c == _POSIX_VDISABLE)//character representing undefined termios setting 
        sprintf(buff,"%s","<undef>");
    else if(iscntrl(c)){//if control character
        buff[i++] = '^';
        //convert control characters to a printable format 
        buff[i++] = ((c + 'A' - 1) &  CTRLCHARMASK);
        buff[i] = '\0';
    }
    else if(isascii(c)){//no change for other ascii characters
        buff[i++] = c;
        buff[i] = '\0';
    }
    else{//for characters out of ascii range
        sprintf(buff,"%s","<NA>");
    }
}

/*
*   sets a terminal property value to local termios object.
*   parameters:
*       o struct termios *ttyp - pointer to termios object
*       o const struct TC_Property* attr - property to set
*       o unsigned char val - value of the property to set
*   Return : void
*/ 
void setProperty(struct termios *ttyp,const struct TC_Property* attr
                                     ,unsigned char val)
{
    tcflag_t* flagVal;//holds the reference to flag at the given offset
    cc_t* cc;//holds pointer to control char array
    
    if(attr->offset == CCOFFSET){//if control character type
        //get pointer to cc array 
        cc = (cc_t*)(((char*)ttyp) + attr->offset);
        cc[attr->propIndex] = val;//set value in ttyp
    }
    else{//if flag type
        //get pointer to flag to set value in ttyp(by reference)
        flagVal = (tcflag_t*)(((char*)ttyp) + attr->offset);
        if(val == STATE_ON)
            *flagVal |= attr->propIndex;//turn on the bit
        else
            *flagVal &= ~(attr->propIndex);//turn off the bit
    }
}

/*
*   displays a terminal property of a termios object.
*   parameters:
*       o struct termios *ttyp - pointer to termios object
*       o const struct TC_Property* attr - property to display
*   Return : void
*/ 
void showProperty(struct termios *ttyp,const struct TC_Property* attr)
{
    tcflag_t flagVal;//holds the flag value at the given offset
    cc_t val;//holds the control character value at the given offset
    cc_t* cc;//holds pointer to control char array
    //buff holds printable string for cc values,'<undef>' is 
    //the largest string it can hold so 8 is a reasonable choice
    char buff[8];  
    
    if(attr->offset == CCOFFSET){//if control character type
        //get control char array at offset in TC_Property struct
        cc = (cc_t*)(((char*)ttyp) + attr->offset);
        val = cc[attr->propIndex];//get value of property at propIndex
        //fill buff by getting a printable string from val
        asciiToStr(val,buff);
        printf("%s = %s; ",attr->propName,buff);
    }
    else{//if flag type
        //get flag value at offset in TC_Property struct 
        //depending upon offset it can represent lflags,oflags or iflags
        flagVal = *(tcflag_t*)(((char*)ttyp) + attr->offset);
        if(!( flagVal & attr->propIndex))//check if flag bit is off
            printf("-");//'-' is printed before property name
        printf("%s ",attr->propName);
    }
}

/*
*   iterates through property table and calls showproperty()
*   to display those properties of a termios object
*   parameters:
*       o struct termios *ttyp - pointer to termios object
*   Return : void
*/ 
void displayProperties(struct termios *ttyp)
{ 
    int i;//i - loop counter
    tcflag_t currOffset;//holds offset value to detrmine termios member type
    //display terminal settings for each attribute in table
    for( i = 0; PopertyTable[i].propName; ++i){
        if( i != 0 && currOffset != PopertyTable[i].offset)
            printf("\n");//new line for each group of settings
        //display settings for terminal's attr at ith row in table 
        showProperty(ttyp,&PopertyTable[i]);
        //set curroffset to determine group type
        currOffset = PopertyTable[i].offset;
    }
    printf("\n");
}

/*
*   Searches a property in property table by property name
*   parameters:
*       o const char* propName - property name
*   Return : If found returns pointer to row in table, otherwise NULL
*/    
const struct TC_Property* searchInTable(const char* propName)
{
    int i;//loo counter
    //search command line argument in table
    for( i = 0; PopertyTable[i].propName; ++i){
        if(strcmp(propName,PopertyTable[i].propName) == 0){
            return &PopertyTable[i];//return if found
        }
    }
    return NULL;//not found
}

/*
*   validates if argument for value of control character
*   has size more than one. If argument is not a single character
*   function reports error and exits with failure, otherwise
*   returns the character
*   parameters:
*       o const char* arg - control character argument value
*   Return : on success,returns the character  
*/
char validatePropertyArg(const char* arg)
{
    //if length of value for control character type is more than 1
    //sttyl supports single character value for control character settings 
    if(strlen(arg) > 1){
        sttylErr(arg,INVALIDARGMSG);//exit
    }
    return arg[0];//if good,return the character 
}

/*
*   Calls searchInTable() to search command line arguments in 
*   property table. 
*   Validate:, 
*       o if argument has leading '-' and argument is not of flag type
*       o if '-' is passed as a single character argument
*       o argument is not found in the table
*       o value is not provided for cc types in the command line
*       o if value provided for cc type has size more than one
*   If failed reports error and exits with failure.
*   Increments counter for iteration of comman line arguments.
*   fill out paramenter val with setting value
*   parameters:
*       o int* pindex - iteration counter of arguments
*       o int argc - number of command line arguments
*       o char* argv[] - array of command line arguments
*       o unsigned char* val - [out] filled with setting value
*   Return : on success, returns pointer to row in table   
*/
const struct TC_Property* findPropertyNVal(int* pindex,int argc,
                                    char* argv[],unsigned char* val)
{
    const struct TC_Property* propInfo;//holds property info from table
    //holds command line argument and inrement index
    char* clArg = argv[(*pindex)++];
    
    if(clArg[0] == '-'){//if command line argument starts with '-'
        if(strlen(clArg) == 1)//if nothing follows '-'
            sttylErr(clArg,INVALIDARGMSG);//exit
        propInfo  = searchInTable(clArg + 1);//search string that follows '-'
        //if arg not found in table or arg is not flag type
        //exit,args starting with '-' have to be flag type
        if(!propInfo || propInfo->offset == CCOFFSET)
            sttylErr(clArg,INVALIDARGMSG);
        *val = STATE_OFF;//arg is flag type and '-' indicates arg's bit is off
    }
    else{//no leading '-' in command line argument
        if(!(propInfo  = searchInTable(clArg)))
            sttylErr(clArg,INVALIDARGMSG);//exit if not found in table 
        if(propInfo->offset == CCOFFSET){//if contrl char type
            if((*pindex) == argc)//if value not provided in command line
                sttylErr(clArg,NOARGMSG);//exit
            //get cntrl characetr value and increment index
            *val = validatePropertyArg(argv[(*pindex)++]);//checks single char
        }
        else//flag type
            *val = STATE_ON;//no leading '-' indicates arg's bit is on
    }
    return propInfo;//returns found TC_Property row in table
}

/*
*   main calls this function to process search command line arguments.
*   It calls findPropertyNVal() to search and setProperty() to set tc values
*   in a local termios object. 
*   parameters:
*       o int argc - number of command line arguments
*       o char* argv[] - array of command line arguments
*       o struct termios *ttyp - pointer to local termios object
*   Return : void 
*/
void setProperties(int argc,char* argv[],struct termios *ttyp)
{ 
    unsigned char val;//holds argument value from command line 
    int i = 1;//loop counter for iterarting command line arguments
    //holds found TC_Property row from table that corresponds to 
    //command line argument
    const struct TC_Property* propInfo;
    
    while(i < argc){
        //findPropertyNVal increments loop counter
        //if argument is flag type counter increments by 1 
        //if argument is cntrl character val is set to next 
        //argument and counter increments by 2 
        propInfo = findPropertyNVal(&i,argc,argv,&val);
        //set argument value in a local termios variable 
        setProperty(ttyp,propInfo,val);
    }
}

/*
*   Main function of the program. get terminal settings using tcgetattr()
*   in a local termios object. If no command line arguments given calls
*   displayProperties() to display properties of termios, otherwise
*   calls setProperties() to set command line arguments to termios object.
*   If all the arguments are processed successfully calls tcsetattr() to
*   overwrite terminal settings with local termios object.
*   parameters:
*       o int argc - number of command line arguments
*       o char* argv[] - array of command line arguments
*   Return : exit status of program
*/
int main(int argc,char* argv[])
{
    struct	termios ttyinfo;/* this struct holds tty info */

    if ( tcgetattr( STDIN_FILENO , &ttyinfo ) == -1 ){   /* get info */
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    if(argc == 1){//no arguments : display terminal properties 
        showSpeed(&ttyinfo);//show baud rate
        printf("\n");
        displayProperties(&ttyinfo);//displays ctrl characters and flags
    }
    else{//set properties for given command line arguments
        //set properties in a local termios variable
        setProperties(argc,argv,&ttyinfo);
        //everything good so far, so set local termios into terminal
        if( tcsetattr(STDIN_FILENO , TCSANOW, &ttyinfo) ==  -1){
            perror( "tcsetattr");
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;//Success exit status
}
