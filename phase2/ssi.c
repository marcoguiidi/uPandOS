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
        pid_sender = SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, payload, 0);
        sender = pid_to_pcb(pid_sender);
        // current_process now is SSI
        msgin = popMessage(&current_process->msg_inbox, NULL); // get message from ANY
        SSIRequest(sender, sender->p_s.reg_a2, (void*)payload);
    }
}

void SSIRequest(pcb_t* sender, int service, void* arg) {

    switch (service) {
        case CREATEPROCESS:
            ;
            ssi_create_process_t* data = (ssi_create_process_t*)arg;
            pcb_t* newprocess = allocPcb();
            if (newprocess == NULL) {
                SYSCALL(SENDMESSAGE, sender, NOPROC, 0);
            } else {
                newprocess->p_s = *data->state;
                newprocess->p_supportStruct = data->support;
                insertChild(sender, newprocess); // is child of sender
                process_spawn(newprocess);
                SYSCALL(SENDMESSAGE, sender, newprocess, 0); // return new process
            }
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
            ;
            ssi_payload_t* payload = (ssi_payload_t*) arg;
              
            break;
        case GETTIME:
            ;
            cpu_t time = sender->p_time;
            SYSCALL(SENDMESSAGE, sender, time, 0);
            break;
        case CLOCKWAIT:
            ;
            // waiting pseudo-clock implementation
            // TODO: WaitForClock
            break;
        case GETSUPPORTPTR:
            ;
            support_t* supportdata = sender->p_supportStruct;
            SYSCALL(SENDMESSAGE, sender, supportdata, 0);
            break;
        case GETPROCESSID:
            ;
            unsigned int pid;
            if (arg == 0) {
                pid = sender->p_pid;
            } else {
                pid = sender->p_parent->p_pid;
            }
            SYSCALL(SENDMESSAGE, sender, pid, 0);
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