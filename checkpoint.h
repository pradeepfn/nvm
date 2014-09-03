#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <stddef.h>
typedef enum { false, true } bool;
// meta data structure used during data persistance
// id + version defines a unique structure.
typedef struct checkpoint{
	int ch_id;
	int version;
	bool is_valid;
	size_t data_size;
	size_t prv_offset;
	size_t offset;
}checkpoint_t;


bool is_chkpoint_present();
int rtrv_last_state();
void checkpoint(void *start_address, checkpoint_t *chkpt, void *data);
void map_memory_file();
void init();

#endif
