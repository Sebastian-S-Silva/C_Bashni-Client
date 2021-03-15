#include"shMem.h"
#include"shMemStrct.h"

int crShm(int size){
	int ID = shmget(IPC_PRIVATE, size, 0666 | IPC_CREAT);
    
    if(ID < 0){
    	printf("Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    }
    
    printf("Succesfully allocated shared memory.\n");
    return ID;
}

void* atShm (int ID){
	void *ptr = shmat(ID, NULL, 0);
    if(ptr == (void *) -1){
        printf("Failed to bind shared memory\n");
   	}else{
    	printf("Succesfully bound shared memory\n");
    }
    
    return ptr;
}