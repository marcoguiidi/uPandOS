#ifndef INITPROC_H
#define INITPROC_H

#include "../../headers/types.h"
#include "../../headers/const.h"

extern pcb_PTR swap_mutex;
extern pcb_PTR test_pcb;

void gainSwap(void);

void releaseSwap(void);

void test(void);

#endif