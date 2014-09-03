#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include "checkpoint.h"


#define FILE_PATH "foo"
#define FILE_SIZE 10240

// variables
void *file_memory;
void *int_memory;
void *meta_memory;
int fd;
int offset;


void init(){
    offset = 0; // pointing to starting address;
}

/*
Sets up the memory map file fro subsequent write/read operations
*/
void checkpoint_init(){
    // create a file
    fd = open (FILE_PATH, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    //dummy write to file makes it big enough
    lseek (fd, FILE_SIZE, SEEK_SET);
    write (fd, "", 1); 
    lseek (fd, 0, SEEK_SET);
    file_memory = mmap (0,FILE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	int_memory = file_memory;
	meta_memory =((int *)file_memory)+1;
	//count = *((int *)int_memory);
    close (fd);
}


/* dump way to see whether checkpoint is present */
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


extern checkpoint_t *get_latest_version(int id){
	//first restore the global counter
	offset = *((int *)int_memory);
	checkpoint_t * current = get_meta(meta_memory,offset);
	return get_latest_version(meta_memory, current,id);
}

checkpoint_t *get_latest_version(void *base_addr, checkpoint_t *head, int id){
	checkpoint_t *current = head;
	while(!is_empty_checkpoint() & current->prv_offset != -1){
		checkpoint_t *ptr = get_meta(base_addr,current->offset);
		if((ptr->id) == id){
			return ptr;
		}
	}
	return NULL;//to-do, handle exception cases

}

extern void checkpoint(int id, int version, size_t size, void *data){
	checkpoint(meta_memory,offset,id,version,size,data);
	return;	
}

void checkpoint(void *base_addr, size_t last_meta_offset, int id, int version, size_t size, void *data){
	checkpoint_t *last_meta = get_meta(base_addr, last_meta_offset);
	void *start_addr = get_start_addr(base_addr, last_meta);
	checkpoint_t chkpt;
	chkpt.id = id;
	chkpt.version = version;
	chkpt.data_size = size;
	chkpt.prv_offset = last_meta_offset;
	chkpt.offset = last_meta->offset + sizeof(checkpoint_t)+last_met->data_size;
	checkpoint(start_addr, &chkpt,data);
	return;
}


void checkpoint(void *start_addr, checkpoint_t *chkpt, void *data){ 
	//copy the metadata 
	memcpy(start_addr,chkpt,sizeof(checkpoint_t));
	//copy the actual value after metadata.
	void *data_offset = ((char *)start_addr)+sizeof(checkpoint_t); 
	memcpy(data_offset,data,chkpt->data_size);
	offset = chkpt->offset;
	memcpy((int *) int_memory, &offset, sizeof(int));
	return;
}        

void *get_start_addr(void *base_addr,checkpoint_t *last_meta){
	size_t tot_offset = last_meta->offset + sizeof(checkpoint_t) + last-meta->data_size;
	char *next_start_addr = (char *)base_addr+tot_offset;
	return (void *)next_start_addr;
}

checkpoint_t *get_meta(void *base_addr,size_t offset){
	checkpoint *ptr = (checkpoint_t *)(((char *)baseaddr) + offset);
	return ptr; 
}

void *get_data_addr(void *base_addr, checkpoint_t *chkptr){
	char *temp = ((char *)base_addr) + chkptr->offset + sizeof(checkpoint_t);
	return (void *)temp;
}

void *get_addr(void *base_addr, size_t offset){
	char *temp = ((char *)base_addr)+offset;
	return temp;
}

int get_new_offset(int offset, size_t data_size){
	int temp = offset + sizeof(checkpoint_t) + size; 
	return temp; 
}

int main(int argc, char *argv[]){
	void *start_address = malloc(sizeof(checkpoint_t)*20);
	
	char ca[]="Hello world.From the copy buffer";
	checkpoint_t chk;
	chk.ch_id = 1;
	chk.data_size = sizeof(ca);
	chk.offset = 0;
	checkpoint(start_address,&chk,ca);

    int calc = chk.data_size + chk.offset + sizeof(checkpoint_t);

	char ca2[] = "second hello";
	checkpoint_t chk2;
	chk2.ch_id = 2;
	chk2.data_size = sizeof(ca2);
	chk2.offset = calc;
	char * offset_calc = (char *)start_address + calc;
	checkpoint(offset_calc,&chk2,ca2);

	checkpoint_t *chk_ptr;
	chk_ptr = (checkpoint_t *)start_address;
	printf("data size %zd \n",chk_ptr->data_size);	
	printf("offset %zd \n",chk.offset);	
	printf("offset %zd \n",chk_ptr->offset);	
	char *c = (char *)get_data_addr(start_address,&chk);
	printf("%s\n", c);
	return 0;
}


