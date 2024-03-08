#ifndef INITIAL_H_INCLUDED
#define INITIAL_H_INCLUDED

#include "/usr/include/umps3/umps/types.h"
#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../headers/const.h"

/*
Declare the Level 3 global variables (1.1)
*/

int process_count = 0;
int soft_block_count = 0;
struct list_head ready_queue;
pcb_t* current_process;
struct list_head blocked_pcbs[SEMDEVLEN]; // last one is for the Pseudo-clock


passupvector_t* passupvector = (memaddr) PASSUPVECTOR;

#endif