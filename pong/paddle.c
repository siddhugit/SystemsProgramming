/*
* Defines paddle interface of the game.
*/

#include <curses.h>
#include "paddle.h"
#include "pong.h"
#include "ball.h"

#define PADDLE_SYMBOL '#'//paddle character symbol

//paddle class
struct pong_paddle
{
    int pad_top;//paddle top edge position
    int pad_bot;//paddle bottom edge position
    int pad_col;//paddle column position
    char symbol;//character to represent paddle
};

static struct pong_paddle the_paddle;//singleton paddle object

/*
*   draw paddle on screen
*   Return : void
*/
static void paddle_draw(void)
{
    int i;//loop counter
    for(i = the_paddle.pad_top; i < the_paddle.pad_bot; ++i)
        mvaddch( i, the_paddle.pad_col, the_paddle.symbol);//draw paddle
}

/*
*   initializes paddle object and draws it on screen
*   Return : void
*/
void paddle_init(void)
{
    int i;//loop counter
    //one third of court height    
    int paddle_height = (BOTTOM_EDGE - TOP_EDGE)/3;
    the_paddle.pad_col =  RIGHT_EDGE;//paddle column
    //placing paddle symetrically at center of edge    
    the_paddle.pad_top = ((2*TOP_EDGE) + BOTTOM_EDGE)/3;
    the_paddle.pad_bot = the_paddle.pad_top + paddle_height;
    the_paddle.symbol = PADDLE_SYMBOL;//paddle symbol
    for(i = the_paddle.pad_top; i < the_paddle.pad_bot; ++i)
        mvaddch( i, the_paddle.pad_col, the_paddle.symbol);//draw paddle
    move(LINES-1,COLS-1);//park cursor    
}

/*
*   checks if ball lies between top and bottom edge of paddle
*   Parameters :
*           int ball_yPos - y-cordinate of ball
*   Return : int
*       0 if ball lies outside paddle
*       1 if ball lies inside paddle
*/
int paddle_contact(int ball_yPos)
{
    return (the_paddle.pad_top <= ball_yPos && 
            the_paddle.pad_bot >= ball_yPos);
}

/*
*   undraw paddle by replacing paddle char with blank
*   Return : void
*/
void paddle_undraw(void)
{
  int i;//loop counter
  for(i = the_paddle.pad_top; i < the_paddle.pad_bot; ++i)
        mvaddch( i, the_paddle.pad_col, BLANK);  //undraw paddle
}

/*
*   moves paddle up
*   Return : void
*/
void paddle_up(void)
{
    if(the_paddle.pad_top > TOP_EDGE){//upper limit of paddle
        paddle_undraw();//undraw paddle
        //decrement paddle position        
        --the_paddle.pad_top;
        --the_paddle.pad_bot;
        paddle_draw();//draw paddle
    }
}

/*
*   moves paddle down
*   Return : void
*/
void paddle_down()
{
    if(the_paddle.pad_bot < BOTTOM_EDGE){//lower limit of paddle
        paddle_undraw();//undraw paddle
        //increment paddle position 
        ++the_paddle.pad_top;
        ++the_paddle.pad_bot;
        paddle_draw();//undraw paddle
    }
}

