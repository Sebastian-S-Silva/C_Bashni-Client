#ifndef sharedmem_H_
#define sharedmem_H_

#include"shMemStrct.h"

int clSock();
int connectCl(confVars* ptr, int shMId, shareS* shMemP, int sockC);
int gameRoutine(int* fd, int shMId, shareS* shMemP, int sockC);

#endif