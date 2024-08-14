#ifndef INITPROC_H
#define INITPROC_H

#include "../../headers/types.h"
#include "../../headers/const.h"
#include <umps3/umps/types.h>

extern pcb_PTR swap_mutex;
extern pcb_PTR    sst_pcb[8];
extern pcb_PTR uproc_pbc[8];

extern state_t   state_t_pool[8];
extern support_t support_t_pool[8];

/**
gain acces to the swap pool table
 */
void gainSwap(void);

/**
release acces to the swap pool table
 */
void releaseSwap(void);

/**
test phase3 code by running multiple u-procs
 */
void test(void);

#endif