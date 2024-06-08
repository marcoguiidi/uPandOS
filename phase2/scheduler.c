/*
scheduler.c This module implements the Scheduler and the deadlock detector.
*/
#include "./headers/scheduler.h"
#include "../phase1/headers/pcb.h"
#include "./headers/initial.h"

#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"
#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/types.h"
#include "../klog.h"
#include "headers/misc.h"


void scheduler() {
    //verifica se la coda dei processi è vuota
  	if (emptyProcQ(&ready_queue) == TRUE) {
		/*
		current process is always NULL, if the ssi is the only one and has 
		finished his work is blocked waiting for receiving a message
		*/
        //caso in cui l’unico processo è SSI
    	if (process_count == 1 && is_pid_in_list(ssi_pcb->p_pid, &blocked_pcbs[BLOKEDRECV])) {
      		HALT(); // Normal Termination
    	} else if (process_count > 1 && soft_block_count > 0) {
            //attraverso lo stato del registro abilito gli interrupts e disabilito il PLT
      		setSTATUS((getSTATUS() | STATUS_IEc | STATUS_IM_MASK) & (~STATUS_TE));
      		WAIT();//il processore non esegue istruzioni in attesa che si verifichi un interrupt 
    	}
    	// deadlock
    	else if (process_count > 0 && soft_block_count == 0) {
      		PANIC(); //Panic Termination
    	}
  	} else {
        //rimuovo il PCB dalla coda dei processi pronti 
    	current_process = removeProcQ(&ready_queue);
    	setTIMER(TIMESLICE);
    	STCK(acc_cpu_time); // restart counting
    	LDST(&current_process->p_s);//carica lo stato del processore corrente
  	}
}