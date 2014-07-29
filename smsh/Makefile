#
# Makefile for smsh
#
# Type  make    to compile the smsh program
#

####### Compiler and options

CC            = gcc
CFLAGS        = -Wall

####### Files

smsh: smsh.o splitline.o execute.o process.o \
		builtin.o flexstr.o  varlib.o calltree.o
	$(CC) $(CFLAGS) -o smsh smsh.o splitline.o execute.o process.o \
		builtin.o flexstr.o  varlib.o calltree.o

splitline.o: splitline.c smsh.h flexstr.h varlib.h
	$(CC) $(CFLAGS) -c splitline.c

execute.o: execute.c varlib.h
	$(CC) $(CFLAGS) -c execute.c

process.o: process.c process.h smsh.h calltree.h splitline.h
	$(CC) $(CFLAGS) -c process.c

builtin.o: builtin.c varlib.h smsh.h splitline.h flexstr.h process.h
	$(CC) $(CFLAGS) -c builtin.c

flexstr.o: flexstr.c flexstr.h splitline.h
	$(CC) $(CFLAGS) -c flexstr.c

varlib.o: varlib.c varlib.h
	$(CC) $(CFLAGS) -c varlib.c

calltree.o: calltree.c splitline.h
	$(CC) $(CFLAGS) -c calltree.c

smsh.o: smsh.c smsh.h
	$(CC) $(CFLAGS) -c smsh.c

clean: 
	rm -f *.o smsh
