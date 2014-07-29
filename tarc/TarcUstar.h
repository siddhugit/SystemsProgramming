#ifndef TARCUSTAR_H__
#define TARCUSTAR_H__


/*
*   Contains definition of constants needed by tarc program.
*/

#define LONGFILENAMEMSG "file or directory name is more than 100 characters"
#define LONGLINKNAMEMSG "link name is more than 100 characters"
#define USAGEMSG "Usage : tarc archivename dir-or-file1 [ dir-or-file2 ... ]"
#define NOSUPPORTMSG "file type not supported"

#define BLOCKLEN 512
#define BLOCKFACTOR 20
#define USTARLEN 500
#define BUFFSIZE 1024 //buffer size for disk I/O
#define UNKNOWN_TYPE -1 //unknown file type

#define ERROREXITSTATUS 1

//mask for permission bits in stat.st_mode 
#define PERMISSIONMASK (S_IRWXU | S_IRWXG | S_IRWXO)
//tar file creation mode
#define CREATEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

//Eight spaces for check sum field before computing check sum value
#define INITCHECKSUMVAL "        "
//check sum mask to get low order 17 bits
#define CHECKSUMMASK (0xffff | 1 << 16)

/* Ustar fields length and offset*/

#define FILENAME_OFFSET 0
#define FILENAME_LEN 100

#define FILEMODE_OFFSET 100
#define FILEMODE_LEN 8

#define FILEUID_OFFSET 108
#define FILEUID_LEN 8

#define FILEGID_OFFSET 116
#define FILEGID_LEN 8

#define FILESIZE_OFFSET 124
#define FILESIZE_LEN 12

#define FILEMTIME_OFFSET 136
#define FILEMTIME_LEN 12

#define CHKSUM_OFFSET 148
#define CHKSUM_LEN 8

#define FILETYPFLAG_OFFSET 156
#define FILETYPFLAG_LEN 1

#define LINKNAME_OFFSET 157
#define LINKNAME_LEN 100

#define MAGIC_OFFSET 257
#define MAGIC_LEN 6

#define VERSION_OFFSET 263
#define VERSION_LEN 2

#define FILEUNAME_OFFSET 265
#define FILEUNAME_LEN 32

#define FILEGNAME_OFFSET 297
#define FILEGNAME_LEN 32



#endif //TARCUSTAR_H__
