/*
exceptions.c This module implements the TLB, Program Trap, and SYSCALL exception
handlers. Furthermore, this module will contain the provided skeleton TLB-Refill event handler
(e.g. uTLB_RefillHandler).
*/
#include "./headers/exceptions.h"
#include "headers/misc.h"
#include "headers/initial.h"
#include "../phase1/headers/msg.h"
#include "headers/scheduler.h"
#include "headers/interrupts.h"
#include <umps3/umps/libumps.h>
#include "../klog.h"


void uTLB_RefillHandler() {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_t*) 0x0FFFF000);
}


void TrapExceptionHandler(state_t *exec_state) { passUpOrDie(GENERALEXCEPT, exec_state); }
void TLBExceptionHandler(state_t *exec_state) { passUpOrDie(PGFAULTEXCEPT, exec_state); }

void passUpOrDie(unsigned type, state_t *exec_state) {

  if (current_process == NULL || current_process->p_supportStruct == NULL) {
    process_kill(current_process);
    scheduler();
    return;
  }
  // salva lo stato del processo
  copy_state_t(exec_state, &current_process->p_supportStruct->sup_exceptState[type]);
  // fare roba cpu time TODO: asd
  
  
  // passa l'eccezione
  context_t context_pass_to = current_process->p_supportStruct->sup_exceptContext[type];
  LDCXT(context_pass_to.stackPtr, context_pass_to.status, context_pass_to.pc);
}

// Exception handler function
void exceptionHandler() {
    // Perform a multi-way branch depending on the cause of the exception
    
    unsigned int status = getSTATUS();
    unsigned int cause  = getCAUSE();
    unsigned int ExcCode = CAUSE_GET_EXCCODE(cause);//(cause & GETEXECCODE) >> CAUSESHIFT;
    state_t *exception_state = (state_t *)BIOSDATAPAGE;

    unsigned int was_in_kernel_mode = IN_KERNEL_MODE(exception_state->cause);
    
    klog_print_dec(ExcCode);
    
    if (ExcCode == IOINTERRUPTS) {
        interruptHandler();
    }
    else if (ExcCode >= 1 && ExcCode <= 3) {
        TLBExceptionHandler(exception_state);
    }
    else if ((ExcCode >= 4 && ExcCode <= 7) || (ExcCode >= 9 && ExcCode <= 12)) {
        klog_print_dec(ExcCode);
        KLOG_PANIC("");
        TrapExceptionHandler(exception_state);
    }
    else if (ExcCode == SYSEXCEPTION) {
        if (was_in_kernel_mode)
            systemcallHandler(exception_state);
        else
            ; // TODO trap
    } else {
        KLOG_PANIC(" execode not match")
    }
}

void systemcallHandler(state_t* exceptionState) {
    // syscall
    int reg_A0 = exceptionState->reg_a0;
    // process
    unsigned int reg_A1 = exceptionState->reg_a1;
    // payload
    int reg_A2 = exceptionState->reg_a2;

    msg_t *msg;

    
    switch (reg_A0) {
        case SENDMESSAGE:
            ;
            pcb_t* dest_process = (pcb_t*)reg_A1;
            
            if (reg_A1 == 0) {
                exceptionState->reg_v0 = DEST_NOT_EXIST;
                break;
            }
            if (isInPcbFree_h(dest_process->p_pid)) {
                //il processo di destinazione è nella lista pcbFree_h
                exceptionState->reg_v0 = DEST_NOT_EXIST;  
                break;   
            }
            // alloco messaggio
            msg = allocMsg();
            if (msg == NULL) {
                exceptionState->reg_v0 = MSGNOGOOD; // messaggi liberi esauriti
                break;                
            }
            msg->m_payload = (unsigned int)reg_A2;
            msg->m_sender = current_process;
            if (outProcQ(&blocked_pcbs[BLOKEDRECV], dest_process) != NULL) { // is blocked
                // is waiting, unblock it
                insertProcQ(&ready_queue, dest_process);
                soft_block_count--;
            }
            // add message to dest
            insertMessage(&dest_process->msg_inbox, msg);
            exceptionState->reg_v0 = 0; // succes

            break;

        case RECEIVEMESSAGE:
            ;
            pcb_t *sender_process = (pcb_PTR)reg_A1;
            msg = popMessage(&current_process->msg_inbox, sender_process);
            
            // no message, wait
            if (msg == NULL) {
                // is running, put in waiting state
                copy_state_t(exceptionState, &(current_process->p_s));
                insertProcQ(&blocked_pcbs[BLOKEDRECV], current_process);
                soft_block_count++;

                // TODO: TIMER
                
                // call the scheduler
                current_process = NULL;
                scheduler();
                return; // non ci deve mai andare
            }

            // return the payload
            exceptionState->reg_v0 = (unsigned int)msg->m_sender;

            if (reg_A2 != 0) {
                // save message if has one
                *((unsigned int*)reg_A2) = (unsigned int)msg->m_payload;
            }
            break;
        
        default:
            KLOG_PANIC("");
            TrapExceptionHandler(exceptionState);
            break;
    }
    // reuturn from non bloking
    exceptionState->pc_epc += WORDLEN;
    LDST(exceptionState);
}