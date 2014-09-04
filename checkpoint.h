#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <stddef.h>
typedef enum { false, true } bool;
// meta data structure used during data persistance
// id + version defines a unique structure.
typedef struct checkpoint{
	int id;
	int version;
	bool is_valid;
	size_t data_size;
	size_t prv_offset;
	size_t offset;
}checkpoint_t;


bool is_chkpoint_present();
void map_memory_file();
void init();
checkpoint_t *get_meta(void *base_addr,size_t offset);
checkpoint_t *get_latest_version(int id);
checkpoint_t *get_latest_version1(void *base_addr, checkpoint_t *head, int id);
void checkpoint(int id, int version, size_t size, void *data);
void checkpoint2(void *base_addr, size_t last_meta_offset, int id, int version, size_t size, void *data);
void checkpoint1(void *start_addr, checkpoint_t *chkpt, void *data);
void *get_start_addr(void *base_addr,checkpoint_t *last_meta);
void *get_data_addr(void *base_addr, checkpoint_t *chkptr);
void *get_addr(void *base_addr, size_t offset);
int get_new_offset(int offset, size_t data_size);
void print_data(checkpoint_t *chkptr);

#endif
