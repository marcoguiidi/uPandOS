/*
exceptions.c This module implements the TLB, Program Trap, and SYSCALL exception
handlers. Furthermore, this module will contain the provided skeleton TLB-Refill event handler
(e.g. uTLB_RefillHandler).
*/
#include "./headers/exceptions.h"
#include "headers/misc.h"
#include "headers/initial.h"
#include "headers/scheduler.h"
#include "headers/interrupts.h"

#include "../phase1/headers/msg.h"
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include "../klog.h"

void uTLB_RefillHandler() {
    // 1 determine the page number of the missing TLB entry
    state_t *exception_state = (state_t *)BIOSDATAPAGE;
    unsigned int missing_page_num = (exception_state->entry_hi & GETPAGENO) >> VPNSHIFT;
    if (missing_page_num > USERPGTBLSIZE) {
        missing_page_num = USERPGTBLSIZE -1;
    }

    // 2 get the page table entry for page number for the current process
    pteEntry_t* missing_tlb_entry = &current_process->p_supportStruct->sup_privatePgTbl[missing_page_num];

    //klog_print("[missing page ");
    //klog_print_dec(missing_page_num);
    //klog_print(" tlb entry");
    //klog_print_hex(missing_tlb_entry->pte_entryHI);
    //klog_print(" ");
    //klog_print_hex(missing_tlb_entry->pte_entryLO);
    //klog_print("]");
    
    // 3 write the page table entry into the TLB
    setENTRYHI(missing_tlb_entry->pte_entryHI);
    setENTRYLO(missing_tlb_entry->pte_entryLO);
    TLBWR();
    
    // 4 return control to the current process
    LDST(exception_state);
}


void TrapExceptionHandler(state_t *exec_state) { 
    passUpOrDie(GENERALEXCEPT, exec_state); 
}

void TLBExceptionHandler(state_t *exec_state) {
     passUpOrDie(PGFAULTEXCEPT, exec_state); 
}

void passUpOrDie(unsigned type, state_t *exec_state) {
  if (current_process == NULL || current_process->p_supportStruct == NULL) {
    if (current_process == NULL) {
        KLOG_PANIC("unknown process to kill");
    } else {
        process_killall(current_process);
    }
    
    scheduler();
  }
  // salva lo stato del processo
  copy_state_t(exec_state, &current_process->p_supportStruct->sup_exceptState[type]);
  
  // aggiorna cput time
  current_process->p_time += get_elapsed_time();
  current_process->p_time -= get_elapsed_time_interupt();
  
  // passa l'eccezione
  context_t context_pass_to = current_process->p_supportStruct->sup_exceptContext[type];
  LDCXT(context_pass_to.stackPtr, context_pass_to.status, context_pass_to.pc);
}

/*
time saved when entering exception,
time used in sycall is counted, 
tracking of time spent in excHandler
*/
cpu_t interrupt_enter_time;

void exceptionHandler() {
    STCK(interrupt_enter_time);
    //STCK legge il valore del registro del contatore di clock e lo memorizza nella var interrupt_enter_time
    unsigned int cause  = getCAUSE();
    unsigned int ExcCode = CAUSE_GET_EXCCODE(cause);
    state_t *exception_state = (state_t *)BIOSDATAPAGE;
    unsigned int was_in_kernel_mode = in_kernel_mode(exception_state->cause);
    
    if (ExcCode == IOINTERRUPTS) {
        interruptHandler();
    }
    else if (ExcCode >= 1 && ExcCode <= 3) {
        TLBExceptionHandler(exception_state);
    }
    else if ((ExcCode >= 4 && ExcCode <= 7) || (ExcCode >= 9 && ExcCode <= 12)) {
        TrapExceptionHandler(exception_state);
    }
    else if (ExcCode == SYSEXCEPTION) {
        if (was_in_kernel_mode)
            systemcallHandler(exception_state);
        else {
            exception_state->cause = PRIVINSTR; // RI = 10 reserved instruction
            TrapExceptionHandler(exception_state);
        }    
    } else {
        KLOG_PANIC(" execode not match")
    }
}

void systemcallHandler(state_t* exceptionState) {
    // syscall type
    int reg_A0 = exceptionState->reg_a0;
    // dest process
    unsigned int reg_A1 = exceptionState->reg_a1;
    // payload of the mex
    int reg_A2 = exceptionState->reg_a2;

    msg_t *msg;

    switch (reg_A0) {
    //perform a multi-way branching depending on the type of exception
        case SENDMESSAGE: {
            
            pcb_t* dest_process = (pcb_t*)reg_A1;
            
            if (reg_A1 == 0) { 
                //non è stato dato un puntatore valido per il dest process
                exceptionState->reg_v0 = DEST_NOT_EXIST;
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
        case RECEIVEMESSAGE: {
            
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
            TrapExceptionHandler(exceptionState);
            break;
    }
    // reuturn from non bloking
    exceptionState->pc_epc += WORDLEN; //pc updated
    /*move the PC to the next instruction, avoiding entering an 
    infinite loop that would repeat the same syscall*/
    LDST(exceptionState);
    //restores processor state
}