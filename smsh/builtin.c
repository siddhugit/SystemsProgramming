/* builtin.c
 * contains the switch and the functions for builtin commands
 */

#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include    <unistd.h>
#include    <stdlib.h>
#include	"smsh.h"
#include	"varlib.h"
#include	"splitline.h"
#include	"flexstr.h"
#include    "process.h"

int assign(char *);
int okname(char *);

typedef int(*BUILT_IN_HANDLER)(const char*);

//data structure to hold command name and handler function
struct BuiltInHandler
{
    const char* builtInComm;
    BUILT_IN_HANDLER func;
};

/*
*   read handler
*   returns 1 if str is integer otherwise 0
*/
int readInput(const char* arg)
{
    FLEXSTR assignArg;
    int ret = 0;
    char* cmdLine = next_cmd("", stdin);
    
    if(arg != NULL){
        fs_init(&assignArg, 0);				/* initialize the str	*/
        fs_addstr(&assignArg,arg);
        fs_addch(&assignArg,'=');
        fs_addstr(&assignArg,cmdLine);
        ret = assign(fs_getstr(&assignArg));//assign
        fs_free(&assignArg);
    }
    free(cmdLine);
    return ret;
}

/*
*   converts string to integer
*   parameters : const char* str - string to convert 
*                int* ret - holds converted value
*   returns 1 if str is integer otherwise 0
*/
int isInteger(const char* str,int* ret)
{
    char* p;
    int temp;
    
    temp = strtol(str,&p,10);//covert
    if(*p == '\0'){//if str in int
        *ret = temp;
        return 1;
    }
    return 0;//if not int
}

/*
*   exit handler
*   returns 0 if success otherwise 1
*/
int exitShell(const char* arg)
{
    int exitStatus;
    
    if(arg == NULL){
        exitStatus = 0;
    }
    else{   //convert string to int
        if(!isInteger(arg,&exitStatus)){//if arg is not int
            fprintf(stderr,"%s : Invalid exit argument\n",arg);
            return 1;
        }
    }
    exit(exitStatus);//exit with exit status
}

/*
*   cd handler
*   returns 0 if success otherwise 1
*/
int changeDir(const char* path)
{
    int ret;
    
    if(path == NULL || strcmp(path,"~") == 0){//no path or ~
        path = getenv("HOME");//get home dir
    }
    if((ret = chdir(path)) == -1)//change dir
        perror(path);
    return ((ret == -1) ? 1 : 0);
}

/*
*   dot operator handler
*   returns 0 if success otherwise 1
*/
int execScript(const char* scrName)
{
    int ret;
    FILE* fp;

    if(scrName != NULL){
        fp = fopen(scrName,"r");
        ret = processInput(fp);//process included script file
        fclose(fp);
        return ret;
    }  
    return 1;
}

//built in command and handler function mapping
static struct BuiltInHandler builtHandlers[] =
    {{"cd",changeDir} , {"exit",exitShell} ,
     {"read",readInput} , {".",execScript} , {NULL,NULL}
    };

/*
*   Verifies if command is in(cd,read,exit,dot)
*   if found calls the corresponding handler function
*   retruns 0 if not found other wise 1
*/
int runBuiltInHandler(const char* command,const char* arg,int *resultp)
{
   int i;
   
    for(i=0; builtHandlers[i].builtInComm != NULL ; i++ )
    {
		if ( strcmp(command, builtHandlers[i].builtInComm) == 0 ){//found
		    *resultp = builtHandlers[i].func(arg);//call handler
		    return 1;
		}
	} 
	return 0;//not found
}

int builtin_command(char **args, int *resultp)
/*
 * purpose: run a builtin command 
 * returns: 1 if args[0] is builtin, 0 if not
 * details: test args[0] against all known builtins.  Call functions
 */
{
	int rv = 0;

	if ( strcmp(args[0],"set") == 0 ){	     /* 'set' command? */
		VLlist();
		*resultp = 0;
		rv = 1;
	}
	else if ( strchr(args[0], '=') != NULL ){   /* assignment cmd */
		*resultp = assign(args[0]);
		if ( *resultp != -1 )		    /* x-y=123 not ok */
			rv = 1;
	}
	else if ( strcmp(args[0], "export") == 0 ){
		if ( args[1] != NULL && okname(args[1]) )
			*resultp = VLexport(args[1]);
		else
			*resultp = 1;
		rv = 1;
	}//handles cd,exit,read and dot
	else if(runBuiltInHandler(args[0],args[1],resultp)){
	    rv = 1;
	}
	else if ( args[0][0] == '#' ){//comments handling
		rv = 1;
	}
	return rv;
}

int assign(char *str)
/*
 * purpose: execute name=val AND ensure that name is legal
 * returns: -1 for illegal lval, or result of VLstore 
 * warning: modifies the string, but retores it to normal
 */
{
	char	*cp;
	int	rv ;

	cp = strchr(str,'=');
	*cp = '\0';
	rv = ( okname(str) ? VLstore(str,cp+1) : -1 );
	*cp = '=';
	return rv;
}
int okname(char *str)
/*
 * purpose: determines if a string is a legal variable name
 * returns: 0 for no, 1 for yes
 */
{
	char	*cp;

	for(cp = str; *cp; cp++ ){
		if ( (isdigit((int)*cp) && cp==str) || !(isalnum((int)*cp) || *cp=='_' ))
			return 0;
	}
	return ( cp != str );	/* no empty strings, either */
}

int oknamechar(char c, int pos)
/*
 * purpose: used to determine if a char is ok for a name
 */
{
	return ( (isalpha((int)c) || (c=='_' ) ) || ( pos>0 && isdigit((int)c) ) );
}
