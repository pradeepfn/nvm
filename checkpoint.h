#ifndef CHEcKPOINT
#define CHECKPOINT
typedef enum { false, true } bool;

bool is_chkpoint_present();
int rtrv_last_state();
void checkpoint(int i );
void map_memory_file();

#endif
