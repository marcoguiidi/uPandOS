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
#include <umps3/umps/libumps.h>

#include "../klog.h"


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
                klog_print("interrupt not resolved");
            }
        }
        line++;
    }
}



void nonTimerInterrupt(int line){

    /* 1
    * calculate address of this device device's register
    */
    unsigned int devAddrBase;
    unsigned int add;
    int devNo;

    // switch case per ottenere indirizzo della word per ottenere poi il device number
    switch (line) {
        case DISKINT:
            add = 0x1000002C;
            break;
        case FLASHINT: 
            add = 0x1000002C + 0x04;
            break;
        case NETWINT: 
            add = 0x1000002C + 0x08;
            break;
        case PRNTINT: 
            add = 0x1000002C + 0x0C;
            break;
        case TERMINT: 
            add = 0x1000002C + 0x10;
            break;
    }

    // cercare il primo bit acceso all'interno della word associata a quell' indirizzo e assegnarlo a devNo
    devNo = calcDevNo(add);

    // calcolo indirizzo base
    devAddrBase = 0x10000054 + (line * 0x80) + (devNo * 0x10);

    /* 2 && 3
    * save off status code && ACK command the outstanding interrupt
    */
    unsigned int statusCode;
    unsigned int charCode;
    devreg_t* devReg =  0x10000254; //devAddrBase;
    
    /*if (line == 7){
        if (devReg->term.transm_status & mask){  // codice 5 nel campo status del device register del terminale
            statusCode = devReg->term.transm_status; 
            devReg->term.transm_command = ACK;
        }else{
            statusCode = devReg->term.recv_status;
            devReg->term.recv_command = ACK;
        }
    }else{
        statusCode = devReg->dtp.status;
        devReg->dtp.command = ACK;
    }*/
    unsigned int statusCodeRaw = devReg->term.transm_status;
    statusCode = devReg->term.transm_status & 0b11111111; 
    charCode   = devReg->term.transm_status & 0b1111111100000000;
    
    klog_print(" ");
    klog_print_dec(statusCode);
    klog_print(" ");

    devReg->term.transm_command = ACK;

    statusCode = devReg->term.transm_status & 0b11111111;
    if (statusCode != 1) {
        KLOG_PANIC("ack not acked");
    }
    /* 4
    * send message to unblock caller pcb
    */
    int terminalline = 7;
    int terminalnumber = 0;
    pcb_t* unblocked = removeProcQ(&blocked_pcbs[calcBlockedQueueNo(terminalline, terminalnumber)]);
    
    if (unblocked == NULL) {
        KLOG_PANIC("pcb not found");
    }

    msg_t* msg = allocMsg();
    if (msg == NULL) { // messaggi finiti
        KLOG_PANIC("messaggi finiti");
    }
    msg->m_sender = ssi_pcb; 
    msg->m_payload = statusCodeRaw;
    pushMessage(&unblocked->msg_inbox, msg);
    insertProcQ(&ready_queue, unblocked);
    soft_block_count--;

    /* 7
     * return control to current process
    */
    if (current_process == NULL) {
        scheduler();
    } else {
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
        insertProcQ(&ready_queue, current_process);
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
        return;
    }
    
    /* 3
    * return control to current process
    */
    
    LDST((state_t*)BIOSDATAPAGE);
}

int calcDevNo(unsigned int address){
    unsigned int* word = address;
    
    if (*word & DEV0ON){
        return 0;
    }else if (*word & DEV1ON){
        return 1;
    }else if (*word & DEV2ON){
        return 2;
    }else if (*word & DEV3ON){
        return 3;
    }else if (*word & DEV4ON){
        return 4;
    }else if (*word & DEV5ON){
        return 5;
    }else if (*word & DEV6ON){
        return 6;
    }else if (*word & DEV7ON){
        return 7;
    }

    return -1;
}