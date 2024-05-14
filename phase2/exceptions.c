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
#include <umps3/umps/libumps.h>

// Exception handler function
void exceptionHandler() {
    // Perform a multi-way branch depending on the cause of the exception
    
    unsigned int status = getSTATUS();
    unsigned int cause  = getCAUSE();
    unsigned int ExcCode = CAUSE_GET_EXCCODE(cause);
    state_t *exception_state = (state_t *)BIOSDATAPAGE;

    unsigned int was_in_kernel_mode = IN_KERNEL_MODE(exception_state->cause);
    switch (ExcCode) {
        case IOINTERRUPTS:
            interruptHandler(); // TODO:
            break;
        case EXC_MOD: // phase 3
            break;
        case EXC_TLBL: // phase 3
            break;
        case EXC_TLBS: // phase 3
            passUpOrDieHandler(PGFAULTEXCEPT);            
            break;
        case SYSEXCEPTION:
            if (was_in_kernel_mode)
                systemcallHandler(exception_state);
            else
                ; // TODO trap
            break;
        default: //4-7, 9-12
            passUpOrDieHandler(GENERALEXCEPT);            
            break;
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
                exceptionState->reg_v0 = DEST_NOT_EXIST; // reg v0 ?? a0 ?
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
            msg->m_payload = (unsigned)reg_A0;
            msg->m_sender = current_process;
            if (isInBlocked_pcbs(dest_process->p_pid)) {
                // is waiting, unblock it
                insertProcQ(&ready_queue, getBlocked_pcbs(dest_process->p_pid));
                soft_block_count--;
            }
            // add message to dest
            insertMessage(&dest_process->msg_inbox, msg);
            exceptionState->reg_v0 = 0; // succes

            // reuturn from non bloking
            exceptionState->pc_epc += WORDLEN;
            LDST(exceptionState);
            break;

        case RECEIVEMESSAGE:
            ;
            pcb_t *sender_process = (pcb_PTR)reg_A1;
            msg = popMessage(&current_process->msg_inbox, sender_process);
            
            // no message, wait
            if (msg == NULL) {
                // is running, put in waiting state
                insertProcQ(&blocked_pcbs[BLOKEDRECV], current_process);
                soft_block_count++;
                // copy state
                copy_state_t(exceptionState, &(current_process->p_s));
                // TODO: TIMER
                
                // call the scheduler
                scheduler();
                return; // non ci deve mai andare
            }

            // return the payload
            exceptionState->reg_v0 = (unsigned int)msg->m_sender;

            if (reg_A2 != 0) {
                // has a payload
                *((unsigned int*)reg_A2) = (unsigned int)msg->m_payload;
            }
            break;
        
        default:
            // invalid syscalkl
            // TODO: trap
            break;
    }
    // reuturn from non bloking
    exceptionState->pc_epc += WORDLEN;
    LDST(exceptionState);
}


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
    // then the process and the progeny of the process must be terminated
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