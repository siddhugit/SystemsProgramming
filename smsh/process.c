/* process.c    - supports builtins
 * command processing layer
 * 
 * The processInput(FILE* fp) function is called by the main loop
 * It sits in front of the execute() function.  This layer handles
 * two main classes of processing:
 * 	a) built-in functions (e.g. exit(), set, =, read, .. )
 * 	b) control structures (e.g. if, while, for)
 */
 
#include	<stdio.h>
#include <stdlib.h>
#include	"process.h"
#include	"smsh.h"
#include	"calltree.h"
#include	"splitline.h"


#define	DFL_PROMPT	"> "
#define	SCR_PROMPT	""

static enum PROCESSMODE procMode;

int builtin_command(char **, int *);

/*
*   set mode of the process (REGULAR or SCRIPT)
*/
void setMode(enum PROCESSMODE mode)
{
   procMode = mode; 
}

/*
*   Returns blank string if process running in script mode
*   or DFL_PROMPT if running in regular mode.
*/
const char* getPrompt()
{
    return (procMode == SCRIPT) ? SCR_PROMPT : DFL_PROMPT;
}

int process(char **args,FILE* fp)
/*
 * purpose: process user command
 * returns: result of processing command
 * details: if a built-in then call appropriate function, if not execute()
 *  errors: arise from subroutines, handled there
 */
{
	int		rv = 0;

	if ( args[0] == NULL )
		rv = 0;
	else if ( is_control_command(args[0]) ){
	    if((rv = buildCallTree(args,fp)) == 0){
	        rv = processCallTree();
	    }
	}
	else if ( !builtin_command(args,&rv) )
		rv = execute(args);
	return rv;
}

/*
*   called by main function to process file input.
*   calls next_cmd to get commands from input and 
*   call process() to process each line.
*/
int processInput(FILE* fp)
{
    char	*fullCmdLine,*cmdline, **arglist;
    const char* prompt;    
    int result = 0;

    prompt = getPrompt();
    while ( (cmdline = next_cmd(prompt, fp)) != NULL )
	{
        if( (fullCmdLine = varSubstitute(cmdline)) != NULL ){
		    if ( (arglist = splitline(fullCmdLine)) != NULL  ){

			    result = process(arglist,fp);

			    freelist(arglist);
		    }
		    free(fullCmdLine);
        }
        free(cmdline);
	}
    return result;
}
