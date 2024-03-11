/*
initial.c This module implements main() and exports the Nucleus’s global variables (e.g.
process count, soft blocked count, blocked PCBs lists/pointers, etc.)
*/
#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"

#include "./headers/exceptions.h"
#include "./headers/initial.h"
#include "./headers/interrupts.h"
#include "./headers/scheduler.h"
#include "./headers/ssi.h"


#include "../phase1/headers/msg.h"
#include "../phase1/headers/pcb.h"

extern void test();

int process_count = 0;
int soft_block_count = 0;
struct list_head ready_queue;
pcb_t* current_process;
struct list_head blocked_pcbs[SEMDEVLEN]; // last one is for the Pseudo-clock


passupvector_t* passupvector = PASSUPVECTOR;

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
    mkEmptyProcQ(&ready_queue);
    current_process = NULL;
    for (int i = 0; i < SEMDEVLEN; i++) {
        INIT_LIST_HEAD(&blocked_pcbs[i]);
    }

    /*
    load the system-wide Interval Timer with 100 milliseconds (constant PSECOND) (1.5)
    */
    LDIT(PSECOND);

    /*
    instantiate a first process, place its PCB in the Ready Queue, and increment Process Count (1.6)
    */
    /*
    p_s is the processor state (state_t)
    typedef struct state {
	    unsigned int entry_hi;
	    unsigned int cause;
	    unsigned int status;
	    unsigned int pc_epc;
	    unsigned int gpr[STATE_GPR_LEN];
	    unsigned int hi;
	    unsigned int lo;
    } state_t;

    */
    pcb_t* first = allocPcb();
    /*
    p.25 of uMPS3princOfOperations.pdf 
    */
    first->p_s.status = (first->p_s.status | STATUS_IEp ) & (~STATUS_KUp);
    RAMTOP(first->p_s.reg_sp);
    first->p_s.pc_epc = (memaddr) SSI_function_entry_point;
    first->p_s.reg_t9 = (memaddr) SSI_function_entry_point;
    first->p_time = 0;
    first->p_supportStruct = NULL;

    insertProcQ(&ready_queue, first);
    process_count++;

    /*
    (1.7)
    */
    pcb_t* second = allocPcb();
    /*
    p.25 of uMPS3princOfOperations.pdf 
    */
    second->p_s.status = (first->p_s.status | STATUS_IEp | STATUS_TE) & (~STATUS_KUp);
    RAMTOP(second->p_s.reg_sp) - (2 * FRAMESIZE);
    second->p_s.pc_epc = (memaddr) test;
    second->p_s.reg_t9 = (memaddr) test;
    second->p_time = 0;
    second->p_supportStruct = NULL;

    insertProcQ(&ready_queue, second);
    process_count++;

    /*
    call the scheduler (1.8) 
    */
    scheduler(); //TODO:

    return 0;
}