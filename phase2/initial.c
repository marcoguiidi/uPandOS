/*
initial.c This module implements main() and exports the Nucleus’s global variables (e.g.
process count, soft blocked count, blocked PCBs lists/pointers, etc.)
*/

#include "./headers/exceptions.h"
#include "./headers/initial.h"
#include "./headers/interrupts.h"
#include "./headers/scheduler.h"
#include "./headers/ssi.h"


#include "../phase1/headers/msg.h"
#include "../phase1/headers/pcb.h"

int main(void) {
    /*
    uTLB_RefillHandler function definition is in exceptions.h
    exceptionHandler function definition is in exceptions.h
    */
    /*
    init passupvector (1.2)
    typedef struct passupvector {
        unsigned int tlb_refill_handler;
        unsigned int tlb_refill_stackPtr;
        unsigned int exception_handler;
        unsigned int exception_stackPtr;
    } passupvector_t;
    */
    passupvector->tlb_refill_handler    = (memaddr)uTLB_RefillHandler; //init passupvector (1.2)
    passupvector->tlb_refill_stackPtr   = KERNELSTACK;
    passupvector->exception_handler     = (memaddr)exceptionHandler;
    passupvector->exception_stackPtr    = KERNELSTACK; 

    /*
    init Level 2 data structures (1.3)
    */
    initPcbs();
    initMsgs();

    /*
    init (1.1) global variables (1.4)
    */
    process_count = 0;
    soft_block_count = 0;
    INIT_LIST_HEAD(&ready_queue);
    current_process = NULL;
    for (int i = 0; i < SEMDEVLEN; i++) {
        INIT_LIST_HEAD(&blocked_pcbs[i]);
    }

    return 0;
}