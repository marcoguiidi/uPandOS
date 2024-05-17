#ifndef INTERRUPTS_H_INCLUDED
#define INTERRUPTS_H_INCLUDED

#include <umps3/umps/types.h>
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"



#include "initial.h"

/*
* funzione da richiamare son un pthread, in quanto cicla continuamente alla ricerca di interrupt
*/
void interruptHandler();

/*
* manage non-timer interrupts (lines 3-7)
*/
void nonTimerInterrupt(int);

/*
* manage PLT interrupts (line 1)
*/
void PLTinterrupt();

/*
* manage Interval Timer interrupts (line 2)
*/
void ITinterrupt();

/*
* returns the devNo associated
*/
int calcDevNo(unsigned int address);

void TrapExceptionHandler(state_t *exec_state);
void TLBExceptionHandler(state_t *exec_state);

#endif