/**
 **     smsh
 **		this one parses the command line into strings
 **		uses fork, exec, wait, and ignores signals
 **		It also uses the flexstr library to handle
 **		a the command line and list of args
 **/
 
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<signal.h>
#include	<sys/wait.h>
#include	"smsh.h"
#include	"varlib.h"
#include "process.h"

void setup()
/*
 * purpose: initialize shell
 * returns: nothing. calls fatal() if trouble
 */
{
	extern char **environ;

	VLenviron2table(environ);
	signal(SIGINT,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

void fatal(char *s1, char *s2, int n)
{
	fprintf(stderr,"Error: %s,%s\n", s1, s2);
	exit(n);
}

/*
*   Main function of the program
*   Ignores SIGINT and SIGQUIT
*   calls processInput and passes a file 
*   pointer to it. If a script name is provided 
*   in command line then file piter poits the 
*   script file otherwise it points to stdin
*   returns overall status of process status 
*/
int main(int argc,char* argv[])
{
	int result;
    enum PROCESSMODE mode = INTERACTIVE;    
    FILE* fp = stdin; //default is stdin   
    void setup();  

    setup();//ignore signals
    if(argc > 1){//if script name provided
        if((fp = fopen(argv[1],"r")) == NULL)//fp points to script file
            fatal("read",argv[1],1);
        mode = SCRIPT;//mode is changed to script
    }
    setMode(mode);
	result = processInput(fp);//process input file
	return result;
}

