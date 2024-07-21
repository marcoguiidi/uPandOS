#ifndef VMSUPPORT_H
#define VMSUPPORT_H

#include "../../headers/types.h"
#include "../../headers/const.h"

extern swap_t swap_pool[POOLSIZE];

void initSwapStruct(void);

void pager(void);

#endif