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


int devAddrBase_get_IntlineNo_DevNo(unsigned int devAddrBase, int* IntlineNo, int* DevNo) {
    // bruteforce da cambiare magari
    for (int intlineno = 0; intlineno <= 8; intlineno++) {
        for (int devno = 0; devno <= DEV7ON; devno++) {
            if (devAddrBase == (0x10000054 + ((intlineno - 3) * 0x80) + (devno * 0x10))) {
                *IntlineNo = intlineno;
                *DevNo = devno;
                return 1;
            }
        }
    }
    return 0;
}

void SSI_function_entry_point() {
    ssi_payload_t* payload;
    pcb_t* sender;
    while (TRUE) {
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
            if (newprocess == NULL) {
                SYSCALL(SENDMESSAGE, sender, NOPROC, 0);
            } else {
                copy_state_t(data->state, &newprocess->p_s);
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
            ssi_do_io_PTR doioarg = (ssi_do_io_PTR)arg;
            /*
            uso sempre il terinale che è questo da mettere a posto
            doio->commandAddr fa un exception
            */
            int devno = 0;
            int intlineno = 7;
            //devAddrBase_get_IntlineNo_DevNo(doio->commandAddr, &intlineno, &devno);

            pcb_t* suspended_process = out_pcb_in_all(sender);
            if (suspended_process == NULL) {
                KLOG_PANIC("cannot find process");
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
            insertProcQ(&blocked_pcbs[BOLCKEDPSEUDOCLOCK], out_pcb_in_all(sender));
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