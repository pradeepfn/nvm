#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include "checkpoint.h"

#define FILE_PATH "foo"

struct Checkpoint{
	bool is_valid;
    int value;
};

// variables
void *file_memory;
int fd;

/*int main(int argc, char *argv[]){
	//printf("checkpoint test main\n"); 
    map_memory_file();
	checkpoint(14);
	int saved = rtrv_last_state();
    //printf("saved value : %d\n", saved);
	return 0;
}*/

void map_memory_file(){
    // create a file
    fd = open (FILE_PATH, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    //dummy write to file makes it big enough
    lseek (fd, sizeof(struct Checkpoint) *10 + 1, SEEK_SET);
    write (fd, "", 1); 
    lseek (fd, 0, SEEK_SET);
   
    file_memory = mmap (0, sizeof(struct Checkpoint)*10, PROT_WRITE, MAP_SHARED, fd, 0);
    close (fd);
}

bool is_chkpoint_present(){
    struct Checkpoint *chk = (struct Checkpoint *)file_memory;
	return chk->is_valid;
}

int rtrv_last_state(){
    struct Checkpoint *chk = (struct Checkpoint *)file_memory;
	int temp = chk->value;
	return temp;
}

/*
    Checkpointing in to the file systesm.
*/
void checkpoint(int i){ 
    struct Checkpoint checkpoint = {.is_valid=false,.value=i};
    //printf("Structure values, is_valid : %d , value : %d \n",checkpoint.is_valid, checkpoint.value);  
    // doing the memory copy from DRAM to FILE	
	memcpy((struct Checkpoint *)file_memory,&checkpoint,sizeof(struct Checkpoint));
	((struct Checkpoint *)file_memory)->is_valid = true; // done with checkpoint
}        







