#ifndef SHAREDMEM
#define SHAREDMEM


#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


void* create_shared_memory_segment(size_t memsize);

//void write_to_shared_mem(void* data, void* shared_mem_address);

int lock_mem_segment(void* addr, size_t size);


int unlock_mem_segment(void* addr, size_t size);



#endif
