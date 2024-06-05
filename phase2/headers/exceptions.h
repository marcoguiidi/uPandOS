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

extern cpu_t interrupt_enter_time;

void uTLB_RefillHandler();

void TrapExceptionHandler(state_t *exec_state);

void TLBExceptionHandler(state_t *exec_state);

void passUpOrDie(unsigned type, state_t *exec_state);

void exceptionHandler(); 

void systemcallHandler(state_t* exceptionState);

#endif