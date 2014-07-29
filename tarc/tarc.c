#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tar.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include "TarcUstar.h"
#include "TarcFileManager.h"

#define MAXNUMFLDLEN 12

static int ExitStatus = 0;

static char *uid_to_name( uid_t uid )
/* 
 *	returns pointer to username associated with uid, uses getpw()
 */	
{
	struct	passwd *getpwuid(), *pw_ptr;
	static  char numstr[10];

	if ( ( pw_ptr = getpwuid( uid ) ) == NULL ){
            sprintf(numstr,"%d", uid);
            return numstr;
	}
	else
            return pw_ptr->pw_name ;
}

static char *gid_to_name( gid_t gid )
/*
 *	returns pointer to group number gid. used getgrgid(3)
 */
{
	struct group *getgrgid(), *grp_ptr;
	static  char numstr[10];

	if ( ( grp_ptr = getgrgid(gid) ) == NULL ){
            sprintf(numstr,"%d", gid);
            return numstr;
	}
	else
            return grp_ptr->gr_name;
}

/*
* returns check sum value of header
*/
static unsigned int computeCheckSum(char* headerBuff)
{
    unsigned int sum = 0;
    int i; 
    for(i = 0; i < USTARLEN; ++i)
	{
	    sum += (unsigned char)headerBuff[i];
	}
    //CHECKSUMMASK = 0xfff | 1 << 16
    //a number whose only 17 low order bits are on
    sum &= CHECKSUMMASK;//get low order 17 bits 
    return sum;
}

/*
* returns type of entity in ustar format
*/
static int getType(int filemode)
{
    if(S_ISREG(filemode))
        return REGTYPE;//regular  file
    else if(S_ISCHR(filemode))
        return CHRTYPE;//character device file
    else if(S_ISBLK(filemode))
        return BLKTYPE;//block device file
    else if(S_ISDIR(filemode))
        return DIRTYPE;//directory
    else if(S_ISFIFO(filemode))
        return FIFOTYPE;//FIFO
    else if(S_ISLNK(filemode))
        return SYMTYPE;//Symbolic link
    return UNKNOWN_TYPE; 
}

/*
*   Function removes leading slash if entity 
*   provided by user is referred by its absolute path
*/
static void removeLeadingSlash(char* fileName,char* tempFName)
{
    int len;
    len = strlen(fileName);
    memset(tempFName,0,FILENAME_LEN + 1);
    if(len > 1 && fileName[0] == '/'){
        //copy rest of characters leaving first slash
        strncpy(tempFName,fileName + 1,FILENAME_LEN);
    }
    else
        //no change if filename does not have leading slash
        strncpy(tempFName,fileName,FILENAME_LEN + 1);  
}

/*
*   Validates if entity name exceeds 100 characters.
*   In case of directory it also appends a slash at the end 
*/
static int validateElemName(char* elemName,char* fileName,int filetype)
{
    int len;
    len = strlen(elemName);
    if(len > FILENAME_LEN){
        fprintf(stderr,"%s : %s\n",elemName,LONGFILENAMEMSG);
        return -1;
    }
    //directory gets one byte less because last index is filled by slash
    else if(len == FILENAME_LEN && filetype == DIRTYPE){
        fprintf(stderr,"%s : %s\n",elemName,LONGFILENAMEMSG);
        return -1;
    }
    //initialize filename with null characters so that unfilled 
    //characters remain null
    memset(fileName,0,FILENAME_LEN + 1); 
    strncpy(fileName,elemName,len);
    if(filetype == DIRTYPE){
        fileName[len] = '/';//appends slash in directory name
    }      
    return 0;
}

/*
*   Copies string value to header buffer
*/
static void writeStringVal(char* buff,int offset,int size,char* val)
{
    strncpy(buff + offset,val,size);
}

/*
*   Copies numeric values to header buffer.
*   Copies numeric value with leading zeroes 
*   and number octet filled till last but one sapce 
*   and then ends with a null character
*/
static void writeIntVal(char* buff,int offset,int size,int val)
{
    //MAXNUMFLDLEN = 12 is a reasonable choice beacuse it is equal to maximum  
    //field length of numeric data in the ustar struct
    char temp[MAXNUMFLDLEN];
    //(size -1) ensures string buffer of a numeric values if
    //filled by  leading zeroes 
    //and number octet till last but one sapce 
    //and then ends with a null character
    sprintf(temp,"%0*o",size - 1,val);
    strncpy(buff + offset,temp,size);
}

/*
*   Validates if link name exceeds 100 characters
*   Used readlink(2) to get linkname/linksize of a symbolic link. 
*/
static int validateLinkname(char* filename,char* linkName)
{
    int linkSize = readlink(filename,linkName,LINKNAME_LEN + 1);
    if( linkSize == -1 ){
        perror("readlink");
        return -1;
    }
    if(linkSize > LINKNAME_LEN){
        fprintf(stderr,"%s : %s\n",filename,LONGLINKNAMEMSG);
        return -1;
    }
    //readlink() does not append a null byte
    linkName[linkSize] = '\0';
    return 0;
}

/*
*   writes header values to buffer at prescribed locations
*/
static void writeHeaderToBuff(char* headerBuff,char* fileName,int fileType,
                                int fileSize,struct stat* fileinfo)
{
    char linkName[LINKNAME_LEN + 1],tempFName[FILENAME_LEN + 1];
    unsigned int chcksum;
    removeLeadingSlash(fileName,tempFName);//if absolute path is given
    writeStringVal(headerBuff,FILENAME_OFFSET,FILENAME_LEN,tempFName);
    //filters out permission bits using PERMISSIONMASK
    writeIntVal(headerBuff,FILEMODE_OFFSET,FILEMODE_LEN,
                            fileinfo->st_mode & PERMISSIONMASK);
    writeIntVal(headerBuff,FILEUID_OFFSET,FILEUID_LEN,fileinfo->st_uid);
    writeIntVal(headerBuff,FILEGID_OFFSET,FILEGID_LEN,fileinfo->st_gid);
    writeIntVal(headerBuff,FILESIZE_OFFSET,FILESIZE_LEN,fileSize);
    writeIntVal(headerBuff,FILEMTIME_OFFSET,FILEMTIME_LEN,fileinfo->st_mtime);
    sprintf(headerBuff + FILETYPFLAG_OFFSET,"%c",fileType);
    if(fileType == SYMTYPE){//validates linkname size for sym links
        if( validateLinkname(fileName,linkName) != -1 ){
            writeStringVal(headerBuff,LINKNAME_OFFSET,LINKNAME_LEN,linkName);
        }
    }
    writeStringVal(headerBuff,MAGIC_OFFSET,MAGIC_LEN,TMAGIC);
    writeStringVal(headerBuff,VERSION_OFFSET,VERSION_LEN,TVERSION);
    writeStringVal(headerBuff,FILEUNAME_OFFSET,FILEUNAME_LEN,
                            uid_to_name(fileinfo->st_uid));
    writeStringVal(headerBuff,FILEGNAME_OFFSET,FILEGNAME_LEN
                            ,gid_to_name(fileinfo->st_gid));
    //fills checksum field in buffer with eight spaces
    writeStringVal(headerBuff,CHKSUM_OFFSET,CHKSUM_LEN,INITCHECKSUMVAL);
    chcksum = computeCheckSum(headerBuff);//computes checksum
    writeIntVal(headerBuff,CHKSUM_OFFSET,CHKSUM_LEN,chcksum);//actual chksum
}

/*
*   Returns true for file types that tarc supports,
*   otherwise returns false
*/
static int isSupportedFileType(int fileType)
{
    switch(fileType)
    {
        case REGTYPE:
        case DIRTYPE:
        case FIFOTYPE:
        case SYMTYPE:
            return 1;
        default:
            return 0;
    }
}

/*
*   Writes header part of file/directory to a buffer
*   and calls tar file TarFileManager::writeHeaderBuffToFile
*   to write buffer to tar file
*/
static void writeHeader(char* fileName,struct stat* fileinfo)
{
    char headerBuff[USTARLEN];
    int fileType,fileSize;
    memset(headerBuff,0,USTARLEN);//initialize head buff to zero bytes
    fileType = getType(fileinfo->st_mode);
    //reports error for file types not supported by tarc
    if( !isSupportedFileType(fileType) )
    {
        ExitStatus = ERROREXITSTATUS;
        fprintf(stderr,"%s : %s\n",fileName,NOSUPPORTMSG);
        return;
    }
	fileSize = (fileType == REGTYPE) ? fileinfo->st_size : 0;
	//writes to buffer
	writeHeaderToBuff(headerBuff,fileName,fileType,fileSize,fileinfo); 
	//writes buffer to tar file 
	writeHeaderBuffToFile(headerBuff);
}

/*
*   Reads data from file to be archived and
*   writes to tar file
*/
static void writeFileData(char* fileName,int filefd,struct stat* fileinfo)
{
    int bytesRead;
    unsigned char dataBuff[BUFFSIZE];
    while(( bytesRead = read(filefd,dataBuff,BUFFSIZE)))
    {
        //calls TarFileManager::writeDataBuffToFile to write 
        //file data to tar file 
        writeDataBuffToFile(dataBuff,bytesRead);
    }
    //calls TarFileManager::writeDataFinished after file data write
    //is finished so that tar file manager can update bytes in tar file
    //to exact multiple of BLOCKLEN
    writeDataFinished();
}

/*
*   Process every elements to be archived.
*   Calls funtions to write header and data in case of file types
*   to the archive.
*/
static void processElement(char* elemName,struct stat* fileinfo)
{
    int filefd,fileType,ret;
    char fileName[FILENAME_LEN + 1];
    fileType = getType(fileinfo->st_mode);
    //validates size of file/directory name
    ret = validateElemName(elemName,fileName,fileType);
    if(ret == -1)
        return;
    if(fileType == REGTYPE){
        //handles file no read error
        if (( filefd = open(fileName, O_RDONLY) ) == -1 ){
            perror(fileName);
            ExitStatus = ERROREXITSTATUS;
            return;
        }
        writeHeader(fileName,fileinfo);
        //writes data if file type is regular file
        writeFileData(fileName,filefd,fileinfo);
        close(filefd);
    }
    else{
        writeHeader(fileName,fileinfo);
    }
}

/*
* Calls lstat(2) to get file info, reports error if lstat fails
*/
static int getStat(char* elemName,struct stat* fileinfo)
{
    int ret;
    if((ret = lstat(elemName,fileinfo)) == -1){
        perror(elemName);
        ExitStatus = ERROREXITSTATUS; 
    }
    return ret;
}

/*
*   Main calls this function for each element to be archived in the 
*   passed by user as command line arguments.
*   For each element it calls processelement() in order to process
*   elements. For directory elements the function iterates over child 
*   elements and calls itself recursively for child elements. 
*   Function uses opendir(3),readdir(3) and closedir(3) to iterate over
*   child elements. opendir() is called before processing directory in
*   order to check file noread/noexecute errors.
*     
*/
static void traverse(char* elemName)
{
    struct dirent * direntP;struct stat fileinfo;
    DIR* dirp;int type;char elemNameBuff[BUFFSIZE];
    if(getStat(elemName,&fileinfo) == -1)
        return;//if lstat returns error
    if((type = getType(fileinfo.st_mode)) == DIRTYPE){
        if(!(dirp = opendir(elemName))){//handles dir noread/noexecute
            perror(elemName);
            ExitStatus = ERROREXITSTATUS;
            return;
        }
        //processElement call is delayed after opendir 
        //in order to handle dir noread/noexecute errors
        processElement(elemName,&fileinfo);
        while((direntP = readdir(dirp))){//get child elements of directory
            if(strcmp(direntP->d_name,".") == 0 || 
               strcmp(direntP->d_name,"..") == 0)
                continue;
            sprintf(elemNameBuff,"%s/%s",elemName,direntP->d_name); 
            //calls traverse to process child elements recursively
            traverse(elemNameBuff);
        } 
        closedir(dirp);
    }
    else//procees non directory elements 
        processElement(elemName,&fileinfo);
}

/*
*   Main function of the program 
*/
int main(int argc,char* argv[])
{
    int tarfd,i;
    char* elemName;
    if(argc > 2){//argv[1] is tar file name
        //CREATMODE = 644
        if (( tarfd = creat(argv[1], CREATEMODE) ) == -1 ){
	        perror(argv[1]);
	        exit(ERROREXITSTATUS);
        }
        //sets tar file descriptor for tar file manager
        setTarFileDescriptor(tarfd);
        for(i = 2; i < argc; ++i){
            //calls traverse for each element to be archived
            elemName = argv[i];
            traverse(elemName);
        }
        //make sure 2 empty blocks at the end of tar file
        writeArchiveFinished();
        close(tarfd);
    }
    else{//invalid command line option
        fprintf(stderr,"%s\n",USAGEMSG);
        exit(ERROREXITSTATUS);
    }
    return ExitStatus;//returns 0 if ok otherwise 1   
}
