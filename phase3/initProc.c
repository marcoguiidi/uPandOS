/**
initProc.c: This module implements test and exports the Support Level’s global variables (e.g.
swap mutex PCB [Section 10] and optionally the device PCBs).
*/

#include "../headers/types.h"
#include "../headers/const.h"
#include "./headers/initProc.h"
#include "./headers/sst.h"
#include "./headers/misc.h"
#include "headers/sysSupport.h"
#include "headers/vmSupport.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>
#include "../klog.h"

// swap mutex maybe not accessible

// gain mutual exclusion over the swap pool
void gainSwap(void) {
    SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex, 0, 0);   // gainSwap[0]
    SYSCALL(RECEIVEMESSAGE, (unsigned int)swap_mutex, 0, 0);// gainSwap[1]
}

// release mutual exclusion over the swap pool
void releaseSwap(void) {
    SYSCALL(SENDMESSAGE, (unsigned int)swap_mutex, 0, 0);
}

#define QPAGE 1024
#define IEPBITON 0x4
#define CAUSEINTMASK 0xFD00

void swap_mutex_function(void) {
    pcb_t* sender;
    while (TRUE) {
        // wait for a service request
        sender = (pcb_t*)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0); // gainSwap[0]
        SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);               // gainSwap[1], give the swap a sender

        SYSCALL(RECEIVEMESSAGE, (unsigned int)sender, 0, 0); // now wait for sender to relseaseSwap
    }
}

state_t   state_t_pool[8];
state_t   state_t_sst_pool[8];
support_t support_t_pool[8];

pcb_PTR swap_mutex;
pcb_PTR sst_pcb[8];
pcb_PTR uproc_pbc[8];

state_t state_t_swap_mutex;

void uproc_init(int asid) {
    // initializing u-proc
    // initial processor state for u-proc
    state_t* state = &state_t_pool[asid];
    STST(state);
    // pc and s_t9 set to  0x800000B0
    state->pc_epc = 0x800000B0;
    state->reg_t9 = 0x800000B0;
    // sp set to 0xC0000000
    state->reg_sp = 0xC0000000;
    // status set for user mode with all interrupts and the plt enabled
    state->status = 0b10 | 0b1 | IMON | TEBITON;
    //         user mode| global interrupt enable bit
    // enstryhi.asid to the process's unique id
    state->entry_hi = asid << ASIDSHIFT;

    //ititialization of a support structure
    support_t* support = &support_t_pool[asid];
    // setup sup_asid to the process asid
    support->sup_asid = asid;

    // initialize u-proc page table
    for (int i = 0; i < 31; i++) {
        // VPN(31-12) ASID(11-6) empty(5-0)
        support->sup_privatePgTbl[i].pte_entryHI = ((0x80000 + 0x1*i) << VPNSHIFT) | (asid << ASIDSHIFT);
        // D on G off (aready set)  V off (already set)
        support->sup_privatePgTbl[i].pte_entryLO = DIRTYON;
    }
    // VPN(31-12) ASID(11-6) empty(5-0)
    support->sup_privatePgTbl[31].pte_entryHI = (0xBFFFF << VPNSHIFT) | (asid << ASIDSHIFT);
    // D on G off (aready set)  V off (already set)
    support->sup_privatePgTbl[31].pte_entryLO = DIRTYON;

    // setup sup_exceptContext[2]
    //Set the two PC fields. One of them (0 - PGFAULTEXCEPT) should be set to the address of the
    //Support Level’s TLB handler, while the other one (1 - GENERALEXCEPT) should be set to the
    //address of the Support Level’s general exception handler.
    support->sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)pager;
    support->sup_exceptContext[GENERALEXCEPT].pc = (memaddr)support_general_exception_handler;
    //Set the two Status registers to: kernel-mode with all interrupts and the Processor Local Timer enabled.
    support->sup_exceptContext[0].status = 0b00 | 0b1 | IMON | TEBITON;
    //                               kernel mode| global interrupt enable bit
    support->sup_exceptContext[1].status = 0b00 | 0b1 | IMON | TEBITON;
    //                               kernel mode| global interrupt enable bit
    //Set the two SP fields to utilize the two stack spaces allocated in the Support Structure. Stacks
    //grow “down” so set the SP fields to the address of the end of these areas.
    //E.g. ... = &(...sup_stackGen[499]).
    
    //Allocate per-U-proc TLB, and general exception handler stacks directly from RAM [Section
    //10.1].
    //Directly allocate the two stack spaces per U-proc (one for the Support Level’s TLB exception
    //handler, and one for the Support Level’s general exception handler) from RAM, instead of as
    //fields in the Support Structure. The recommended RAM space to be used are the frames directly
    //below RAMTOP, avoid the actual last frame of RAM (stack page for test).
    //Important: SP values are always the end of the area, not the start. Hence, to use the penul-
    //timate RAM frame as a U-proc’s stack space for one of its Support Level handlers, one would
    //assign the SP value to RAMTOP-PAGESIZE.
    unsigned int ramtop;
    RAMTOP(ramtop);
    support->sup_exceptContext[0].stackPtr = ramtop - ((1+ asid)*PAGESIZE);
    support->sup_exceptContext[1].stackPtr = ramtop - ((1+ asid)*PAGESIZE);
}

/*
sp for swap mutex is one page next kernel (kerenelstack - QPAGE )
so
sp for sst_0 is (swap_mutex_stack - QPAGE) = kernelstack - 2*QPAGE
sp for sst_n = kernelstack - (2+n)*QPAGE
*/

void sst_state_init(void) {
    for (int asid = 0; asid < UPROCMAX; asid++) {
        state_t* state = &state_t_sst_pool[asid];
        STST(state);
        // pc and s_t9 set to  SST_function_entry_point
        state->pc_epc = (memaddr)SST_function_entry_point;
        state->reg_t9 = (memaddr)SST_function_entry_point;
        // sp set to kernelstack - (2+n)*QPAGE
        state->reg_sp = KERNELSTACK - (2+asid)*QPAGE;
        // status set for kernel mode with all interrupts and the plt enabled
        state->status = 0b00 | 0b1 | IMON | TEBITON;
        //         kernel mode| global interrupt enable bit
        // enstryhi.asid to the process's unique id
        state->entry_hi = asid << ASIDSHIFT;
    }
}

// run n test max UPROCMAX
#define TESTRUN 2

void test(void) {

    // start the swap_mutex process
    state_t* swap_mutex_state = &state_t_swap_mutex;
    STST(swap_mutex_state);
    // pc and s_t9 set to  swap_mutex_function
    swap_mutex_state->pc_epc = (memaddr)swap_mutex_function;
    swap_mutex_state->reg_t9 = (memaddr)swap_mutex_function;
    // sp set to kernelstack - QPAGE
    swap_mutex_state->reg_sp = KERNELSTACK - QPAGE;
    // status set for kernel mode with all interrupts and the plt enabled
    swap_mutex_state->status = 0b00 | 0b1 | IMON | TEBITON;
    
    // without support struct
    swap_mutex = create_process(swap_mutex_state, NULL);

    // init swap struct
    initSwapStruct();

    // init u-proc support and state
    for (int asid = 0; asid < UPROCMAX; asid++) {
        uproc_init(asid);
    }

    sst_state_init();
    // launch 8 sst with corrisponding u-procs
    for (int asid = 0; asid < TESTRUN; asid++) {
        // setup sst state
        
        // support is same as u-proc
        sst_pcb[asid] = create_process(&state_t_sst_pool[asid], &support_t_pool[asid]);
    }
    //sst_pcb[TESTRUN - 1] = create_process(&state_t_sst_pool[TESTRUN - 1], &support_t_pool[TESTRUN - 1]);

    // wait for termination of all SST
    void* payload;
    for (int i = 0; i < TESTRUN; i++) {
        SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);
        klog_print(" [sst terminated] ");
    }
    //SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);
    klog_print(" [test terminated] ");
    // work is finished
    kill_process(SELF);
}