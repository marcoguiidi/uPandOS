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

void scheduler() {
  if (emptyProcQ(&ready_queue) == TRUE) {
    if (process_count == 1 && current_process->p_pid == ssi_pcb->p_pid) {
      HALT();
    } else if (process_count > 1 && soft_block_count > 0) {
      setSTATUS((getSTATUS() | STATUS_IEc | STATUS_IM_MASK) & (~STATUS_TE));
      WAIT();
    }
    // deadlock
    else if (process_count > 0 && soft_block_count == 0) {
      PANIC();
    }
  } else {
    current_process = removeProcQ(&ready_queue);
    setTIMER(TIMESLICE);
    LDST(&current_process->p_s);
  }
}
