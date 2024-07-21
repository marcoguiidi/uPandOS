/**
initProc.c: This module implements test and exports the Support Level’s global variables (e.g.
swap mutex PCB [Section 10] and optionally the device PCBs).
*/

#include "../headers/types.h"
#include "../headers/const.h"
#include "./headers/initProc.h"
#include "./headers/misc.h"
#include "headers/vmSupport.h"
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>

pcb_PTR swap_mutex;

// gain mutual exclusion over the swap pool
void gainSwap(void) {
    SYSCALL(SENDMSG, (unsigned int)swap_mutex, 0, 0);   // gainSwap[0]
    SYSCALL(RECEIVEMSG, (unsigned int)swap_mutex, 0, 0);// gainSwap[1]
}

// release mutual exclusion over the swap pool
void releaseSwap(void) {
    SYSCALL(SENDMSG, (unsigned int)swap_mutex, 0, 0);
}

#define QPAGE 1024
#define IEPBITON 0x4
#define CAUSEINTMASK 0xFD00

void swap_mutex_function(void) {
    void* payload;
    pcb_t* sender;
    while (TRUE) {
        // wait for a service request
        sender = (pcb_t*)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, 0, 0); // gainSwap[0]
        SYSCALL(SENDMSG, (unsigned int)sender, 0, 0);               // gainSwap[1], give the swap a sender

        (pcb_t*)SYSCALL(RECEIVEMESSAGE, sender, 0, 0); // now wait for sender to relseaseSwap
    }
}

void test(void) {

    // start the swap_mutex process
    state_t swap_mutex_state;
    STST(&swap_mutex_state);
    swap_mutex_state.reg_sp = swap_mutex_state.reg_sp - QPAGE; // ? QPAGE
    swap_mutex_state.pc_epc = (memaddr)swap_mutex_function;
    swap_mutex_state.status |= IEPBITON | CAUSEINTMASK | TEBITON;
    
    swap_mutex = create_process(&swap_mutex_state);

    // init swap struct
    initSwapStruct();
}