/*
interrupts.c This module implements the device/timer interrupt exception handler. This
module will process all the device/timer interrupts, converting device/timer interrupts into ap-
propriate messages for the blocked PCBs.
*/

#include "./headers/interrupts.h"
#include "./headers/initial.h"
#include "headers/misc.h"
#include "headers/scheduler.h"
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include "headers/exceptions.h"
#include "../klog.h"


cpu_t get_elapsed_time_interupt() {
    cpu_t now;
    STCK(now);
    return (now - interrupt_enter_time);
}

void interruptHandler(){
    
    unsigned int cause = getCAUSE();
    

    int line=1; // parto da 1 così ottengo subito la priorità maggiore
    while (line < 8){
        if (cause & CAUSE_IP(line)){
            if (line >= 3 && line <= 7){
                nonTimerInterrupt(line);
            }else if (line == 1){
                PLTinterrupt();
            }else if (line == 2){ // va sempre qua
                ITinterrupt();
            } else {
                KLOG_PANIC("interrupt not resolved");
            }
        }
        line++;
    }
}



void nonTimerInterrupt(int line){


    // calulate the device number
    unsigned int cause = getCAUSE();
    unsigned int devnum;
    unsigned int* installed_device_map;
    
    // get installed device bitmap for a given line
    switch (line) {
        case 3:
            installed_device_map = 0x1000002c;
            break;
        case 4:
            installed_device_map = 0x1000002c + 0x04;
            break;
        case 5:
            installed_device_map = 0x1000002c + 0x08;
            break;
        case 6:
            installed_device_map = 0x1000002c + 0x0c;
            break;
        case 7:
            installed_device_map = 0x1000002c + 0x10;
            break;
        default:
            KLOG_PANIC("line don't match");
    }

    // get the first installed device for that line
    switch (*installed_device_map) {
        case DEV0ON:
            devnum = 0;
            break;
        case DEV1ON:
            devnum = 1;
            break;
        case DEV2ON:
            devnum = 2;
            break;
        case DEV3ON:
            devnum = 3;
            break;
        case DEV4ON:
            devnum = 4;
            break;
        case DEV5ON:
            devnum = 5;
            break;
        case DEV6ON:
            devnum = 6;
            break;
        case DEV7ON:
            devnum = 7;
            break;
        default:
            KLOG_PANIC("installed_device_map don't match");
    }
    

    unsigned int devAddrBase = 0x10000054 + ((line - 3) * 0x80) + (devnum * 0x10);
    unsigned int statusCodeRaw;
    // use a given device accordingly for his type
    switch (line) {
        case 3:
            // disk device
            break;
        case 4:
            // flash device
            break;
        case 5:
            // network device
            break;
        case 6:
            // printer device
            break;
        case 7:
            // terminal device
            ;
            devreg_t* devReg =  (devreg_t*)devAddrBase;

            statusCodeRaw = devReg->term.transm_status; // save status code
            devReg->term.transm_command = ACK; // ack device

            break;
        default:
            KLOG_PANIC("line don't match");
    }
    
    pcb_t* unblocked = removeProcQ(&blocked_pcbs[calcBlockedQueueNo(line, devnum)]);
    
    if (unblocked == NULL) {
        KLOG_PANIC("pcb not found");
    }
    
    soft_block_count--;
    

    msg_t* msg = allocMsg();
    if (msg == NULL) { // messaggi finiti
        KLOG_PANIC("messaggi finiti");
    }
    msg->m_sender = ssi_pcb; 
    msg->m_payload = statusCodeRaw;
    pushMessage(&unblocked->msg_inbox, msg);
    insertProcQ(&ready_queue, unblocked);

    /* 7
     * return control to current process
    */
    if (current_process == NULL) {
        scheduler();
    } else {
        current_process->p_time -= get_elapsed_time_interupt(); // time elapsed in interrupts doesn't count
        LDST((state_t*)BIOSDATAPAGE);
    }
}

void PLTinterrupt(){

    /* 1
    * Acknowledge PLT
    */
    setTIMER(TIMESLICE);

    /* 2
    * copy processor state in current process
    */
    state_t *saved = (state_t*)BIOSDATAPAGE;

    if (current_process != NULL) {
        copy_state_t(saved, &current_process->p_s);
        current_process->p_time += get_elapsed_time(); // time elapsed in interrupts doesn't count
        current_process->p_time -= get_elapsed_time_interupt();
        insertProcQ(&ready_queue, current_process);
        current_process = NULL;
    } else {
        KLOG_PANIC("pcb is NULL");
    }
    
    /* 4
    * call the scheduler
    */
    scheduler();
}

void ITinterrupt(){

    /* 1
    * acknowledge the interrupt
    */
    LDIT(PSECOND);

    /* 2
    * unblock all pcbs waiting a pseudo clock tick
    */
    pcb_t* pcb_to_unblock;
    while ((pcb_to_unblock = removeProcQ(&blocked_pcbs[BOLCKEDPSEUDOCLOCK])) != NULL) {
        insertProcQ(&ready_queue, pcb_to_unblock);
        msg_PTR msg = allocMsg();
        msg->m_sender = ssi_pcb;
        msg->m_payload = 0;
        insertMessage(&pcb_to_unblock->msg_inbox, msg);
        soft_block_count--;
    }

    if (current_process == NULL) {
        scheduler();
    } else {
        current_process->p_time -= get_elapsed_time_interupt(); // time elapsed in interrupts doesn't count
        LDST((state_t*)BIOSDATAPAGE);
    }

}