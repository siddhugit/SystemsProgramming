#ifndef WTMPBUFFEREDACCESS_H_INCLUDED
#define WTMPBUFFEREDACCESS_H_INCLUDED

int utmp_open(char*);
int utmp_len();
struct utmp* utmp_getrec(int);
int utmp_close();

#endif // WTMPBUFFEREDACCESS_H_INCLUDED
