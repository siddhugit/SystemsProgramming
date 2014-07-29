/*
*   Conatins maain function of the program
*   Initializes game , starts game 
*   executes loop till game is finished and then returns.
*/

#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include "pong.h"
#include "paddle.h"
#include "handlers.h"
#include "game.h"

/*
*   utility function to draw horizontal line of court
*   Return : void
*/
static void addHLine(int sz)
{
    while(sz--)
        addch('-');
}

/*
*   utility function to draw vertical line of court
*   Return : void
*/
static void addVLine(int sz)
{
    int xpos,ypos;//x,y coordinates
    
    //get current cursor position
    xpos = getcurx(stdscr);
    ypos = getcury(stdscr);
    while(sz--)
    {
        addch('|');
        move(++ypos,xpos);//move cursor vertically down
    }
}

/*
*   utility function to draw pong court
*   Return : void
*/
static void draw_court(void)
{
    move(TOP_EDGE - 1,LEFT_EDGE);
    addHLine(RIGHT_EDGE - LEFT_EDGE);//draw top edge
    move(BOTTOM_EDGE,LEFT_EDGE);
    addHLine(RIGHT_EDGE - LEFT_EDGE);//draw bottom edge
    move(TOP_EDGE,LEFT_EDGE -1);
    addVLine(BOTTOM_EDGE - TOP_EDGE);//draw side edge
}

/*
*   initialize curses
*   Return : void
*/
static void init_curses(void)
{
	initscr();		/* turn on curses	*/
	noecho();		/* turn off echo	*/
	cbreak();		/* turn off buffering	*/
}

/*
*   de-initialize curses
*   Return : void
*/
static void wrap_up(void)
{
	endwin();		/* put back to normal	*/
}

/*
*   sets up the game
*   Return : void
*/
static void set_up(void)
{
    srand(getpid());//initialize random number generator
    draw_court();//draw pong court
    game_init();//initialize game
    setup_aio_buffer();//set up aio buffer
    setup_aio_read();//set up aio read
    setIOhandler();//set SIGIO handler
    setAlarmhandler();//set SIGALRM handler
}

/*
*   Main function of the program
*   Initializes game , starts game 
*   executes loop till game is finished and then returns.
*/
int main(void)
{   
    init_curses();//initialize curses
    set_up();//set up the game
    refresh();//refresh screen
    serve();//start game
    while(!isDone())//execute loop till game is finished
        pause();
    wrap_up();//wrap up
    return 0;//exit from process
}
   
