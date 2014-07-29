#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <utmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#define NUMRECS 16 //Maximum number of utp objects in buffer
#define UTMPSIZE (sizeof(struct utmp))
#define DATE_FMT "%a %b %e %H:%M"

static int wtmp_fd = -1;
static int num_records = -1;
static unsigned char utmpBuff[NUMRECS*UTMPSIZE];
static int begin_pos = -1;
static int end_pos = -1;

/*
*   Returns true if utmp object referred by index is 
*   in the buffer otherwise returns false
*/
static int isInBuffer(int index)
{
    return(index >= begin_pos && index < end_pos);
}
/*
*   Reloads buffer  
*/
static int reload(int index)
{
    int beginPos,amtRead,recsRead;
    beginPos = (index < NUMRECS) ? 0 : index - NUMRECS + 1; //begin pos is set to NUMRECS=16 objects behind index 
    if(lseek(wtmp_fd, beginPos*UTMPSIZE, SEEK_SET) == -1) //lseek to begin pos
    {
        return -1;
    }
    amtRead = read(wtmp_fd,utmpBuff,NUMRECS*UTMPSIZE); //reads NUMRECS utmp objects into buffer
    if(amtRead < 0)
    {
        return -1;
    }
    begin_pos = beginPos;
    recsRead = amtRead/UTMPSIZE;
    end_pos = begin_pos + recsRead;
    return recsRead;
}
int utmp_open(char *filename)
{
    wtmp_fd = open(filename,O_RDONLY);
    return wtmp_fd;
}
/*
*   Returns number of utmp records in wtmp
*/
int utmp_len()
{
    off_t offset;    
    if(num_records == -1)
    {
        if((offset = lseek(wtmp_fd,0,SEEK_END)) == -1)//lseek to end of file so that offset is equal to number of bytes in file
        {
            fprintf(stderr,"lseek error\n");
            exit(1);
        }
        num_records = (int)offset/UTMPSIZE;//dividing file size by UTMPSIZE gives number of utmp objects in file
    }
    return num_records;
}
/*
*   Returns pointer to utmp record referred by index
*/
struct utmp* utmp_getrec(int index)
{
    if(!isInBuffer(index) && reload(index) < 0) //reloads if utmp object referred by index is not in buffer
    {
        return NULL;
    }
    return (struct utmp*)(utmpBuff + UTMPSIZE*(index - begin_pos));
}
/*
* Closes utmp file
*/
int utmp_close()
{
    int ok;
    if(wtmp_fd != -1)
    {
        ok = close(wtmp_fd);
        wtmp_fd = -1;
    }
    return ok;
}
