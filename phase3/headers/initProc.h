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

void uproc_init(int asid);

void gainSwap(void);

void releaseSwap(void);

void test(void);

#endif