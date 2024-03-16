/*
ssi.c This module implements the System Service Interface process.
*/

#include "./headers/ssi.h"
#include "./headers/initial.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>

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

pcb_t* pid_to_pcb(unsigned int pid) {
    pcb_t* tmp;
    list_for_each_entry(tmp, &ready_queue, p_list) {
        if (tmp->p_pid == pid)
            return tmp;
    }
    return NULL; // non ci deve mai andare
}

void SSI_function_entry_point() {
    unsigned int payload;
    unsigned int pid_sender;
    pcb_t* sender;
    msg_t* msgin;
    while (TRUE) {
        /*pid_sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, payload, 0); DA RIFARE
        sender = pid_to_pcb(pid_sender);
        msgin = headMessage(&sender->msg_inbox);
        SSIRequest(sender, sender->p_s.reg_a2, (void*)msgin->m_payload);
        popMessage(msgin, sender);*/ 
    }
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
            break;
        case DOIO:
            // TODO: DoIO
            break;
        case GETTIME:
            break;
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