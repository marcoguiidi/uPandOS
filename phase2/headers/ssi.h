#ifndef SSI_H_INCLUDED
#define SSI_H_INCLUDED

#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"

#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"

// for spec compatability spec2 is wrong
#define TERMINATEPROCESS TERMPROCESS

void SSI_function_entry_point();

void SSIRequest(pcb_t* sender, int service, void* arg);

#endif