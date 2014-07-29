#
# makefile for homework - 4
#

####### Compiler and options

CC            = gcc
CFLAGS        = -Wall
LFLAGS        = -lcurses -lrt

####### Files

pong: pong.o paddle.o game.o handlers.o ball.o
	$(CC) $(CFLAGS) -o pong \
        pong.o paddle.o game.o handlers.o ball.o $(LFLAGS) 
	    
paddle.o: paddle.c paddle.h pong.h ball.h
	$(CC) $(CFLAGS) -c paddle.c

ball.o: ball.c ball.h pong.h paddle.h game.h  
	$(CC) $(CFLAGS) -c ball.c

game.o: game.c game.h ball.h pong.h paddle.h
	$(CC) $(CFLAGS) -c game.c

handlers.o: handlers.c handlers.h ball.h  paddle.h game.h
	$(CC) $(CFLAGS) -c handlers.c

clean:
	rm -f pong pong.o paddle.o game.o handlers.o ball.o
