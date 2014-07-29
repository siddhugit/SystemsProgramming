/*
*   This object represents the pong game itself.
*   It handles the various events and life of the game.
*   It acts as a book keeper by managing time elapsed and number of
*   balls left in the game.
*/

#include <curses.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include "game.h"
#include "pong.h"
#include "ball.h"
#include "paddle.h"

/* clock position on screen */
#define CLOCKHOFFSET 24// vertical offset
#define CLOCKVOFFSET 2//horizontal offset



#define NUM_BALLS   3//number of balls allowed in the game

/* time to move range */
#define MAXIMUM_TTM 100//max time to move
#define MINIMUM_TTM 20//min time to move

#define	TICKS_PER_SEC	1000	// affects speed
#define TICKER_TIME     (1000/TICKS_PER_SEC)//ticker time in milliseconds

//game class
struct pong_game
{
    int balls_left;//balls left
    int msecs_elapsed;//time elapsed for game clock
};

static struct pong_game the_game;//singleton object for game

static int done = 0;//game is finished or not

/*
*   Randon number generator for ball's speed nad direction.
*   parameters:
*       o int *ttm - [out]filled with random value of ttm
*       o int *dir - [out]filled with random value of direction 
*   Return : void
*/
static void getRandSpeedandDir(int* ttm,int* dir)
{
    //generates random value for ttm between MAXIMUM_TTM and MINIMUM_TTM
    *ttm = MINIMUM_TTM + (rand() % (MAXIMUM_TTM - MINIMUM_TTM));
    //generates random value for direction ( 1 or -1 )
    *dir = (rand() % 2 ) ? 1 : -1;
}

/*
*   set timer for the program.
*   parameters:
*       o long n_msecs - timer interval in milliseconds
*   Return : 0 if success otherwise -1
*/
static int set_ticker( long n_msecs )
{
    struct itimerval new_timeset;//itimerval object
    long    n_sec, n_usecs;//seconds and microseconds

    n_sec = n_msecs / 1000 ;//integral part
    n_usecs = ( n_msecs % 1000 ) * 1000L ;//fractional part

    new_timeset.it_interval.tv_sec  = n_sec;        /* set reload  */
    new_timeset.it_interval.tv_usec = n_usecs;      /* new ticker value */
    new_timeset.it_value.tv_sec     = n_sec;      /* store this   */
    new_timeset.it_value.tv_usec    = n_usecs ;     /* and this     */

	//set timer for real time of process
	return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

/*
*   Accessor method for max TTM
*   Return : MAXIMUM_TTM
*/
int maxTTM(void)
{
    return MAXIMUM_TTM;
}

/*
*   Accessor method for mix TTM
*   Return : MAXIMUM_TTM
*/
int minTTM(void)
{
    return MINIMUM_TTM;
}

/*
*   initializes game object.
*   Return : void
*/
void game_init(void)
{
    the_game.balls_left = NUM_BALLS;//number of balls allowed
    the_game.msecs_elapsed = 0;//time elapsed
}

/*
*   updates game timer and refreshes clock on screen
*   Return : void
*/
void updateClock(void)
{
    char clockStr[CLOCKHOFFSET];//holds string for time elapsed
    int hours,minutes,seconds;//time values
    
    the_game.msecs_elapsed += TICKER_TIME;//increment game clock
    seconds = the_game.msecs_elapsed/1000;//get seconds
    minutes = seconds/60;//get minutes
    hours = minutes/60;//get hours
    //format time elapsed
    sprintf(clockStr,"TOTAL TIME : %02d:%02d:%02d",hours,
                                minutes % 60,seconds % 60);
    move(TOP_EDGE - CLOCKVOFFSET,RIGHT_EDGE - CLOCKHOFFSET);//move cursor
    addstr(clockStr);//draw clock
}

/*
*   updates balls left and draws it on screen
*   Return : void
*/
void updateBallsLeft(void)
{
    char str[16];//holds string for balls left
    
    //format message, balls left does not include one in play
    sprintf(str,"BALLS LEFT : %d",the_game.balls_left - 1);
    move(TOP_EDGE - CLOCKVOFFSET,LEFT_EDGE);//move cursor
    addstr(str);//draw message of balls left
}

/*
*   handles ball lost event. decrements balls left.
*   stop() and serve() if balls left
*   otherwise set game game finished flag to true
*   Return : void
*/
void ballLost(void)
{
    --the_game.balls_left;//decrements balls left
    
    if(the_game.balls_left > 0){//if balls left
        stop();//stop the game
        serve();//restart game
    }
    else{//no balls left
        game_quit();//quit game
    }
}

/*
*   quit game by setting flag for game finished
*/
void game_quit(void)
{
    done = 1;//set flag for game finished
}

/*
*   Gets random speed and direction of ball
*   Initializes ball with these values
*   Initializes paddle
*   set/reset timer
*   Return : void
*/
void serve(void)
{
    int xdir, ydir;//direction
    int xttm, yttm;//speed
    
    updateBallsLeft();//update balls left
    updateClock();//update clock 
    getRandSpeedandDir(&xttm,&xdir);//get X random speed and direction
    getRandSpeedandDir(&yttm,&ydir);//get Y random speed and direction
    ball_init(xttm,yttm,xdir,ydir);//initialize screen
    paddle_init();//initialize paddle
    refresh();//refresh screen
    sleep(1);//sleep between serves
    if(set_ticker(TICKER_TIME) == -1){//set timer
        perror("set timer failed");
        exit(EXIT_FAILURE);
    }
    
}

/*
*   stop game by killing timer
*   clears paddle and ball from screen
*   Return : void
*/
void stop(void)
{
    if(set_ticker(0) == -1){//kill timer
        perror("set timer failed");
        exit(EXIT_FAILURE);
    }
    paddle_undraw();//undraws paddle
    ball_undraw();//undraw ball
}

/*
*   Returns status of life of game
*   Return : 0 if game is still on, 1 if finished
*/
int isDone(void)
{
    return done;
}
