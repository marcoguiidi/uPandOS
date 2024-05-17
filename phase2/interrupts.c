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


void interruptHandler(){
    
    unsigned int cause = getCAUSE();
    while(1){ // ciclo continuo dell'interrupt handler
        int line=1; // parto da 1 così ottengo subito la priorità maggiore
        while (line < 8){
            if (cause & CAUSE_IP(line)){
                break;
            }
            line++;
        }

        if (STATUS_IEc == 1 && STATUS_IM(line) == 1) {
            // TODO generates interrupt exception
        }

            if (line > 2){
                nonTimerInterrupt(line);
        }else if (line == 1){
            PLTinterrupt();
        }else{
            ITinterrupt();
        }
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
    switch (line + 3) {
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
    unsigned int mask = 1u << 5;
    devreg_t* devReg = devAddrBase;
    
    if (line == 7){
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
    }

    /* 4
    * send message to unblock caller pcb
    */
    pcb_t* unblocked = removeProcQ(&blocked_pcbs[calcBlockedQueueNo(line, devNo)]);
    msg_t* msg = allocMsg();
    if (msg == NULL) { // messaggi finiti
        // TODO
    }
    msg->m_payload = statusCode; // ? lo stato che ritorna il device
    msg->m_sender = ssi_pcb;
    pushMessage(&unblocked->msg_inbox, msg);
    // TODO

    /* 5
    * place saved status code in unblocked pcb's v0 register
    */
    unblocked->p_s.reg_v0 = statusCode;

    /* 6
    * insert newly unblocked pcb in ready queue
    */
    insertProcQ(&ready_queue, unblocked);
    soft_block_count--;

    /* 7
     * return control to current process
     */
     LDST(BIOSDATAPAGE);

}

void PLTinterrupt(){

    /* 1
    * Acknowledge PLT
    */
    setTIMER(0x00000000);

    /* 2
    * copy processor state in current process
    */
    state_t *saved = BIOSDATAPAGE;
    current_process->p_s = *saved;

    /* 3
    * place current process in ready queue
    */
    insertProcQ(&ready_queue, current_process);

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
    while ((pcb_to_unblock = removeProcQ(&blocked_pcbs[BOLCKEDPSEUDOCLOCK]))) {
        insertProcQ(&ready_queue, pcb_to_unblock);
    }
    
    /* 3
    * return control to current process
    */
    LDST(BIOSDATAPAGE);
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
}