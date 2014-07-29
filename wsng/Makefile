#
# Makefile for smsh
#
# Type  make    to compile the wsng program
#

####### Compiler and options

CC            = gcc
CFLAGS        = -Wall

####### Files


wsng: wsng.o socklib.o mimetype.o 
	$(CC) $(CFLAGS) -o wsng wsng.o socklib.o mimetype.o 

socklib.o: socklib.c socklib.h
	$(CC) $(CFLAGS) -c socklib.c

mimetype.o: mimetype.c mimetype.h
	$(CC) $(CFLAGS) -c mimetype.c

wsng.o: wsng.c mimetype.h socklib.h
	$(CC) $(CFLAGS) -c wsng.c


clean:
	rm -f *.o wsng
