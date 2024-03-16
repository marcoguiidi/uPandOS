#ifndef SSI_H_INCLUDED
#define SSI_H_INCLUDED

#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"

#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"

#define CREATEPROCESS 1
#define TERMINATEPROCESS 2
#define DODIO 3
#define GETCPUTIME 4
#define WAITFORCLOCK 5
#define GETSUPPORTDATA 6
#define GETPROCESSID 7

void SSIRequest(pcb_t* sender, int service, void* arg);

#endif