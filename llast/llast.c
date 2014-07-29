#include <stdio.h>
#include <utmp.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <libgen.h>
#include <parseOptions.h>
#include <wtmpBufferedAccess.h>

#define NORMAL 0 //Normal logout
#define SHUTDOWN 1 //shudown event
#define CRASH 2 //crash event

#define CURR_DATE_FMT "%a %b %e %H:%M"
#define PREV_DATE_FMT "%H:%M"
#define FILE_INFO_DATE_FMT "%a %b %e %H:%M:%S %Y"

#define STILL_LOGGED_IN_MSG "still logged in"
#define LOGGED_OUT_MSG "gone - no logout"
#define SHUTDOWN_OUT_MSG "down"

static struct utmp* curr_rec = NULL;
static struct utmp* prev_rec = NULL;
static int event = NORMAL;
static int shutdowntime;
static int systemboottime;


/*
*   Converts unix time to localtime, formats it 
*   and prints it on the standard output
*/
static void show_time(time_t timeval,char* format)
{
    char result[100];
    struct tm* tp = localtime(&timeval);
    strftime(result,100,format,tp);
    fputs(result,stdout);
}
/*
*   Formats and prints the utmp record on standard output
*/
static void show_info(struct utmp* currRec,char* logoutMsg)
{
    char ut_id_fmt[8];
    sprintf(ut_id_fmt, "%%-8.%lds ", (long)sizeof(currRec->ut_id));
    printf("%-8.8s ", currRec->ut_user );
    printf("%-12.12s ", currRec->ut_line );
    printf("%-16.16s ", currRec->ut_host );
    show_time(currRec->ut_time ,CURR_DATE_FMT);
    printf("%-21.21s",logoutMsg);
    putchar('\n');
}
/*
*   Calculates time elapsed between login and logout 
*   in days,hours and minutes, formats it and 
*   prints it on a buffer
*/
static void sprintTimeDiff(char* buff,int time1,int time2)
{
    int seconds,minutes,hours,days;
    int currBuffsz = 0;
    seconds = time2 - time1;
    days = seconds / 86400;
    hours = (seconds / 3600) - (days * 24);
    minutes = (seconds / 60) - (days * 1440) - (hours * 60);
    if(days > 0)
    {
        currBuffsz = strlen(buff);
        sprintf(buff + currBuffsz,"(%d+",days);
    }
    else
        sprintf(buff," (");	
    currBuffsz = strlen(buff);
    sprintf(buff + currBuffsz,"%02d:%02d)",hours,minutes);
}
/*
* Formats logout time and prints it on a buffer
*/
static void sprintfLogoutTime(char* logoutMsg,time_t logOutTime)
{
    struct tm* tp;
    sprintf(logoutMsg," - ");
    tp = localtime(&logOutTime);
    strftime(logoutMsg + 3,64,PREV_DATE_FMT,tp);
    sprintf(logoutMsg + 8," ");
    sprintTimeDiff(logoutMsg + 9,curr_rec->ut_time,logOutTime);
}
/*
*   Processes records for which ut_line is equal to terminal name 
*   passed to the program
*/
static void processTerminalRecord()
{
    char logoutMsg[64];
    if((prev_rec == NULL) && (curr_rec->ut_type == USER_PROCESS))
    {
        (isSystemWTMPUsed()) ? sprintf(logoutMsg,"   %s",STILL_LOGGED_IN_MSG) :
            sprintf(logoutMsg,"    %s",LOGGED_OUT_MSG);
        show_info(curr_rec,logoutMsg);
    }
    else if(prev_rec && (prev_rec->ut_type == DEAD_PROCESS) //handling normal logout
                    && (curr_rec->ut_type == USER_PROCESS))
    {
        sprintfLogoutTime(logoutMsg,prev_rec->ut_time);
        show_info(curr_rec,logoutMsg);
    }
    else if(prev_rec && (prev_rec->ut_type == USER_PROCESS)
	                && (curr_rec->ut_type == USER_PROCESS))
    {
        if(event == SHUTDOWN)//handling shutdown
        {
            sprintf(logoutMsg," - %s  ",SHUTDOWN_OUT_MSG);
            sprintTimeDiff(logoutMsg + 9,curr_rec->ut_time,shutdowntime);
        }
        else
	    sprintfLogoutTime(logoutMsg,prev_rec->ut_time);
        show_info(curr_rec,logoutMsg);
    }
    event = NORMAL;
}
/*
*   Processes the current record in wtmp file and passes to 
*   processTerminalRecord()if ut_line of current record is 
*   equal to terminal name passed to the program
*/
static void processCurrentRecord(struct utmp* currRec,struct utmp* prevRec)
{
    char* terminalName = getTerminalName();
    if(strcmp("shutdown",currRec->ut_user) == 0)
    {
        event = SHUTDOWN;
        shutdowntime = currRec->ut_time;
    }
    else if(strcmp("system boot",currRec->ut_line) == 0)
    {
        event = CRASH;
        systemboottime = currRec->ut_time;
    }
    else if(strcmp(terminalName,currRec->ut_line) == 0)
    {
        curr_rec = currRec;
        if(prev_rec != NULL
	        && (strcmp(curr_rec->ut_user,prev_rec->ut_user) == 0)
		        && curr_rec->ut_time == prev_rec->ut_time)
	        return;	//Ignore duplicate cases
        processTerminalRecord();
        memcpy(prevRec,curr_rec,sizeof(struct utmp));
        prev_rec = prevRec;
    }
}
/*
*   Fetches each record from wtmp file and passes 
*   to processCurrentRecord() to process it
*/
static void processRecords(char* filename,char* terminalName)
{
    struct tm* tp;struct utmp* currRec;
    struct utmp prevRec;
    char msg[64];
    int len,i; 
    if(utmp_open(filename) == -1)
    {
        fprintf (stderr,"llast: %s: No such file or directory\n",filename);
        exit(1);
    }
    len = utmp_len();
    for(i = len - 1; i >= 0; --i)//loop runs backwards to process wtmp records in decreasing order of time 
    {
        currRec = utmp_getrec(i);
        processCurrentRecord(currRec,&prevRec);
    }
    tp = localtime(&utmp_getrec(0)->ut_time);
    strftime(msg,64,FILE_INFO_DATE_FMT,tp);
    printf("\n%s begins %s\n",basename(filename),msg); //at the end prints the wtmp file created time
    utmp_close(filename);
}
/*
* main funtion of the program
*/
int main(int argc,char* argv[])
{  
    char* filename;
    char* terminalName;
    parseOptions(argc,argv);//parse command line arguments
    filename = getWtmpFileName();
    terminalName = getTerminalName();
    processRecords(filename,terminalName);	   
    return 0;
}

