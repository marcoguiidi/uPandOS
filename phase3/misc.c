#include "../headers/const.h"
#include "../headers/listx.h"
#include "../headers/types.h"
#include "../phase1/headers/msg.h"
#include "../phase1/headers/pcb.h"
#include "../phase2/headers/initial.h"

#include <umps3/umps/libumps.h>

#include "./headers/misc.h"

/*
ssi serveces wrappers
*/

support_t* get_support_data(void) {
    support_t *support_data;
    ssi_payload_t payload = {
        .service_code = GETSUPPORTPTR,
        .arg = NULL,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&support_data), 0);
    return support_data;
}

pcb_t *create_process(state_t* state, support_t* support) {
    pcb_t *p;
    ssi_create_process_t ssi_create_process = {
        .state = state,
        .support = support,
    };
    ssi_payload_t payload = {
        .service_code = CREATEPROCESS,
        .arg = &ssi_create_process,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&p), 0);
    return p;
}

#define SELF (pcb_t*)NULL
pcb_t* kill_process(pcb_t* process) {
    pcb_t *p;
    ssi_payload_t payload = {
        .service_code = TERMPROCESS,
        .arg = process,
    };
    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)&payload, 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&p), 0);
    return p;
}