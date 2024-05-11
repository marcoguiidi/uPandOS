#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

#include "../phase1/headers/msg.h"
#include "../phase1/headers/pcb.h"

#include "./headers/initial.h"
#include "./headers/scheduler.h"
#include "./headers/ssi.h"

#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"
#include "/usr/include/umps3/umps/arch.h"
#include "/usr/include/umps3/umps/types.h"

void uTLB_RefillHandler(); 

void exceptionHandler(); 

int isInPcbFree_h (pcb_t* destination);

int isInReadyQueue (pcb_t* destination);

void systemcallHandler(state_t* exceptionState);

void passUpOrDieHandler (int index);

#endif