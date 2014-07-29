#
# makefile for homework - 1
#

####### Compiler and options

CC            = gcc
CFLAGS        = -Wall
INCPATH       = -I.

####### Files

llast: llast.o wtmpBufferedAccess.o parseOptions.o
	$(CC) -o llast llast.o wtmpBufferedAccess.o parseOptions.o

wtmpBufferedAccess.o: wtmpBufferedAccess.c
	$(CC) $(CFLAGS) $(INCPATH) -c wtmpBufferedAccess.c

parseOptions.o: parseOptions.c
	$(CC) $(CFLAGS) $(INCPATH) -c parseOptions.c

llast.o: llast.c
	$(CC) $(CFLAGS) $(INCPATH) -c llast.c

clean:
	rm -r llast llast.o wtmpBufferedAccess.o parseOptions.o

