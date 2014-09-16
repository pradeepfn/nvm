#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <unistd.h>
#include <assert.h>
#include "checkpoint.h"
#include "utils.h"


#define FILE_PATH_ONE "mmap.file.one"
#define FILE_PATH_TWO "mmap.file.two"
#define FILE_SIZE 300
/*#define FILE_SIZE 500000000*/

memmap_t m[2];
memmap_t *current;
int offset;
LIST_HEAD(listhead, entry) head=
	LIST_HEAD_INITIALIZER(head);
struct listhead *headp;                 
struct entry {
    void *ptr;
	size_t size;
	int id;
	int process_id;
	int version;
    LIST_ENTRY(entry) entries;
};
/*
create the initial memory mapped file structure for the first time
and initialize the meta structure.
*/
void init(){
	LIST_INIT(&head);
	//initializing two mem map files and structures pointing to them
	char file1[] = FILE_PATH_ONE;
	char file2[] = FILE_PATH_TWO;
	mmap_files(&m[0],file1);
	mmap_files(&m[1],file2);
	if(!is_chkpoint_present()){
		printf("first run of the program.... no prior checkpointed data!\n");
		//initialize the head meta structure of the mem map
		copy_head_to_mem(&m[1],1);
		copy_head_to_mem(&m[0],0);
		current = &m[0]; // head meata directly operate on map file memory
		printf("current map file is : %d \n", current->head->id);
		FILE *file = fopen("nvm.lck","w+");
		fclose(file);
	}else{
		// after a restart we find latest map file to operate on
		current = get_latest_mapfile(&m[0],&m[1]);	 
		printf("current map file is : %d \n", current->head->id);
	}
}

void mmap_files(memmap_t *m, const char *file_name){
    m->fd = open (file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    lseek (m->fd, FILE_SIZE, SEEK_SET);
    write (m->fd, "", 1); 
    lseek (m->fd, 0, SEEK_SET);
    m->file = mmap (0,FILE_SIZE, PROT_WRITE, MAP_SHARED, m->fd, 0);
	m->head = m->file;
	m->meta =(checkpoint_t *)((headmeta_t *)m->file)+1;
    close (m->fd);
}
//copy the init head metadata portion to memory map
void copy_head_to_mem(memmap_t *m, int fileId){
	headmeta_t head;
	head.id = fileId;
	head.offset = -1;
	gettimeofday(&(head.timestamp),NULL);
	memcpy(m->head,&head,sizeof(headmeta_t));
}

memmap_t *get_latest_mapfile(memmap_t *m1,memmap_t *m2){
	//first check the time stamps of the head values
	headmeta_t *h1 = m1->head;
	headmeta_t *h2 = m2->head;
	struct timeval result;
	//Return 1 if the difference is negative, otherwise 0.
	if(!timeval_subtract(&result, &(h1->timestamp),&(h2->timestamp)) && (h1->offset !=-1)){ 
		return m1;	
	}else if(h2->offset != -1){
		return m2;
	}else{
		printf("Wrong program execution path...");
		assert(0);
	}
}

int initialized = 0;
void *alloc(size_t size, char *var, int id, int process_id, size_t commit_size){
	//init calls happens once
	if(!initialized){	
		init();
		initialized = 1;
	}
		struct entry *n = malloc(sizeof(struct entry)); // new node in list
		n->ptr = malloc(size); // allocating memory for incoming request
		n->size = size;
		n->id = id;
		n->process_id = process_id;
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
	checkpoint_t *result;
	memmap_t *other;
	if((result = get_latest_version1(current, id)) == NULL){
		//if result not found in the current mem map file, then switch the files
		// and do search again
		printf("Not found in the current memory mapped file. Searching the other...\n");
		other = (current == &m[0])?&m[1]:&m[0];	
		result = get_latest_version1(other, id);	
	}
	return result;	
}

checkpoint_t *get_latest_version1(memmap_t *mmap, int id){
	int temp_offset = mmap->head->offset;
	while(temp_offset >= 0){
		checkpoint_t *ptr = get_meta(mmap->meta,temp_offset);
		if((ptr->id) == id){
			return ptr;
		}
		temp_offset = ptr->prv_offset;
	}
	return NULL;//to-do, handle exception cases
}

int is_remaining_space_enough(){
	size_t tot_size=0;
	struct entry *np;
	for (np = head.lh_first; np != NULL; np = np->entries.le_next){
		tot_size+=(sizeof(checkpoint_t)+np->size);	
	}	
	size_t remain_size = FILE_SIZE - (sizeof(headmeta_t) + current->head->offset);
	return (remain_size > tot_size); 
}


extern void chkpt_all(){
	if(!is_remaining_space_enough()){
		printf("remaining space is not enough. Switching to other file....\n");
		//swap the persistant memory if not enough space
		printf("current map file is : %d \n", current->head->id);
		current = (current == &m[0])?&m[1]:&m[0];	
		current->head->offset = -1; // invalidate the data
		gettimeofday(&(current->head->timestamp),NULL); // setting the timestamp
		printf("current map file after switch is : %d \n", current->head->id);
	}
	struct entry *np;
	for (np = head.lh_first; np != NULL; np = np->entries.le_next){
		checkpoint(np->id, np->process_id, np->version,	np->size,np->ptr);
	}	
	return;
}

/*
	try checkpointing to current mem map file if space is not enough
	switch to next mem map file and do checkpointing.
*/
extern void checkpoint(int id, int process_id, int version, size_t size, void *data){
	checkpoint2(current->meta,id, process_id, version,size,data);
	return;	
}

void checkpoint2(void *base_addr, int id, int process_id, int version, size_t size, void *data){
	checkpoint_t chkpt;
	void *start_addr;
	chkpt.id = id;
	chkpt.process_id = process_id;
	chkpt.version = version;
	chkpt.data_size = size;
	if(current->head->offset != -1 ){
		checkpoint_t *last_meta = get_meta(base_addr, current->head->offset);
		start_addr = get_start_addr(base_addr, last_meta);
		chkpt.prv_offset = current->head->offset;
		chkpt.offset = last_meta->offset + sizeof(checkpoint_t)+last_meta->data_size;
	}else{
		start_addr = current->meta;
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
	//directly operating on the mapped memory
	current->head->offset = chkpt->offset;
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
/*
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
	char *my_var = alloc(100,var_name, 3,1, 100);
	my_var = "Hello world checkpoint";
	char *var_name2 = "my_var2";
	char *my_var2 = alloc(50,var_name, 4,1, 50);
	my_var2 = "the Alternate content";
	chkpt_all();	
	checkpoint_t *meta = get_latest_version(2);
	if(meta){
		print_data(meta);
	}else{
		printf("data not found\n");
	}
	return 0;
}
*/


