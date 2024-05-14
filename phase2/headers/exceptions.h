#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"

#include "initial.h"
#include "scheduler.h"
#include "ssi.h"

#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"
#include "/usr/include/umps3/umps/arch.h"
#include "/usr/include/umps3/umps/types.h" 

void exceptionHandler(); 

void systemcallHandler(state_t* exceptionState);

void passUpOrDieHandler (int index);

void uTLB_RefillHandler();

void passUpOrDie(unsigned type, state_t *exec_state);

#endif