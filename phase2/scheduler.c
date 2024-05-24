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

  if (current_process != NULL) {
    current_process->p_time += get_elapsed_time();
  }

  if (emptyProcQ(&ready_queue) == TRUE) {
    if (process_count == 1 && current_process->p_pid == ssi_pcb->p_pid) {
      HALT();
    } else if (process_count > 1 && soft_block_count > 0) {
      setSTATUS((getSTATUS() | STATUS_IEc | STATUS_IM_MASK) & (~STATUS_TE));
      WAIT();
    }
    // deadlock
    else if (process_count > 0 && soft_block_count == 0) {
      KLOG_PANIC("DEADLOCK");
    }
  } else {
    current_process = removeProcQ(&ready_queue);
    setTIMER(TIMESLICE);
    STCK(acc_cpu_time); // restart counting
    LDST(&current_process->p_s);
  }
}
