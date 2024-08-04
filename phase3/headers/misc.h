#ifndef MISC_3_H
#define MISC_3_H

#include <umps3/umps/types.h>
#include "../../headers/types.h"
/*
ssi serveces wrappers
*/

support_t* get_support_data(void);

pcb_t *create_process(state_t* state, support_t* support);

#define SELF (pcb_t*)NULL
pcb_t* kill_process(pcb_t* process);

#endif