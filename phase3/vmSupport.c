/**
vmSupport.c: This module implements:
- the TLB exception handler (The Pager). TODO:
Since reading and writing to each U-proc’s flash device is limited to supporting paging, 
- this module should also contain the function(s) for reading and writing flash devices. TODO: 
Additionally, the Swap Pool table is local to this module. Instead of declaring them globally in initProc.c they can be
declared module-wide in vmSupport.c. The test function will now invoke a new “public” function
- initSwapStructs which will do the work of initializing the Swap Pool table. [x]
Technical Point: Since the code for the TLB-Refill event handler was replaced (without
relocating the function), uTLB_RefillHandler should still be found in the Level 3/Phase 2
exceptions.c file.
*/

#include "../headers/types.h"
#include "../headers/const.h"
#include "./headers/vmSupport.h"
#include "./headers/misc.h"
#include "../phase2/headers/exceptions.h"
#include "headers/initProc.h"
#include <umps3/umps/types.h>

swap_t swap_pool[POOLSIZE];

void initSwapStruct(void) {
    for (int i = 0; i < POOLSIZE; i++) {
        swap_pool[i].sw_asid = NOPROC;
    }
}

void pager(void) {
    // 1 get support data
    support_t* support_data = get_support_data();

    // 2 determine the cause of the TLB exception
    state_t* exceptstate = &(support_data->sup_exceptState[0]);

    // 3 
    if (exceptstate->cause == TLBMOD) TrapExceptionHandler(exceptstate);

    // 4
    gainSwap();
}