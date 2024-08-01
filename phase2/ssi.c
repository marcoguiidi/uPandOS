/*
ssi.c This module implements the System Service Interface process.
*/

#include "./headers/ssi.h"
#include "./headers/initial.h"
#include "./headers/misc.h"
#include "headers/interrupts.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/arch.h>
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

int devaddr_get_lineno_devno_regno(memaddr* devaddr, int* lineno, int* devno, int* regno) {
    int retstatus = 0;
    memaddr* base = 0;
    if (devaddr >= TERMINAL_DEVICES_START) {
        base = TERMINAL_DEVICES_START;
        *lineno = 7;
    } else if (devaddr >= PRINTER_DEVICES_START) {
        base = PRINTER_DEVICES_START;
        *lineno = 6;
    } else if (devaddr >= ETHERNET_DEVICES_START) {
        base = ETHERNET_DEVICES_START;
        *lineno = 5;
    } else if (devaddr >= FLASH_DEVICES_START) {
        base = FLASH_DEVICES_START;
        *lineno = 4;
    } else if (devaddr >= DISK_DEVICES_START) {
        base = DISK_DEVICES_START;
        *lineno = 3;
    } else {
        retstatus = -1;
    }

    *devno = (devaddr - base) / 16;
    *regno = (devaddr - base) % 16;

    return retstatus;
}

void SSIRequest(pcb_t* sender, int service, void* arg) {
    switch (service) {
        case CREATEPROCESS:
            ;
            ssi_create_process_t* data = (ssi_create_process_t*)arg;
            pcb_t* newprocess = allocPcb();
            if (newprocess == NULL) { // no more free pcb
                SYSCALL(SENDMESSAGE, (unsigned int)sender, NOPROC, 0);
            } else {
                copy_state_t(data->state, &newprocess->p_s);
                newprocess->p_supportStruct = data->support;
                newprocess->p_time = 0;
                insertChild(sender, newprocess); // make a child of sender
                process_spawn(newprocess); // add to the ready queue
                SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)newprocess, 0); // return new process
            }
            break;
        case TERMPROCESS:
            if (arg == NULL) {
                process_killall(sender); // no need to repy, the process is dead
            } else {
                pcb_t* processtokill = (pcb_t*)arg;
                process_killall(processtokill);
                SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0); // response
            }
            
            break;
        case DOIO:
            ;
            ssi_do_io_PTR doioarg = (ssi_do_io_PTR)arg;

            
            int intlineno, devno, regno;
            int ret = devaddr_get_lineno_devno_regno(doioarg->commandAddr, &intlineno, &devno, &regno);

            if (ret != 0) {
                KLOG_PANIC("device not known")
            }
            
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
            SYSCALL(SENDMESSAGE, (unsigned int)sender, time, 0);
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
            SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)supportdata, 0);
            break;
        case GETPROCESSID:
            ;
            unsigned int pid;
            if (arg == 0) {
                pid = sender->p_pid;
            } else {
                pid = sender->p_parent->p_pid;
            }
            SYSCALL(SENDMESSAGE, (unsigned int)sender, pid, 0);
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