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
void *int_memory;
void *struct_memory;
int fd;
int count;

/*int main(int argc, char *argv[]){
	printf("checkpoint test main\n"); 
    printf("counter value : %d\n", count);
    map_memory_file();
	checkpoint(28);
    printf("counter value : %d\n", count);
	checkpoint(40);
    printf("counter value : %d\n", count);
	int saved = rtrv_last_state();
    printf("counter value : %d\n", count);
    printf("saved value : %d\n", saved);
	return 0;
}*/

void init(){
    count = -1;
}

void map_memory_file(){
    // create a file
    fd = open (FILE_PATH, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    //dummy write to file makes it big enough
    lseek (fd, sizeof(int) + sizeof(struct Checkpoint) *10 + 1, SEEK_SET);
    write (fd, "", 1); 
    lseek (fd, 0, SEEK_SET);
    file_memory = mmap (0,sizeof(int) + sizeof(struct Checkpoint)*10, PROT_WRITE, MAP_SHARED, fd, 0);
	int_memory = file_memory;
	struct_memory =((int *)file_memory)+1;
	//count = *((int *)int_memory);
    close (fd);
}

bool is_chkpoint_present(){
   // struct Checkpoint *chk = (struct Checkpoint *)file_memory;
   //	return chk->is_valid;
   FILE * file = fopen("foo", "r");
   if (file)
    {
        fclose(file);
        return true;
    }
    return false;
}

int rtrv_last_state(){
	count = *((int *)int_memory); // restoring the counter
	printf("counter value : %d\n", count);
    int j;
	for(j=0;j<10;j++){
		struct Checkpoint *chk = ((struct Checkpoint *)struct_memory) + (count%10);
		if(chk->is_valid){
			return chk->value;
		}
		count--;
	}
	return -1;
}

/*
    Checkpointing in to the file systesm.
*/
void checkpoint(int i){ 
    struct Checkpoint checkpoint = {.is_valid=false,.value=i};
	memcpy(((struct Checkpoint *)struct_memory)+((count+1)%10),&checkpoint,sizeof(struct Checkpoint));
	(((struct Checkpoint *)struct_memory) + ((count+1)%10))->is_valid = true; // done with checkpoint
	count++;
	memcpy((int *) int_memory, &count, sizeof(int)); // update the counter 
}        







