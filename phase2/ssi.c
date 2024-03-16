/*
ssi.c This module implements the System Service Interface process.
*/

#include "./headers/ssi.h"
#include "./headers/initial.h"

/**
terminate the proces pointed by process and its progeny
*/
void terminateprocess(pcb_t* process) {
    while (!emptyChild(process)) {
        pcb_t* child = removeChild(process);
        terminateprocess(child);
    }
    process_kill(process);
}

void SSIRequest(pcb_t* sender, int service, void* arg) {

    switch (service) {
        case CREATEPROCESS:
            ;
            ssi_create_process_t* data = (ssi_create_process_t*)arg;
            pcb_t* newprocess = allocPcb();
            if (newprocess == NULL) return; //return NOPROC;  //TODO: COME DEVO RITORNARE???
            newprocess->p_s = data->state;
            newprocess->p_supportStruct = data->support;
            insertChild(sender, newprocess); // is child of sender
            process_spawn(newprocess);
            break;
        case TERMINATEPROCESS:
            if (arg == NULL) {
                terminateprocess(sender);
            } else {
                pcb_t* processtokill = (pcb_t*)arg;
                terminateprocess(processtokill);
            }
        default:
            /*
            If service does not match any of those 
            provided by the SSI, the SSI should terminate the process
            and its progeny
            */
            terminateprocess(sender);
            break;
    }
}