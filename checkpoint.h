#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <stddef.h>
#include <sys/time.h>
typedef enum { false, true } bool;
// meta data structure used during data persistance
// id + version defines a unique structure.
typedef struct checkpoint{
	int id;
	int process_id;
	int version;
	bool is_valid;
	size_t data_size;
	size_t prv_offset;
	size_t offset;
}checkpoint_t;

typedef struct headmeta{
	int offset;
	int id;
	struct timeval timestamp;
}headmeta_t;

typedef struct memmap{
	void *file;
	headmeta_t *head;
	checkpoint_t *meta;
	int fd;
}memmap_t;


int is_chkpoint_present();
void map_memory_file();
void init();
checkpoint_t *get_meta(void *base_addr,size_t offset);
checkpoint_t *get_latest_version(int id, int process_id);
checkpoint_t *get_latest_version1(memmap_t *mmap, int id, int process_id);
void checkpoint(int id,int process_id, int version, size_t size, void *data);
void checkpoint2(void *base_addr, int id, int process_id, int version, size_t size, void *data);
void checkpoint1(void *start_addr, checkpoint_t *chkpt, void *data);
void *get_start_addr(void *base_addr,checkpoint_t *last_meta);
void *get_data_addr(void *base_addr, checkpoint_t *chkptr);
void *get_addr(void *base_addr, size_t offset);
int get_new_offset(int offset, size_t data_size);
void mmap_files(memmap_t *m, const char *file_name);
void copy_head_to_mem(memmap_t *m, int fileId);
memmap_t *get_latest_mapfile(memmap_t *m1,memmap_t *m2);
#endif
