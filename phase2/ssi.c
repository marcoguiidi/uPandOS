/*
ssi.c This module implements the System Service Interface process.
*/

#include "./headers/ssi.h"
#include "./headers/initial.h"
#include "./headers/misc.h"
#include "headers/interrupts.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>


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
            ssi_do_io_t* doio = payload->arg;
            int devno;
            int intlineno;
            devAddrBase_get_IntlineNo_DevNo(doio->commandAddr, &intlineno, &devno);

            pcb_t* suspended_process = outProcQ(&ready_queue, sender);
            // save it on the corrisponding device
            insertProcQ(&blocked_pcbs[calcBlockedQueueNo(intlineno, devno)], suspended_process);
            soft_block_count++;
            *doio->commandAddr = doio->commandValue;
            /*
            * every process should now be in a blocked state as everyone is waiting for a task to end, so the
                the scheduler should call the WAIT() function; Important: The current process must be set to
                NULL and all interrupts must be ON; ???
            * an interrupt exception should be raised by the CPU; ???
            given the cause code, the interrupt handler should understand which device triggered the TRAP;
            check the status and send to the device an acknowledge (setting the device command address to ACK);
            
            ci pensano gli interrups a mandare la risposta del device
            */

            // ...
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