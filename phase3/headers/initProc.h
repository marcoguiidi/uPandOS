#ifndef INITPROC_H
#define INITPROC_H

#include "../../headers/types.h"
#include "../../headers/const.h"

extern pcb_PTR swap_mutex;
extern pcb_PTR test_pcb;

extern state_t   state_t_pool[8];
extern support_t support_t_pool[8];

void uproc_init(int asid);

void gainSwap(void);

void releaseSwap(void);

void test(void);

#endif