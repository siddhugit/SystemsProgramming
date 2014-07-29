#
# makefile for homework - 2
#

####### Compiler and options

CC            = gcc
CFLAGS        = -Wall

####### Files

tarc: tarc.o TarcFileManager.o
	$(CC) $(CFLAGS) -o tarc tarc.o TarcFileManager.o

TarcFileManager.o: TarcFileManager.c TarcFileManager.h TarcUstar.h
	$(CC) $(CFLAGS) -c TarcFileManager.c

tarc.o: tarc.c TarcFileManager.h TarcUstar.h
	$(CC) $(CFLAGS) -c tarc.c

clean:
	rm -r tarc tarc.o TarcFileManager.o

