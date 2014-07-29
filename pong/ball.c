/*
*   Defines the ball interface.
*   Draws and undraws ball.
*   handles ball movement with time.
*   checks if ball bounced or lost.
*/

#include <stdlib.h>
#include <curses.h>
#include "ball.h"
#include "pong.h"
#include "paddle.h"
#include "game.h"

#define BALL_SYMBOL 'o'//character to represent ball 

//ball class
struct pong_ball
{
    int x_pos,y_pos,//X and Y cordinates of ball
        x_dir,y_dir,//X and Y direction
        x_ttm,y_ttm,//X and Y time to move
        x_ttg,y_ttg,//X and Y time to go
        m_moved;//flag to check if ball is moved and redrawn
    char symbol;//character to represent ball 
};

static struct pong_ball the_ball;//singleton ball object

/*
*   randomize the speed of ball
*   called when ball hits paddle
*/
static void ball_randomizeSpeed(void)
{
    //randomize speed in X-direction
    the_ball.x_ttm = minTTM() + (rand() % (maxTTM() - minTTM()));
    //randomize speed in Y-direction
    the_ball.y_ttm = minTTM() + (rand() % (maxTTM() - minTTM()));
}

/*
*   checks if ball bounced or lost
*   if ball hits paddle speed of ball is slowed down.
*   Return : int
*       0 if neither bounced nor lost
*       1 if bounced
*       -1 if lost
*/
static int bounce_or_lose(void)
{
    int ret_val = 0;//return value of bounce or loose
    
    if ( the_ball.y_pos == TOP_EDGE ){//if ball hits top edge of court
		the_ball.y_dir = 1 ;//reverse direction
		ret_val = 1;//bounced
	} else if ( the_ball.y_pos == BOTTOM_EDGE - 1 ){//ball hits bottom edge
		the_ball.y_dir = -1 ;//reverse direction
		ret_val = 1;//bounced
	}
	if ( the_ball.x_pos == LEFT_EDGE ){//ball hits left edge
		the_ball.x_dir = 1 ;//reverse direction
		ret_val = 1;//bounced
	} else if ( the_ball.x_pos == RIGHT_EDGE - 1){//ball hits right edge
	    if(paddle_contact(the_ball.y_pos)){//ball hits paddle
		    the_ball.x_dir = -1;//reverse direction
            ball_randomizeSpeed();//randomize speed if ball hits paddle
		    ret_val = 1;//bounced
		}
		else{//ball misses paddle
		    ret_val = -1;//lost
		}
	}
	return ret_val;
}

/*
*   initializes ball object's position and speed and draws it on screen
*   parameters:
*       o int xTTM - X time to move
*       o int yTTM - Y time to move
*       o int xdir - x direction
*       o int ydir - y direction
*   Return : void
*/
void ball_init(int xTTM,int yTTM,int xdir,int ydir)
{
    the_ball.m_moved = 0;//initialized ball move flag to zero
    the_ball.y_pos = (TOP_EDGE + BOTTOM_EDGE)/2;//x center of court
	the_ball.x_pos = (LEFT_EDGE + RIGHT_EDGE)/2;//y center of court
	the_ball.y_ttg = the_ball.y_ttm = yTTM ;//x time to move
	the_ball.x_ttg = the_ball.x_ttm = xTTM ;//y time to move
	the_ball.y_dir = ydir;//y direction
	the_ball.x_dir = xdir;//x direction
	the_ball.symbol = BALL_SYMBOL ;//character symbol for ball 
	mvaddch( the_ball.y_pos, the_ball.x_pos, the_ball.symbol);//draw ball
	move(LINES-1,COLS-1);//park cursor
}

/*
*   ball move handler
*   First checks if ball has moved earlier so that we call 
*   bounce_or_loose only if ball moved.
*   if moved checks if ball bounced or lost.
*   bounce_or_loose is checked before moving the ball to ensure
*   that correct state of paddle and ball has been compared.
*   Otherwise if we move ball first and at the same time paddle 
*   is moved by pressing keyboard key by user we may loose paddle
*   movement while ball_move is running because during ball_move
*   SIGIO is blocked.
*   moves ball when x/y ticker reaches 0.
*   if moved resets x/y ticker otherwise decrements it.
*   lost redraws ball at a new position 
*   notifies if ball lost.
*   Return : void
*/
void ball_move(void)
{
	int	y_cur, x_cur;//x and y current position
    //check bounce_or_lose() first before moving the ball
    if( the_ball.m_moved && bounce_or_lose() == -1){//check if lost or bounced
	    ballLost();//notifies if ball lost
        return;//return if ball lost
	}
	y_cur = the_ball.y_pos ;		// old position
	x_cur = the_ball.x_pos ;
	the_ball.m_moved = 0 ;
	if ( the_ball.y_ttm > 0 && the_ball.y_ttg-- == 1 ){//y movement
		the_ball.y_pos += the_ball.y_dir ;	// move
		the_ball.y_ttg = the_ball.y_ttm  ;	// reset
		the_ball.m_moved = 1;
	}
	if ( the_ball.x_ttm > 0 && the_ball.x_ttg-- == 1 ){//y movement
		the_ball.x_pos += the_ball.x_dir ;	// move
		the_ball.x_ttg = the_ball.x_ttm  ;	// reset
		the_ball.m_moved = 1;
	}
	if ( the_ball.m_moved ){//if ball moved
		mvaddch( y_cur, x_cur, BLANK );//draw blank
		mvaddch( the_ball.y_pos, the_ball.x_pos, the_ball.symbol );//redraw	
		move(LINES-1,COLS-1);//park cursor
		refresh();//refresh screen
	}	
}

/*
*   replaces ball's character with blank
*   Return : void
*/
void ball_undraw(void)
{
    mvaddch( the_ball.y_pos, the_ball.x_pos, BLANK );//draw ball
    move(LINES-1,COLS-1);//park cursor
}
