#include	<stdio.h>
#include	<strings.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<string.h>

/*
 *	socklib.c
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
 *
 *	history: 2010-04-16 replaced bcopy/bzero with memcpy/memset
 *	history: 2005-05-09 added SO_REUSEADDR to make_server_socket
 */ 

int
make_server_socket( int portnum )
{
        struct  sockaddr_in   saddr;   /* build our address here */
	int	sock_id;	       /* line id, file desc     */
	int	on = 1;		       /* for sockopt		 */

	/*
	 *      step 1: build our network address
	 *               domain is internet, hostname is any address
	 *               of local host, port is some number
	 */

	memset(&saddr, 0, sizeof(saddr));          /* 0. zero all members   */

	saddr.sin_family = AF_INET;		   /* 1. set addr family    */
	saddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 2. and IP addr	    */
	saddr.sin_port = htons(portnum);	   /* 3. and the port 	    */

	/*
	 *      step 2: get socket, set option, then then bind address
	 */

	sock_id = socket( PF_INET, SOCK_STREAM, 0 );    /* get a socket */
	if ( sock_id == -1 ) return -1;
	if ( setsockopt(sock_id,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) == -1 )
		return -1;
	if ( bind(sock_id,(struct sockaddr*)&saddr, sizeof(saddr)) ==  -1 )
	       return -1;

	/*
	 *      step 3: tell kernel we want to listen for calls
	 */
	if ( listen(sock_id, 1) != 0 ) return -1;
	return sock_id;
}


int
connect_to_server( char *hostname, int portnum )
{
	struct sockaddr_in  servadd;        /* the number to call */
	struct hostent      *hp;            /* used to get number */
	int    sock_id;			    /* returned to caller */

        /*
         *  build the network address of where we want to call
         */

       memset( &servadd, 0, sizeof( servadd ) );   /* 0. zero the address   */
       servadd.sin_family = AF_INET ;              /* 1. fill in addr type  */

       hp = gethostbyname( hostname );		   /* 2. and host addr      */
       if ( hp == NULL ) return -1;
       memcpy( &servadd.sin_addr, hp->h_addr, hp->h_length);
       servadd.sin_port = htons(portnum);          /* 3. and port number    */

       /*
        *        make the connection
        */

       sock_id = socket( PF_INET, SOCK_STREAM, 0 );    /* get a line   */
       if ( sock_id == -1 ) return -1;                 /* or fail      */
                                                       /* now dial     */
       if ( connect(sock_id,(struct sockaddr*)&servadd, sizeof(servadd)) !=0 )  
               return -1;
       return sock_id;
}
