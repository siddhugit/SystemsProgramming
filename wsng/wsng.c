/*
 * wsng.c - a web server
 *
 *    usage: ws [ -c configfilenmame ]
 * features: supports the GET and HEAD command only
 *           runs in the current directory
 *           forks a new child to handle each request
 *           needs many additional features 
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<string.h>
#include	<netdb.h>
#include	<errno.h>
#include	<unistd.h>
#include	<sys/types.h>
#include    <sys/socket.h>
#include    <arpa/inet.h>
#include    <sys/wait.h>
#include	<sys/stat.h>
#include    <fcntl.h>
#include	<sys/param.h>
#include	<signal.h>
#include    <time.h>
#include    <dirent.h>
#include	"socklib.h"
#include "mimetype.h"




#define	PORTNUM	80
#define	SERVER_ROOT	"."
#define	CONFIG_FILE	"wsng.conf"
#define	VERSION		"1"

#define	MAX_RQ_LEN	4096
#define	LINELEN		1024
#define	PARAM_LEN	128
#define	VALUE_LEN	512
#define SERVER_NAME "wsng_E215_1.0"//server name in header
#define CREATEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)//logfile create mode

char	myhost[MAXHOSTNAMELEN];
int	myport;
char	*full_hostname();

static char logFile[PARAM_LEN];//logfile name
int logfd;//log file descriptor

#define	oops(m,x)	{ perror(m); exit(x); }

/*
 * prototypes
 */

int	startup(int, char *a[], char [], int *);
void	read_til_crnl(FILE *);
void	process_rq( char *, FILE *);
void	bad_request(FILE *);
void	cannot_do(FILE *fp);
void	do_403(char *item, FILE *fp,int);
void	do_404(char *item, FILE *fp,int);
void	do_cat(char *f, FILE *fpsock,int isHeadReq);
void	do_exec( char *prog, FILE *fp,int isHeadReq);
void	do_ls(char *dir, FILE *fp,int isHeadReq);
int	ends_in_cgi(char *f);
char 	*file_type(char *f);
void	header( FILE *fp, int code, char *msg, char *content_type );
int	isadir(char *f);
char	*modify_argument(char *arg, int len);
void checkQueryString(char* item);
int	not_exist(char *f);
void	fatal(char *, char *);
void	handle_call(int,struct sockaddr_in *,int);
int	read_request(FILE *, char *, int);
char	*readline(char *, int, FILE *);
void child_handler(int signum);

/*
*   SIGCHLD handler
*/
void child_handler(int signum)
{
    //cleans up all terminated child processes
    while(waitpid(-1,NULL,WNOHANG) != -1);
}

int
main(int ac, char *av[])
{
	int 	sock, fd , clilen;
    struct sockaddr_in cli_addr;
	/* set up */
	sock = startup(ac, av, myhost, &myport);

	/* sign on */
	printf("wsng%s started.  host=%s port=%d\n", VERSION, myhost, myport);

    signal(SIGCHLD,child_handler);//collect child exit status to avoid zombies
    
    clilen = sizeof(cli_addr);
	/* main loop here */
	while(1)
	{
		fd    = accept( sock, (struct sockaddr *)&cli_addr 
		                         , (socklen_t *)&clilen );	/* take a call	*/
		if ( fd == -1 )
		{
		    //continue when accept returns due to interruption
		    if(errno == EINTR)
		        continue;
		    else //other error
			    perror("accept");
	    }
		else
			handle_call(fd,&cli_addr,clilen);		/* handle call	*/
	}
	return 0;
	/* never end */
}

/*
*   Does server logging this called from request handler 
*   process as well as a child process forked by request handler.
*   Writes logging message on stdout.
*   child process redirects standard output to log file so that
*   output goes to screen(in parent process) as well as log file.
*/
void doLogging(char* request,struct sockaddr_in* cli_addr,int clilen)
{
    time_t now;
    struct tm * timeinfo;
    char clientip[INET_ADDRSTRLEN];
    int ipAddr = cli_addr->sin_addr.s_addr;//client ip
    time ( &now );
    timeinfo = localtime ( &now );//requst date and time
    inet_ntop( AF_INET, &ipAddr, clientip, INET_ADDRSTRLEN );
    printf("%sgot a call from %s: request = %s\n", 
                    asctime(timeinfo),clientip,request);
    fflush(stdout);
}

/*
 * handle_call(fd) - serve the request arriving on fd
 * summary: fork, then get request, then process request
 *  forks a child process in request handler process to do logging.
 *  in logging process stdout is redirected to log file descriptor.
 *    rets: child exits with 1 for error, 0 for ok
 *    note: closes fd in parent
 */
void handle_call(int fd,struct sockaddr_in* cli_addr,int clilen)
{
	int	pid = fork();
	FILE	*fpin, *fpout;
	char	request[MAX_RQ_LEN];
	int logpid;

	if ( pid == -1 ){
		perror("fork");
		return;
	}

	/* child: buffer socket and talk with client */
	if ( pid == 0 )
	{
		fpin  = fdopen(fd, "r");
		fpout = fdopen(fd, "w");
		if ( fpin == NULL || fpout == NULL )
			exit(1);

		if ( read_request(fpin, request, MAX_RQ_LEN) == -1 )
			exit(1);
		
        logpid = fork();//fork a process for logging
        if(logpid == -1){
            oops("fork for logging()",2);
        }
        else if(logpid == 0){
            dup2(logfd,1);//redirect stdout to log file
        }
        else{
		    process_rq(request, fpout);
		    fflush(fpout);		/* send data to client	*/
		}
		//do logging on screen and in log file
		doLogging(request,cli_addr,clilen);
		close(logfd);
		exit(0);		/* child is done	*/
		    			    /* exit closes files	*/
	}
	/* parent: close fd and return to take next call	*/
	close(fd);
}

/*
 * read the http request into rq not to exceed rqlen
 * return -1 for error, 0 for success
 */
int read_request(FILE *fp, char rq[], int rqlen)
{
	/* null means EOF or error. Either way there is no request */
	if ( readline(rq, rqlen, fp) == NULL )
		return -1;
	read_til_crnl(fp);
	return 0;
}

void read_til_crnl(FILE *fp)
{
        char    buf[MAX_RQ_LEN];
        while( readline(buf,MAX_RQ_LEN,fp) != NULL 
			&& strcmp(buf,"\r\n") != 0 )
                ;
}

/*
 * readline -- read in a line from fp, stop at \n 
 *    args: buf - place to store line
 *          len - size of buffer
 *          fp  - input stream
 *    rets: NULL at EOF else the buffer
 *    note: will not overflow buffer, but will read until \n or EOF
 *          thus will lose data if line exceeds len-2 chars
 *    note: like fgets but will always read until \n even if it loses data
 */
char *readline(char *buf, int len, FILE *fp)
{
        int     space = len - 2;
        char    *cp = buf;
        int     c;

        while( ( c = getc(fp) ) != '\n' && c != EOF ){
                if ( space-- > 0 )
                        *cp++ = c;
        }
        if ( c == '\n' )
                *cp++ = c;
        *cp = '\0';
        return ( c == EOF && cp == buf ? NULL : buf );
}
/*
 * initialization function
 * 	1. process command line args
 *		handles -c configfile
 *	2. open config file
 *		read rootdir, port
 *	3. chdir to rootdir
 *	4. open a socket on port
 *	5. gets the hostname
 *	6. return the socket
 *       later, it might set up logfiles, check config files,
 *         arrange to handle signals
 *
 *  returns: socket as the return value
 *	     the host by writing it into host[]
 *	     the port by writing it into *portnump
 */
int startup(int ac, char *av[],char host[], int *portnump)
{
	int	sock;
	int	portnum     = PORTNUM;
	char	*configfile = CONFIG_FILE ;
	int	pos;
	void	process_config_file(char *, int *);

    strcpy(logFile, "defaultlogname");
	for(pos=1;pos<ac;pos++){
		if ( strcmp(av[pos],"-c") == 0 ){
			if ( ++pos < ac )
				configfile = av[pos];
			else
				fatal("missing arg for -c",NULL);
		}
	}
	process_config_file(configfile, &portnum);
	logfd = open(logFile, O_WRONLY|O_APPEND|O_CREAT,CREATEMODE);
	if(logfd == -1)
	    oops("opening log file error",2);
	sock = make_server_socket( portnum );
	if ( sock == -1 ) 
		oops("making socket",2);
	strcpy(myhost, full_hostname());
	*portnump = portnum;
	return sock;
}

/*
*   creates a copy of string in dynamic storage.
*   caller is responsible to free the string returned.
*/
char* getcopy(const char* str)
{
    char* temp = malloc(strlen(str) + 1);
    strcpy(temp,str);
    return temp;
}

/*
 * opens file or dies
 * reads file for lines with the format
 *   port ###
 *   server_root path
 *   log file name
 *   mime types
 * at the end, return the portnum by loading *portnump
 * and chdir to the rootdir
 */
void process_config_file(char *conf_file, int *portnump)
{
	FILE	*fp;
	char	rootdir[VALUE_LEN] = SERVER_ROOT;
	char	param[PARAM_LEN];
	char	value[VALUE_LEN];
	char	type[VALUE_LEN];
	int	port;
	int	read_param(FILE *, char *, int, char *, int ,char*);

	/* open the file */
	if ( (fp = fopen(conf_file,"r")) == NULL )
		fatal("Cannot open config file %s", conf_file);

	/* extract the settings */
	while( read_param(fp, param, PARAM_LEN, value, VALUE_LEN,type) != EOF )
	{
		if ( strcasecmp(param,"server_root") == 0 )
			strcpy(rootdir, value);
		else if ( strcasecmp(param,"port") == 0 )
			port = atoi(value);
		else if ( strcasecmp(param,"LOGFILENAME") == 0 )
			strcpy(logFile, value);//log file name
		else if ( strcasecmp(param,"type") == 0 )
			addMimeType(getcopy(value),getcopy(type));//mime types
	}
	fclose(fp);

	/* act on the settings */
	if (chdir(rootdir) == -1)
		oops("cannot change to rootdir", 2);
	*portnump = port;
	return;
}

/*
 * read_param:
 *   purpose -- read next parameter setting line from fp
 *   details -- a param-setting line looks like  name value
 *		for example:  port 4444
 *     extra -- skip over lines that start with # and those
 *		that do not contain two strings
 *   returns -- EOF at eof and 1 on good data
 *
 */
int read_param(FILE *fp, char *name, int nlen, char* value, int vlen,
                                                        char* type)
{
	char	line[LINELEN];
	int	c;
	char	typefmt[100] ;

	sprintf(typefmt, "%%%ds%%%ds%%%ds", nlen, vlen ,vlen);

	/* read in next line and if the line is too long, read until \n */
	while( fgets(line, LINELEN, fp) != NULL )
	{
		if ( line[strlen(line)-1] != '\n' )
			while( (c = getc(fp)) != '\n' && c != EOF )
				;
		if ( sscanf(line, typefmt, name, value ,type) == 2 && *name != '#' )
			return 1;//server_root,port,log file name
	    else if( sscanf(line, typefmt, name, value,type ) == 3 && *name != '#' )
			return 1;//mime types
			
	}
	return EOF;
}
	


/* ------------------------------------------------------ *
   process_rq( char *rq, FILE *fpout)
   do what the request asks for and write reply to fp
   rq is HTTP command:  GET /foo/bar.html HTTP/1.0
   ------------------------------------------------------ */

void process_rq(char *rq, FILE *fp)
{
	char	cmd[MAX_RQ_LEN], arg[MAX_RQ_LEN];
	char	*item, *modify_argument();
	int isHeadReq;

	if ( sscanf(rq, "%s%s", cmd, arg) != 2 ){
		bad_request(fp);
		return;
	}
	item = modify_argument(arg, MAX_RQ_LEN);
	if ( strcmp(cmd,"GET") != 0 && strcmp(cmd,"HEAD") != 0)
		cannot_do(fp);//supports get and head only
    else{
        checkQueryString(item);//checks query string in request
        if(setenv("REQUEST_METHOD",cmd,1) == -1)//set http command in env
            fprintf(stderr,"setenv error : REQUEST_METHOD");
        isHeadReq = (strcmp(cmd,"HEAD") == 0);//If request command is HEAD
	    if ( not_exist( item ) )
		    do_404(item, fp,isHeadReq );//resource not found
	    else if ( isadir( item ) )
		    do_ls( item, fp,isHeadReq );//directory listing
	    else if ( ends_in_cgi( item ) )
		    do_exec( item, fp,isHeadReq );//cgi execution
	    else
		    do_cat( item, fp,isHeadReq );//file request
    }
}

/*
*   Checks if request has a query string.
*   If so sets QUERY_STRING env variable.
*/
void checkQueryString(char* item)
{
    char	*nexttoken;
    
    strtok(item, "?");
    if((nexttoken = strtok(NULL, "?")) != NULL){//sets query string as value of
        setenv("QUERY_STRING",nexttoken,1);     //QUERY_STRING env variable
    }
}

/*
 * modify_argument
 *  purpose: many roles
 *		security - remove all ".." components in paths
 *		cleaning - if arg is "/" convert to "."
 *  returns: pointer to modified string
 *     args: array containing arg and length of that array
 */

char *
modify_argument(char *arg, int len)
{
	char	*nexttoken;
	char	*copy = malloc(len);

	if ( copy == NULL )
		oops("memory error", 1);

	/* remove all ".." components from path */
	/* by tokeninzing on "/" and rebuilding */
	/* the string without the ".." items	*/

	*copy = '\0';

	nexttoken = strtok(arg, "/");
	while( nexttoken != NULL )
	{
		if ( strcmp(nexttoken,"..") != 0 )
		{
			if ( *copy )
				strcat(copy, "/");
			strcat(copy, nexttoken);
		}
		nexttoken = strtok(NULL, "/");
	}
	strcpy(arg, copy);
	free(copy);

	/* the array is now cleaned up */
	/* handle a special case       */
	if ( strcmp(arg,"") == 0 )
		strcpy(arg, ".");
	return arg;
}

/*
*   writes date header in a child process
*   by running system date command ad redirecting output to
*   socket connection
*/
void writeDate(FILE *fp)
{
    int pid;
    int fd = fileno(fp);
    
    pid = fork();
    if ( pid == -1 ){
		perror("fork");
		return;
	}
	if(pid == 0){//run date in child process
	    dup2(fd,1);
	    dup2(fd,2);
	    close(fd);
	    execlp("/bin/date","date",NULL);//exec date command
	    perror("writeDate");
	}
	else{
	    wait(NULL);//parent waits
	}
    
}

/* ------------------------------------------------------ *
   the reply header thing: all functions need one
   if content_type is NULL then don't send content type
   ------------------------------------------------------ */

void
header( FILE *fp, int code, char *msg, char *content_type )
{
	fprintf(fp, "HTTP/1.0 %d %s\r\n", code, msg);
	if ( content_type )
		fprintf(fp, "Content-type: %s\r\n", content_type );
    fprintf(fp, "server: %s\r\n", SERVER_NAME );
    fprintf(fp, "Date: ");
    fflush(fp);
    writeDate(fp);
}

/* ------------------------------------------------------ *
   simple functions first:
	bad_request(fp)     bad request syntax
        cannot_do(fp)       unimplemented HTTP command
    and do_404(item,fp)     no such object
   ------------------------------------------------------ */

void
bad_request(FILE *fp)
{
	header(fp, 400, "Bad Request", (char*)getMimeType("text"));
	fprintf(fp, "\r\nI cannot understand your request\r\n");
}

void
cannot_do(FILE *fp)
{
	header(fp, 501, "Not Implemented", (char*)getMimeType("text"));
	fprintf(fp, "\r\n");

	fprintf(fp, "That command is not yet implemented\r\n");
}

/*
*   writes header and body for 403 error.
*/
void
do_403(char *item, FILE *fp,int isHeadReq)
{
    printf("do_403() , item = %s\n",item);
	header(fp, 403, "Forbidden", (char*)getMimeType("html"));
	fprintf(fp, "\r\n");
    if(!isHeadReq){
	    fprintf(fp, "The request is \r\n FORBIDDEN\r\n");
	}
}

/*
*   writes header and body for 403 error.
*/
void
do_404(char *item, FILE *fp,int isHeadReq)
{
	header(fp, 404, "Not Found", (char*)getMimeType("text"));
	fprintf(fp, "\r\n");
    if(!isHeadReq){
	    fprintf(fp, "The item you requested: %s\r\nis not found\r\n", 
			item);
	}
}

/* ------------------------------------------------------ *
   the directory listing section
   isadir() uses stat, not_exist() uses stat
   do_ls runs ls. It should not
   ------------------------------------------------------ */

int
isadir(char *f)
{
	struct stat info;
	return ( stat(f, &info) != -1 && S_ISDIR(info.st_mode) );
}

int
not_exist(char *f)
{
	struct stat info;

	return( stat(f,&info) == -1 && errno == ENOENT );
}

/*
*   Craetes an html output for directory listing
*   so that files directories appear as links on browser.
*   directory lists shows directory and regular files only
*/
void createLink(const char* name,struct stat* info,FILE* fp)
{
    char buff[20];
    //modification time of a file
    strftime(buff, 20, "%Y-%m-%d %H:%M", localtime(&info->st_mtime)); 
    if(S_ISDIR(info->st_mode)){
        fprintf(fp,"<a href=\"%s/\">%s/</a>\t",name,name);//dir name
        fprintf(fp,"%s\t",buff);//dir modification time
        fprintf(fp,"-\r\n");
    }
    else if(S_ISREG(info->st_mode)){//regular file
        fprintf(fp,"<a href=\"%s\">%s</a>\t",name,name);//file name
        fprintf(fp,"%s\t",buff);//file modification time
        fprintf(fp,"%d\r\n",(int)info->st_size);//file size
    }
}

/*
*   reads a directory and lists the contents in 
*   a html format and writes it socket connection.
*/
void listDir(const char* dir, FILE *fp,int isHeadReq,DIR* dirp)
{
    
    struct stat info;
    char path[1024];
    struct dirent* entry;
   
    header(fp, 200, "OK", (char*)getMimeType("html"));
    fprintf(fp,"\r\n");
    fflush(fp);
    if(!isHeadReq)//if GET reuest
    {
        fprintf(fp,"<html><body><pre><hr>");
        while ((entry = readdir(dirp)) != NULL) {
            if (entry->d_name[0] != '.') {
                strcpy(path,dir);
                strcat(path,"/");
                strcat(path, entry->d_name);
                if(stat(path,&info) != 0)
                    perror("stat");
                else{
                    createLink(entry->d_name,&info,fp);
                }
            }
        }
        fprintf(fp,"<hr></pre></body></html>");
    } 
}

/*
*   Return 1 if directory contains index.cgi or
*   index.html. If directory contains index file(html or cgi)
*   then it calls do_cat/do_exec to process it.  
*/

int indexFile(const char* path,const char* ext, FILE *fp,int isHeadReq)
{
    char fileName[1024];
    int ret;
    
    strcpy(fileName,path);
    strcat(fileName,ext);
    ret = !not_exist(fileName);//if index file exists
    if(ret && (strcasecmp(".html",ext) == 0) )
        do_cat(fileName,fp,isHeadReq);//cat html
    if(ret && (strcasecmp(".cgi",ext) == 0) )
        do_exec(fileName,fp,isHeadReq);//exec cgi file
    return ret;
}


/*
 * lists the directory named by 'dir' 
 * sends the listing to the stream at fp
 */
void
do_ls(char *dir, FILE *fp,int isHeadReq)
{
    char path[1024];
    DIR* dirp;
    
    if( (dirp = opendir(dir)) == NULL){//directory permission error
        perror("opendir()");
        do_403((char*)dir,fp,isHeadReq);//forbidden
        return;
   }
    strcpy(path,dir);
    strcat(path,"/index");
    if(!indexFile(path,".html",fp,isHeadReq) && //if index file do not exist
                !indexFile(path,".cgi",fp,isHeadReq)){
	    listDir(dir,fp,isHeadReq,dirp);
    }
}

/* ------------------------------------------------------ *
   the cgi stuff.  function to check extension and
   one to run the program.
   ------------------------------------------------------ */

char *
file_type(char *f)
/* returns 'extension' of file */
{
	char	*cp;
	if ( (cp = strrchr(f, '.' )) != NULL )
		return cp+1;
	return "";
}

int
ends_in_cgi(char *f)
{
	return ( strcmp( file_type(f), "cgi" ) == 0 );
}

/*
* exec cgi file and write output to socket connection.
* If request is HEAD then calls popen() to run cgi file
* and read output till http header ( ends with a blank line)
* and write it to socket connection
*/
void
do_exec( char *prog, FILE *fp,int isHeadReq)
{
	int	fd = fileno(fp);
    char line[LINELEN];
    FILE* pfpipe;
    
	header(fp, 200, "OK", NULL);
	fflush(fp);
    if(!isHeadReq){//if GET request, exec cgi file
	    dup2(fd, 1);
	    dup2(fd, 2);
	    execl(prog,prog,NULL);//exec and write output to socket 
	    perror(prog);
	}
	else{//if HEAD request exec cgi file and read output 
	    pfpipe = popen(prog,"r");        //till http header(blank line)
	    while( fgets(line, LINELEN, pfpipe) != NULL ){//read cgi output
	        fprintf(fp, "%s",line);//write to socket connection
	        if((strcmp(line,"\r\n") == 0) || (strcmp(line,"\n") == 0))
	            break;//read till blank line
	    }
	    pclose(pfpipe);//close pipe connection to avoid zombies
	}
}
/* ------------------------------------------------------ *
   do_cat(filename,fp)
   sends back contents after a header
   ------------------------------------------------------ */

void
do_cat(char *f, FILE *fpsock,int isHeadReq)
{
	char	*extension = file_type(f);
	char	*content = (char*)getMimeType("text");
	FILE	*fpfile;
	int	c;

	content = (char*)getMimeType(extension);

	fpfile = fopen( f , "r");
	if ( fpfile != NULL )
	{
		header( fpsock, 200, "OK", content );
		fprintf(fpsock, "\r\n");
		if(!isHeadReq){
		    while( (c = getc(fpfile) ) != EOF )
			    putc(c, fpsock);
		    fclose(fpfile);
		}
	}
	else{
	    do_403(f,fpsock,isHeadReq);//file open error, forbidden
	}
}

char *
full_hostname()
/*
 * returns full `official' hostname for current machine
 * NOTE: this returns a ptr to a static buffer that is
 *       overwritten with each call. ( you know what to do.)
 */
{
	struct	hostent		*hp;
	char	hname[MAXHOSTNAMELEN];
	static  char fullname[MAXHOSTNAMELEN];

	if ( gethostname(hname,MAXHOSTNAMELEN) == -1 )	/* get rel name	*/
	{
		perror("gethostname");
		exit(1);
	}
	hp = gethostbyname( hname );		/* get info about host	*/
	if ( hp == NULL )			/*   or die		*/
		return NULL;
	strcpy( fullname, hp->h_name );		/* store foo.bar.com	*/
	return fullname;			/* and return it	*/
}


void fatal(char *fmt, char *str)
{
	fprintf(stderr, fmt, str);
	exit(1);
}
