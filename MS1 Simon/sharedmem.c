//#include "sharedmem.h"
#define _GNU_SOURCE
#include "bashniclient.h"


void* create_shared_memory_segment(size_t memsize){

    int prot = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANON;
    return mmap(NULL, memsize, prot, visibility, -1, 0);

} 

/*
void write_to_shared_mem(void* data, void* shared_mem_address){
    //lock memory
    int lock_ret;
    while (1){
        lock_ret = mlock(shared_mem_address, sizeof(data));
        if(lock_ret != 0){
            perror("could not lock shared mem: ");
            if(errno != EAGAIN){
                printf("quitting\n");
                exit(-1);
            }
        }
        else {
            printf("locked shared mem\n");
            break;
        }
    }
     

    
}*/

int lock_mem_segment(void* addr, size_t size){
    while(1){
        if (mlock(addr, size) == 0) return 0;
        else if(errno == EAGAIN){
            printf("."); 
            continue;
        }
        else {
            perror("could not aquire lock: ");
            return -1;
        
        }
    }
}



int unlock_mem_segment(void* addr, size_t size){
    while(1){
        if (munlock(addr, size) == 0) return 0;
        else if(errno == EAGAIN) continue;
        else {
            perror("could not unlock memory: ");
            return -1;
        
        }
    }
}
