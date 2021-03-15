#ifndef shMem_H_
#define shMem_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

int crShm(int size);
void* atShm(int ID);

#endif