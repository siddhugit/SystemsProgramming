/*
*   Declaration of game paddle interface
*/

#ifndef PADDLE_H__
#define PADDLE_H__

int paddle_contact(int y);
void paddle_init(void);
void paddle_up(void);
void paddle_down(void);
void paddle_undraw(void);

#endif //PADDLE_H__

