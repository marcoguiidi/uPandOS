#ifndef VMSUPPORT_H
#define VMSUPPORT_H

#include "../../headers/types.h"
#include "../../headers/const.h"

/*
The Swap Pool data structure/table. The Support Level will maintain a table, one entry per
Swap Pool frame, recording information about the logical page occupying it. At a minimum,
each entry should record the ASID and logical page number of the occupying page.
*/
extern swap_t swap_pool_table[POOLSIZE];

/**
initialize the swap_pool_table
 */
void initSwapStruct(void);

/**
page fault trap handler
While TLB-Refill events will be handled by the Support Level’s TLB-Refill event handler (e.g.
uTLB_RefillHandler), page faults are passed up by the Nucleus to the Support Level’s TLB exception
handler – the Pager.
 */
void pager(void);

/**
free occupied frames after an uproc dies
 */
void free_occupied_frames(unsigned int asid);

#endif