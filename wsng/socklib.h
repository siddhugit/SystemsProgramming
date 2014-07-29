/*
 *	socklib.h
 *
 *	This file contains functions used lots when writing internet
 *	client/server programs.  The two main functions here are:
 *
 *	make_server_socket( portnum )	returns a server socket
 *					or -1 if error
 *
 *	connect_to_server(char *hostname, int portnum)
 *					returns a connected socket
 *					or -1 if error
 */ 

int make_server_socket( int );
int connect_to_server( char *, int );
