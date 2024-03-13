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


void interruptHandler();

unsigned int getIp();

void nonTimerInterrupt(int);

void timerInterrupt(int);

#endif