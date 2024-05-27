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
#include "headers/p2test.h"

unsigned int times = 0;

void scheduler() {
  	if (emptyProcQ(&ready_queue) == TRUE) {
		/*
		current process is always NULL, if the ssi is the only one and has 
		finished his work is blocked waiting for receiving a message
		*/
    	if (process_count == 1 && is_pid_in_list(ssi_pcb->p_pid, &blocked_pcbs[BLOKEDRECV])) {
      		HALT();
    	} else if (process_count > 1 && soft_block_count > 0) {
      		setSTATUS((getSTATUS() | STATUS_IEc | STATUS_IM_MASK) & (~STATUS_TE));
			times++;
			
			if (times == 50) {
				deadlock_logs();
				KLOG_PANIC("waiting for too long");
			}

      		WAIT();
    	}
    	// deadlock
    	else if (process_count > 0 && soft_block_count == 0) {
      		deadlock_logs();
			KLOG_PANIC("DEADLOCK");
    	}
  	} else {
		times = 0;
    	current_process = removeProcQ(&ready_queue);
    	setTIMER(TIMESLICE);
    	STCK(acc_cpu_time); // restart counting
    	LDST(&current_process->p_s);
  	}
}