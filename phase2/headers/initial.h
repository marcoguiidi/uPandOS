#ifndef INITIAL_H_INCLUDED
#define INITIAL_H_INCLUDED

#include "/usr/include/umps3/umps/types.h"
#include "../../headers/listx.h"
#include "../../headers/types.h"
#include "../../headers/const.h"

/*
Declare the Level 3 global variables (1.1)
*/

extern int process_count;
extern int soft_block_count;
extern struct list_head ready_queue;
extern pcb_t* current_process;

#define PSEUDO_CLOCK_BLOCKED SEMDEVLEN - 1
extern struct list_head blocked_pcbs[SEMDEVLEN]; // last one is for the Pseudo-clock


extern passupvector_t* passupvector;

extern pcb_t* ssi_pcb;

/** last used pid */
extern unsigned int lastpid;

/**
return a not used pid
*/
unsigned int new_pid();
void process_spawn(pcb_t* process);
void process_kill(pcb_t*  process);

#endif