#ifndef SSI_H_INCLUDED
#define SSI_H_INCLUDED

#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"

#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"

#define SSI_CREATEPROCESS 1
#define SSI_TERMINATEPROCESS 2
#define SSI_DODIO 3
#define SSI_GETCPUTIME 4
#define SSI_WAITFORCLOCK 5
#define SSI_GETSUPPORTDATA 6
#define SSI_GETPROCESSID 7

void SSI_function_entry_point(pcb_t* sender, int service, void* arg); //TODO:

#endif