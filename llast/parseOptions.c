#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define WTMP_FILE "/var/log/wtmp"
#define USAGE_ERROR_MSG "Usage: llast [-f file] tty.."

static char* terminal_name = NULL;
static char* wtmp_filename = WTMP_FILE;

/*
*   Parses Command line option
*/
static void getCommandOptions(int argc,char* argv[])
{
    int c;
    while ((c = getopt(argc, argv, "f:")) != -1)
    {
        switch (c)
        {
            case 'f':
              wtmp_filename = optarg;
              break;
            case '?':
              if (optopt == 'f') // handles -f option to get wtmp filename
              {
                  fprintf (stderr, "-%c requires a WTMP filename\n", optopt);
                  fprintf (stderr,"%s\n",USAGE_ERROR_MSG);				
                  exit(1);
              }
              else
              {
                  fprintf (stderr,"Unknown option character \'%c\'\n",optopt);
                  fprintf (stderr,"%s\n",USAGE_ERROR_MSG);
                  exit(1);
              }
            default :
              exit(1);
        }
    }
    terminal_name = argv[optind]; //terminal name, non-argument option
}
/*
*   main() calls this function and pass command line 
*   arguments to get terminal name and wtmp file name.
*/
void parseOptions(int argc,char* argv[])
{
    if(argc == 4 || argc == 2) //llast expects terminal name and/or wtmp filename with -f option
    {
        getCommandOptions(argc,argv);
    }
    else
    {
        fprintf (stderr,"%s\n",USAGE_ERROR_MSG); //usage: llast [-f file] tty..
        exit(1);
    }
}
/*
*   Returns wtmp file name
*/
char* getWtmpFileName()
{
    return wtmp_filename;
}
/*
*   Returns terminal name
*/
char* getTerminalName()
{
    return terminal_name;
}
/*
*   Returns true if llast is processing 
*   /var/log/wtmp, otherwise returns false
*/
int isSystemWTMPUsed()
{
    return (strcmp(wtmp_filename,WTMP_FILE) ==0);
}
