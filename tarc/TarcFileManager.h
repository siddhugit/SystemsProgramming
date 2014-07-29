#ifndef TARCFILEMANAGER_H__
#define TARCFILEMANAGER_H__

void setTarFileDescriptor(int tarfd);
void writeHeaderBuffToFile(char* headerBuff);
void writeDataBuffToFile(unsigned char* dataBuff,int bytesToWrite);
void writeDataFinished(void);
void writeArchiveFinished(void);

#endif //TARCFILEMANAGER_H__
