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
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

swap_t swap_pool[POOLSIZE];

void initSwapStruct(void) {
    for (int i = 0; i < POOLSIZE; i++) {
        swap_pool[i].sw_asid = NOPROC;
    }
}

// 5.4
unsigned int pageReplacementAlgorithm(void) {
    static unsigned int index = 0; // static variable inside a function keeps its value between invocations
    
    index = (index + 1) % POOLSIZE;
    return index;
}

int isSwapPoolFrameOccupied(unsigned int framenum) {
    if (swap_pool[framenum].sw_asid != NOPROC) return TRUE;
    else return FALSE;
}

void pager(void) {
    unsigned int saved_status;

    // 1 get support data
    support_t* support_data = get_support_data();

    // 2 determine the cause of the TLB exception
    state_t* exceptstate = &(support_data->sup_exceptState[0]);

    // 3 
    if (exceptstate->cause == TLBMOD) TrapExceptionHandler(exceptstate);

    // 4
    gainSwap();

    // 5 Determine the missing page number (p)
    unsigned int missing_page_num = (exceptstate->entry_hi & GETPAGENO) >> VPNSHIFT;

    // 6 pick a frame i from the swap pool..
    unsigned int frame_victim_num = pageReplacementAlgorithm();

    // 7 determine if frame i is occupied
    if (isSwapPoolFrameOccupied(frame_victim_num)) {
        // 8
        // ATOMIC start
        saved_status = getSTATUS();
        setSTATUS(saved_status & ~IECON); // disable interrupts

        int belogns_to_ASID = swap_pool[frame_victim_num].sw_asid;

        // (a)
        swap_pool[frame_victim_num].sw_pte->pte_entryLO &= !VALIDON;

        // (b)
        TLBCLR(); // TODO: 5.2 update tlb when all debugged

        // (d)

        // TODO: contunua da qui

        setSTATUS(saved_status);
        // ATOMIC end
    }
}