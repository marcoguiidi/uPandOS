#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

void uTLB_RefillHandler(); 

void exceptionHandler(); 

int isInPcbFree_h (pcb_t* destination);

int isInReadyQueue (pcb_t* destination);

void systemcallHandler(state_t* exceptionState);

void passUpOrDieHandler (int index);

#endif