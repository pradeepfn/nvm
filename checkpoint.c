#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <unistd.h>
#include "checkpoint.h"


#define FILE_PATH "foo"
#define FILE_SIZE 500000000

// variables
void *file_memory;
void *int_memory;
void *meta_memory;
int fd;
int offset;


LIST_HEAD(listhead, entry) head=
	LIST_HEAD_INITIALIZER(head);
struct listhead *headp;                 
struct entry {
    void *ptr;
	size_t size;
	int id;
	int version;
    LIST_ENTRY(entry) entries;
};
	


/*
create the initial memory mapped file structure for the first time
and initialize the meta structure.
*/
void init(){
	LIST_INIT(&head);
    // create a file
    fd = open (FILE_PATH, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    //dummy write to file makes it big enough
    lseek (fd, FILE_SIZE, SEEK_SET);
    write (fd, "", 1); 
    lseek (fd, 0, SEEK_SET);
    file_memory = mmap (0,FILE_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	int_memory = file_memory;
	meta_memory =((int *)file_memory)+1;
	if(!is_chkpoint_present()){
		//initialize the memory
		printf("first run of the program.... no prior checkpointed data!");
		offset = -1;
		memcpy(int_memory,&offset,sizeof(int));
		FILE *file = fopen("nvm.lck","w+");
		fclose(file);
	}else{
		offset =  *((int*)int_memory); 
	}
    close (fd);
}
int initialized = 0;
void *alloc(size_t size, char *var, int id, size_t commit_size){
	//init calls happens once
	if(!initialized){	
		init();
		initialized = 1;
	}
		struct entry *n = malloc(sizeof(struct entry)); // new node in list
		n->ptr = malloc(size); // allocating memory for incoming request
		n->size = size;
		n->id = id;
		n->version = 0;
		LIST_INSERT_HEAD(&head, n, entries);
		return n->ptr;

}

/* check whether our checkpoint init flag file is present */
int is_chkpoint_present(){
   // struct Checkpoint *chk = (struct Checkpoint *)file_memory;
   //	return chk->is_valid;
   FILE * file = fopen("nvm.lck", "r");
   if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}


extern checkpoint_t *get_latest_version(int id){
	//first restore the global counter
	offset = *((int *)int_memory);
	checkpoint_t * current = get_meta(meta_memory,offset);
	return get_latest_version1(meta_memory, current,id);
}

checkpoint_t *get_latest_version1(void *base_addr, checkpoint_t *head, int id){
	checkpoint_t *current = head;
	int temp_offset = offset;
	while(offset >= 0){
		checkpoint_t *ptr = get_meta(base_addr,temp_offset);
		if((ptr->id) == id){
			return ptr;
		}
		temp_offset = ptr->prv_offset;
	}
	return NULL;//to-do, handle exception cases

}


extern void chkpt_all(){
	struct entry *np;
	for (np = head.lh_first; np != NULL; np = np->entries.le_next){
		checkpoint(np->id, np->version,	np->size,np->ptr);
	}	
	return;
}


extern void checkpoint(int id, int version, size_t size, void *data){
	checkpoint2(meta_memory,offset,id,version,size,data);
	return;	
}

void checkpoint2(void *base_addr, size_t last_meta_offset, int id, int version, size_t size, void *data){
	checkpoint_t chkpt;
	void *start_addr;
	chkpt.id = id;
	chkpt.version = version;
	chkpt.data_size = size;
	if(offset != -1 ){
		checkpoint_t *last_meta = get_meta(base_addr, last_meta_offset);
		start_addr = get_start_addr(base_addr, last_meta);
		chkpt.prv_offset = last_meta_offset;
		chkpt.offset = last_meta->offset + sizeof(checkpoint_t)+last_meta->data_size;
	}else{
		start_addr = meta_memory;
		chkpt.prv_offset = -1;
		chkpt.offset = 0;
	}
	print_data(&chkpt);
	checkpoint1(start_addr, &chkpt,data);
	return;
}


void checkpoint1(void *start_addr, checkpoint_t *chkpt, void *data){ 
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
	size_t tot_offset = last_meta->offset + sizeof(checkpoint_t) + last_meta->data_size;
	char *next_start_addr = (char *)base_addr+tot_offset;
	return (void *)next_start_addr;
}

checkpoint_t *get_meta(void *base_addr,size_t offset){
	checkpoint_t *ptr = (checkpoint_t *)(((char *)base_addr) + offset);
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
	int temp = offset + sizeof(checkpoint_t) + data_size; 
	return temp; 
}

void print_data(checkpoint_t *chkptr){
	printf("id : %d\n", chkptr->id);
	printf("version : %d\n", chkptr->version);
	printf("is_valid : %d\n", chkptr->is_valid);
	printf("data_size : %zd\n", chkptr->data_size);
	printf("prev_offset : %zd\n", chkptr->prv_offset);
	printf("offset : %zd\n", chkptr->offset);
	return;
}
//test main
int main(int argc, char *argv[]){
	printf("Checkpoint testing...\n");
	char *var_name = "my_var";
	char *my_var = alloc(100,var_name, 1, 100);
	my_var = "Hello world checkpoint";
	char *var_name2 = "my_var2";
	char *my_var2 = alloc(50,var_name, 2, 50);
	my_var = "the Alternate content";
	chkpt_all();	
	checkpoint_t *meta = get_latest_version(2);
	print_data(meta);
	return 0;
}


