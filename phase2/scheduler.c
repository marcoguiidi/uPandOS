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

void deadlock_logs() {
  	pcb_t* tmp;

	for (int i = 0; i < BLOCKED_QUEUE_NUM; i++) {
		if (i == BLOKEDRECV) {
			klog_print("waiting for a message\n");
		} else if (i == BOLCKEDPSEUDOCLOCK) {
			klog_print("waiting for timer\n");
		} else {
			klog_print("waiting for a device n. ");
			klog_print_dec(i);
			klog_print("\n");
		}
		list_for_each_entry(tmp, &blocked_pcbs[i], p_list) {
			if (tmp == test_pcb) {klog_print("test_pcb\n");}
			else if (tmp == print_pcb ) {klog_print("print_pcb\n");}
			else if (tmp == p2_pcb) {klog_print("p2_pcb\n");}
			else if (tmp == p3_pcb) {klog_print("p3_pcb\n");}
			else if (tmp == p4_pcb_v1 ) {klog_print("p4_pcb_v1\n");}
			else if (tmp == p4_pcb_v2 ) {klog_print("p4_pcb_v2\n");}
			else if (tmp == p5_pcb) {klog_print("p5_pcb\n");}
			else if (tmp == p6_pcb) {klog_print("p6_pcb\n");}
			else if (tmp == p7_pcb) {klog_print("p7_pcb\n");}
			else if (tmp == p8_pcb) {klog_print("p8_pcb\n");}
			else if (tmp == p8root_pcb) {klog_print("p8root_pcb\n");}
			else if (tmp == child1_pcb) {klog_print("child1_pcb\n");}
			else if (tmp == child2_pcb) {klog_print("child2_pcb\n");}
			else if (tmp == gchild1_pcb) {klog_print("gchild1_pcb\n");}
			else if (tmp == gchild2_pcb) {klog_print("gchild2_pcb\n");}
			else if (tmp == gchild3_pcb) {klog_print("gchild3_pcb\n");}
			else if (tmp == gchild4_pcb) {klog_print("gchild4_pcb\n");}
			else if (tmp == p9_pcb) {klog_print("p9_pcb\n");}
			else if (tmp == p10_pcb) {klog_print("p10_pcb\n");}
			else {
			    klog_print("**\n");
			}
		}
	}
}


volatile unsigned int times = 0;

void scheduler() {
	
	if (current_process != NULL) {
    	current_process->p_time += get_elapsed_time();
  	}

  	if (emptyProcQ(&ready_queue) == TRUE) {
    	if (process_count == 1 && current_process->p_pid == ssi_pcb->p_pid) {
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