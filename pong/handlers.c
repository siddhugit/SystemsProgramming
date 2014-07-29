/*
*   Defines SIGIO and SIGALRM handlers
*/
#include <stdio.h>
#include <signal.h>
#include <aio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "handlers.h"
#include "paddle.h"
#include "ball.h"
#include "game.h"

static struct aiocb kbcbuf;	/* an aio control buf   */

/*
*   SIGALRM handler
*   Parameter:
*       o int signum : signal number
*   Return : void
*/
static void ball_move_h(int signum)
{
    updateClock();//update game clock
    ball_move();//handle ball motion
}

/*
*   set up aio read
*   Return : void
*/
void setup_aio_read(void)
{
    aio_read(&kbcbuf);//asynchronous read
}

/*
*   set SIGIO handler
*   Return : void
*/
void setIOhandler(void)
{
    static struct sigaction newhandler;//sigaction place holder
    static sigset_t blocked;//place holder for set of blocked signals
    
    memset(&newhandler,0,sizeof(struct sigaction));//initialize sigaction
    newhandler.sa_handler = on_input;//SIGIO signal handler
    sigemptyset(&blocked);//empty blocked set
    sigaddset(&blocked,SIGALRM);//add SIGALRM to set of blocked signals
    newhandler.sa_mask = blocked;//set blocked signals
    if(sigaction(SIGIO,&newhandler,NULL)){//set handler
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }
}

/*
*   set SIALRM handler
*   Return : void
*/
void setAlarmhandler(void)
{
    static struct sigaction newhandler;//sigaction place holder
    static sigset_t blocked;//place holder for set of blocked signals
    
    memset(&newhandler,0,sizeof(struct sigaction));//initialize sigaction
    newhandler.sa_handler = ball_move_h;//SIGALRM signal handler
    sigemptyset(&blocked);//empty blocked set
    sigaddset(&blocked,SIGIO);//add SIGIO to set of blocked signals
    newhandler.sa_mask = blocked;//set blocked signals
    if(sigaction(SIGALRM,&newhandler,NULL) == -1){//set handler
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }
}

/*
*   set aio buffer
*   Return : void
*/
void setup_aio_buffer(void)
{
    static char input[1];// 1 char of input 

    /* describe what to read */
    kbcbuf.aio_fildes     = STDIN_FILENO;// standard intput 
    kbcbuf.aio_buf        = input;//buffer     
    kbcbuf.aio_nbytes     = 1;// number to read  
    kbcbuf.aio_offset     = 0;// offset in file  

    /* describe what to do when read is ready */
    kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    kbcbuf.aio_sigevent.sigev_signo  = SIGIO;  /* send sIGIO   */
}

/*
*   SIGIO handler
*   Parameter:
*       o int signum : signal number
*   Return : void
*/
void on_input(int s)
{
    int  c;
    char *cp = (char *) kbcbuf.aio_buf;	/* cast to char * */

    /* check for errors */
    if ( aio_error(&kbcbuf) != 0 )
        perror("reading failed");
    else 
        /* get number of chars read */
        if ( aio_return(&kbcbuf) == 1 )
        {
            c = *cp;
            if(c == 'm')
                paddle_down();//move paddle down
            else if(c == 'k')
                paddle_up();//move paddle up
            else if(c == 'q' || c == 'Q')
                game_quit();//quit game
        }

    /* place a new request */
    aio_read(&kbcbuf);
}
