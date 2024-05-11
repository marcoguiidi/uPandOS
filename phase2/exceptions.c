/*
exceptions.c This module implements the TLB, Program Trap, and SYSCALL exception
handlers. Furthermore, this module will contain the provided skeleton TLB-Refill event handler
(e.g. uTLB_RefillHandler).
*/
#include "./headers/exceptions.h"

// Exception handler function
void exceptionHandler() {
    // Perform a multi-way branch depending on the cause of the exception
    switch ((getCAUSE() >> CAUSESHIFT) & GETEXECCODE) {
        case EXC_INT:
            interruptHandler();
            break;
        case EXC_MOD:
        case EXC_TLBL:
        case EXC_TLBS:
            passUpOrDieHandler(PGFAULTEXCEPT);            
            break;
        case EXC_SYS:
            systemcallHandler(EXCEPTION_STATE);
            break;
        default: //4-7, 9-12
            passUpOrDieHandler(GENERALEXCEPT);            
            break;
    }
}


int isInPcbFree_h (pcb_t* destination) {
    struct pcb_t* iter;
    // itero sugli elementi della lista pcbFree_h
    LIST_FOREACH(iter, &pcbFree_h, p_list) {
        // confronto l'indirizzo del PCB del processo di destinazione con gli indirizzi dei PCB nella lista
        if (iter == destination) {
            // Il processo di destinazione è nella lista pcbFree_h
            return 1;
        }
    }
    // Il processo di destinazione non è nella lista pcbFree_h
    return 0;
}
int isInReadyQueue (pcb_t* destination) {
    struct pcb_t* iter;
    // itero sugli elementi della lista pcbFree_h
    LIST_FOREACH(iter, &ready_queue, p_list) {
        // confronto l'indirizzo del PCB del processo di destinazione con gli indirizzi dei PCB nella lista
        if (iter == destination) {
            // Il processo di destinazione è nella ready queue
            return 1;
        }
    }
    // Il processo di destinazione non è nella ready queue
    return 0;
}


void systemcallHandler(state_t* exceptionState) {
     if ((~STATUS_KUp) & (reg_a0>=(-2) & reg_a0<=(-1))){ 
        unsigned int syscallCode = CAUSE_GET_EXCCODE(exceptionState->pc_epc);  
        switch (syscallCode) {
            case SENDMESSAGE:
            reg_a0 = -1;
            reg_a2 = (unsigned int) payload;
            pcb_t* reg_a1 = pcb_t* destination;
            SYSCALL(SENDMESSAGE(destination, payload, 0));
            msg_PTR res = headMessage(destination);
            if (res->m_payload==payload) {
                //messaggio arrivato
                exceptionState->v0 = 0;
            }
             else if (isInPcbFree_h(destination) == 1) {
                 //il processo di destinazione è nella lista pcbFree_h
                exceptionState->v0 = DEST_NOT_EXIST;     
            }
            else if (isInReadyQueue(destination)) {
                //il processo di destinazione è nella ready queue
                pushMessage(destination, payload);
            }
            else if (emptyMessageQ(destination)==1){ 
                //il processo attende un messaggio
                  insertProcQ(ready_queue, destination);
            } else {
                 return (MSGNOGOOD);
            }
                break;

            case RECEIVEMESSAGE:
            reg_a0 = -2;
            reg_a1=pcb_t* sender;
            reg_a2= NULL;//perchè il payload viene ignorato (0 nella chiamata)
            if (reg_a1 == ANYMESSAGE){
                    if (emptyMessageQ(sender) == 1){    
                    /*If a1 contains a ANYMESSAGE pointer, then the requesting process 
                    is looking for the first message in its inbox, without any restriction 
                    about the sender. In this case it will be frozen only if the queue is empty,
                    and the first message sent to it will wake up it and put it in the Ready Queue????.*/
                    return gerPRID();
                }
                return (headMessage(sender)->m_payload);
            } 
            SYSCALL(RECEIVEMESSAGE, sender, (unsigned int)payload, 0);
            
                break;
        }
    
    } else { //user mode
        Cause.ExcCode(BIOS_DATA_PAGE_BASE)= RI; // Set ExcCode to Reserved Instruction
        passUpOrDieHandler(GENERALEXCEPT); 
    }
}


void uTLB_RefillHandler() {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_t*) 0x0FFFF000);
}


void passUpOrDieHandler (int index) {
  if (current_process->p_supportStruct == NULL){
      terminateprocess(current_process);
      scheduler();
    }
    else{
        current_process->p_supportStruct->sup_exceptState[index] = *EXCEPTION_STATE;
        context_t cont = currentProcess->p_supportStruct->sup_exceptContext[index];
        LDCXT (cont.c_stackPtr, cont.c_status, cont.c_pc);
    }
}