#ifndef SSI_H_INCLUDED
#define SSI_H_INCLUDED

#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"

#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"

/*
while (TRUE) {
    receive a request;              [X]
    satisfy the received request;
    send back the results;
}
*/
void SSI_function_entry_point();


/*
while (TRUE) {
    receive a request;
    satisfy the received request;   [X]
    send back the results;          [X]
}
*/
void SSIRequest(pcb_t* sender, int service, void* arg);

#endif