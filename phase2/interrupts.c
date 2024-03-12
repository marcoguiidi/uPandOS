/*
interrupts.c This module implements the device/timer interrupt exception handler. This
module will process all the device/timer interrupts, converting device/timer interrupts into ap-
propriate messages for the blocked PCBs.
*/

#include "./headers/interrupts.h"
#include <umps3/umps/libumps.h>


void interruptHandler(){
    
    while(1){ // ciclo continuo dell'interrupt handler
        int line=1; // parto da 1 così ottengo subito la priorità maggiore
        while (line < 8){
            if (CAUSE_IP(line)){
                break;
            }
            line++;
        }

        if (STATUS_IEc == 1 && STATUS_IM(line) == 1) {
            // TODO generates interrupt exception
        }

            if (line > 2){
                nonTimerInterrupt(line);
        }else{
            timerInterrupt(line);
        }
    }
    
}



void nonTimerInterrupt(int line){

    /* 1
    * calculate address of this device device's register
    */
    unsigned int devAddrBase;
    int devNo; //TODO
    devAddrBase = 0x10000054 + (line * 0x80) + (devNo * 0x10);

    /* 2
    * save off status code
    */
    unsigned int statusCode;
    devreg_t* devReg = devAddrBase;
    if (line == 7){
        statusCode = devReg->term.transm_status;
    }else{
        statusCode = devReg->dtp.status;
    }

    /* 3
    * acknowledge the outstanding interrupt
    */
    if (line == 7){
        devReg->term.transm_command = ACK;
    }else{
        devReg->dtp.command = ACK;
    }

}