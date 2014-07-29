#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "TarcUstar.h"
#include "TarcFileManager.h"

#define USTAREMPTYBLOCKSZ 2

static int tar_fd = -1;
static int BytesInFile = 0;
static int ReservedSpace = 0;

/*
*   Sets file descriptor for tar file manager 
*/
void setTarFileDescriptor(int tarfd)
{
    tar_fd = tarfd;
}

/*
*   initialize next "size" bytes with null character in the tar file 
*/
static void initBlocks(int size)
{
    int bytesToWrite,currOffset;
    char buff[BUFFSIZE];
    memset(buff,0,sizeof(buff));
    //get current file cursor in tar file
    if((currOffset = lseek(tar_fd,0,SEEK_CUR)) == -1){
        perror("lseek");
        exit(ERROREXITSTATUS);
    }
    //lseek to end of initialized part of tar file
    if( lseek(tar_fd,ReservedSpace,SEEK_SET) == -1){
        perror("lseek");
        exit(ERROREXITSTATUS);
    }
    //initialize next "size" byte with null character
    while(size > 0){
        bytesToWrite = (size > BUFFSIZE) ? BUFFSIZE : size;
        if( write(tar_fd,buff,bytesToWrite) != bytesToWrite){
            perror("write");
            exit(ERROREXITSTATUS);
        }
        size -= bytesToWrite;
        ReservedSpace += bytesToWrite;
    }
    //reset file cursor to original offset
    if(lseek(tar_fd,currOffset,SEEK_SET) == -1){
        perror("lseek");
        exit(ERROREXITSTATUS);
    }
}

/*
*   Writes header data in header buff to the tar file 
*/
void writeHeaderBuffToFile(char* headerBuff)
{
    /*always allocates and initializes a chunk of 
    BLOCKLEN*BLOCKFACTOR bytes with null character 
    if reserved space is empty or filled*/
    if( BytesInFile >= ReservedSpace){
            initBlocks(BLOCKLEN*BLOCKFACTOR);
    }
    //move file cursor to next block of file
    if( lseek(tar_fd,BytesInFile,SEEK_SET) == -1){
        perror("lseek");
        exit(ERROREXITSTATUS);
    }
    //write header buffer to tar file
    if( write(tar_fd,headerBuff,USTARLEN) != USTARLEN){
        perror("write");
        exit(ERROREXITSTATUS);
    }
    //increment Bytes in File to a new position for next write 
    BytesInFile += BLOCKLEN;
}

/*
*   Writes file data in dataBuff to tar file 
*/
void writeDataBuffToFile(unsigned char* dataBuff,int bytesToWrite)
{
    /*always allocates and initializes a chunk of 
    BLOCKLEN*BLOCKFACTOR bytes with null character 
    if bytes in file plus bytes to write exceeds reserved space*/
    if( BytesInFile + bytesToWrite > ReservedSpace){
        initBlocks(BLOCKLEN*BLOCKFACTOR);
    }
    //move file cursor to next block of file
    if( lseek(tar_fd,BytesInFile,SEEK_SET) == -1){
        perror("lseek");
        exit(ERROREXITSTATUS);
    }
    //write to tar file
    if(write(tar_fd,dataBuff,bytesToWrite) != bytesToWrite){
        perror("write");
        exit(ERROREXITSTATUS);
    }
    //update bytes in file
    BytesInFile += bytesToWrite;
}

/*
*   update bytes in file to exact multiple of BLOCKLEN for the next
*   element in the archive. Call this when all file data 
*   is written to the tar file
*/
void writeDataFinished(void)
{
    //call this function when write file data is finished
    //update bytes in file to exact multiple of BLOCKLEN for next write
    if(!(BytesInFile % BLOCKLEN == 0)){
        BytesInFile = (BytesInFile - (BytesInFile % BLOCKLEN)) + BLOCKLEN;
    }
}

/*
*   Writes another 20 empty blocks if tar file has 
*   less than 2 empty blocks at the end. this function
*   is called when all elements in arvchive file have been
*   written to tar file 
*/
void writeArchiveFinished(void)
{
    int emptyBlocks;
    if(BytesInFile == 0){//return if nothing is written to tar file
        return;
    }
    emptyBlocks = (ReservedSpace - BytesInFile)/BLOCKLEN;
    //if tar file does not have 2 empty blocks at the end
    if(emptyBlocks < USTAREMPTYBLOCKSZ){
        initBlocks(BLOCKLEN*BLOCKFACTOR);
    }
}

