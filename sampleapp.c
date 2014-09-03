#include <stdio.h>
#include <unistd.h>
#include "checkpoint.h"
#define LOOP_COUNT 100
#define SLEEP_DELAY 1  // sleep duration in seconds
#define FILE_PATH "foo";

int main(int argc, char *argv[]){
    printf("Hello World\n");
	int i;
    if(is_chkpoint_present()){
		printf("Chekpoint present\n");
		map_memory_file();
		// do checkpoint recover routines
		i = rtrv_last_state();
	}else{
        // start from the begining
		init();
		map_memory_file();
		printf("Starting from begining\n");
		i = 0; 
	}
	for(;i<LOOP_COUNT;i++){
		printf("loop value : %d\n", i);	
		if(i%10 == 0){
			char str[]= "Hello world";
			checkpoint(i,123,sizeof(str),str);
		}
		sleep(SLEEP_DELAY);
	}
    return 0; 
}
