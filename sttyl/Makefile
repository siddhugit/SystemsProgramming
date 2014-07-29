#
# makefile for homework - 3
#

####### Compiler and options

CC            = gcc
CFLAGS        = -Wall

####### Files

sttyl: sttyl.o TCProperty.o
	$(CC) $(CFLAGS) -o sttyl sttyl.o TCProperty.o

TCProperty.o: TCProperty.c sttyl.h
	$(CC) $(CFLAGS) -c TCProperty.c

sttyl.o: sttyl.c sttyl.h
	$(CC) $(CFLAGS) -c sttyl.c

clean:
	rm -r sttyl sttyl.o TCProperty.o

