/*
*   Declaration of game public interface
*/

#ifndef GAME_H__
#define GAME_H__

void game_init(void);
void updateClock(void);
void updateBallsLeft(void);
void ballLost(void);  
void serve(void);
void stop(void); 
void game_quit(void); 
int isDone(void);
int maxTTM(void);
int minTTM(void);

#endif //GAME_H__





