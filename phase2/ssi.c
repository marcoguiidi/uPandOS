/*
ssi.c This module implements the System Service Interface process.
*/

#include "./headers/ssi.h"
#include "./headers/initial.h"
#include "./headers/misc.h"
#include "headers/interrupts.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include "../klog.h"

void SSI_function_entry_point() {
    ssi_payload_t* payload;
    pcb_t* sender;
    while (TRUE) {
        // wait for a service request
        sender = (pcb_t*)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);
        
        SSIRequest(sender, payload->service_code, payload->arg);
    }
}

void SSIRequest(pcb_t* sender, int service, void* arg) {
    switch (service) {
        case CREATEPROCESS:
            ;
            ssi_create_process_t* data = (ssi_create_process_t*)arg;
            pcb_t* newprocess = allocPcb();
            if (newprocess == NULL) { // no more free pcb
                SYSCALL(SENDMESSAGE, sender, NOPROC, 0);
            } else {
                copy_state_t(data->state, &newprocess->p_s);
                newprocess->p_supportStruct = data->support;
                newprocess->p_time = 0;
                insertChild(sender, newprocess); // make a child of sender
                process_spawn(newprocess); // add to the ready queue
                SYSCALL(SENDMESSAGE, sender, newprocess, 0); // return new process
            }
            break;
        case TERMPROCESS:
            if (arg == NULL) {
                process_killall(sender); // no need to repy, the process is dead
            } else {
                pcb_t* processtokill = (pcb_t*)arg;
                process_killall(processtokill);
                SYSCALL(SENDMESSAGE, sender, 0, 0); // response
            }
            
            break;
        case DOIO:
            ;
            ssi_do_io_PTR doioarg = (ssi_do_io_PTR)arg;
            
            /*
            print to the first terminal
            */
            int devno = 0;
            int intlineno = 7;

            pcb_t* suspended_process = out_pcb_in_all(sender);
            if (suspended_process == NULL) {
                KLOG_PANIC("pcb not found");
            }
            // save it on the corrisponding device
            insertProcQ(&blocked_pcbs[calcBlockedQueueNo(intlineno, devno)], suspended_process);
            soft_block_count++;
            *doioarg->commandAddr = doioarg->commandValue;
            break;
        case GETTIME:
            ;
            cpu_t time = sender->p_time;
            SYSCALL(SENDMESSAGE, sender, time, 0);
            break;
        case CLOCKWAIT:
            ;
            // move the process in blocked state waiting for clock
            insertProcQ(&blocked_pcbs[BOLCKEDPSEUDOCLOCK], out_pcb_in_all(sender));
            soft_block_count++;
            // don't repy so it's remain waiting
            break;
        case GETSUPPORTPTR:
            ;
            support_t* supportdata = sender->p_supportStruct;
            SYSCALL(SENDMESSAGE, sender, supportdata, 0);
            break;
        case GETPROCESSID:
            ;
            int arg_getprocessid = (int)arg;
            unsigned int pid;
            if (arg == 0) {
                pid = sender->p_pid;
            } else {
                pid = sender->p_parent->p_pid;
            }
            SYSCALL(SENDMESSAGE, sender, pid, 0);
            break;
        default:
            /*
            If service does not match any of those 
            provided by the SSI, the SSI should terminate the process
            and its progeny
            */
            process_killall(sender);
            break;
    }
}