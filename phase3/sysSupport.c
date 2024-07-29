/**
sysSupport.c: This module implements the Support Level’s:
- General exception handler [Section 7].
- SYSCALL exception handler [Section 8].
- Program Trap exception handler [Section 9].
*/

#include "./headers/sysSupport.h"
#include "./headers/misc.h"
#include "../phase2/headers/misc.h"
#include "../phase2/headers/interrupts.h"
#include "../phase2/headers/scheduler.h"
#include "headers/initProc.h"
#include <umps3/umps/types.h>
#include <umps3/umps/const.h>
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/cp0.h>
#include "../klog.h"

void support_syscall_exception_handler() {
    // get state register at time of syscall
    state_t* exceptionState = &current_process->p_supportStruct->sup_exceptState[GENERALEXCEPT];

    // syscall type
    int reg_A0 = exceptionState->reg_a0;
    // dest process
    unsigned int reg_A1 = exceptionState->reg_a1;
    // payload of the mex
    int reg_A2 = exceptionState->reg_a2;

    msg_t *msg;

    switch (reg_A0) {
    //perform a multi-way branching depending on the type of exception
        case SENDMSG: {
            
            pcb_t* dest_process = (pcb_t*)reg_A1;
            
            //  If a1 contains PARENT, then the requesting process send the message to its SST [Section 6], that is its parent.
            if (reg_A1 == PARENT) { 
                dest_process = current_process->p_parent;
                break;
            }
            if (isInPcbFree_h(dest_process->p_pid)) {
                //il dest process è nella lista pcbFree_h => non puo ricevere mex
                exceptionState->reg_v0 = DEST_NOT_EXIST;  
                break;   
            }
            // alloco messaggio
            msg = allocMsg();
            if (msg == NULL) {
                KLOG_PANIC("msg all used");
                exceptionState->reg_v0 = MSGNOGOOD; // messaggi liberi esauriti
                break;                
            }

            if (current_process == NULL) {
                KLOG_PANIC("current_process is NULL");
            }

            msg->m_payload = (unsigned int)reg_A2;
            msg->m_sender = current_process;
            
            if (outProcQ(&blocked_pcbs[BLOKEDRECV], dest_process) != NULL) { // dest_process is blocked
                // dest_process is waiting for a mex, unblock it
                insertProcQ(&ready_queue, dest_process);
                soft_block_count--;  // decrease the blocked processes counter, one is ready
            }
            // add new message to dest inbox
            insertMessage(&dest_process->msg_inbox, msg); 
            exceptionState->reg_v0 = 0; // success

            current_process->p_time -= get_elapsed_time_interupt(); 
            //updated by subtracting the time elapsed during the interrupt
            break;
        }
        case RECEIVEMSG: {
            
            pcb_t *sender_process = (pcb_PTR)reg_A1;
            //extract mex from inbox
            msg = popMessage(&current_process->msg_inbox, sender_process);
            
            // no message, wait
            if (msg == NULL) {
                // is running, put in waiting state
                copy_state_t(exceptionState, &(current_process->p_s));
                
                // add time spent in syscall
                current_process->p_time += get_elapsed_time();
                insertProcQ(&blocked_pcbs[BLOKEDRECV], current_process);
                soft_block_count++;

                
                // call the scheduler
                current_process = NULL;
                scheduler();
            }

            // return the payload
            exceptionState->reg_v0 = (unsigned int)msg->m_sender;

            if (reg_A2 != 0) {
                // save payload of the mex if there is one
                *((unsigned int*)reg_A2) = (unsigned int)msg->m_payload;
            }

            freeMsg(msg);
            break;
        }
        default:
            KLOG_PANIC("USYS code not found")
            break;
    }
    // reuturn from non bloking
    exceptionState->pc_epc += WORDLEN; //pc updated
    /*move the PC to the next instruction, avoiding entering an 
    infinite loop that would repeat the same syscall*/
    LDST(exceptionState);
    //restores processor state
}

void support_trap_exception_handler() {
    // send message to swap mutex
    msg_PTR msg = allocMsg();
    if (msg == NULL) {
        KLOG_PANIC("msg all used");
    }

    if (current_process == NULL) {
        KLOG_PANIC("current_process is NULL");
    }

    msg->m_payload = (unsigned int)0; // empty message
    msg->m_sender = current_process;
            
    if (outProcQ(&blocked_pcbs[BLOKEDRECV], swap_mutex) != NULL) { // dest_process is blocked
        // dest_process is waiting for a mex, unblock it
        insertProcQ(&ready_queue, swap_mutex);
        soft_block_count--;  // decrease the blocked processes counter, one is ready
    }
    // add new message to dest inbox
    insertMessage(&swap_mutex->msg_inbox, msg);

    // kill process
    process_killall(current_process);
}

void support_general_exception_handler() {
    state_t* state = &current_process->p_supportStruct->sup_exceptState[GENERALEXCEPT];
    unsigned int ExcCode = CAUSE_GET_EXCCODE(state->cause);

    if (ExcCode == SYSEXCEPTION) {
        support_syscall_exception_handler();
    } else {
        support_trap_exception_handler();
    }

}