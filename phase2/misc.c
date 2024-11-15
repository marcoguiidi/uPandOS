#include "headers/misc.h"
#include "headers/initial.h"
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include "../phase1/headers/msg.h"
#include "../phase1/headers/pcb.h"
#include "../klog.h"

int in_kernel_mode(unsigned int status) {
    if ((status & STATUS_KUp) == 0) // the previous kernel mode bit flag
        return TRUE;
    else
        return FALSE;
}

int is_pid_in_list(unsigned int pid, struct list_head* list) {
    pcb_t* tmp;
    list_for_each_entry(tmp, list, p_list) {
        if (tmp->p_pid == pid)
            return TRUE;
    }
    return FALSE;
}

void memory_copy(void* src, void* dst, unsigned int len) {
    char* csrc = (char*)src;
    char* cdst = (char*)dst;
    for (unsigned int i = 0; i < len; i++) {
        cdst[i] = csrc[i];
    }
}

void copy_state_t(state_t* src, state_t* dest) {
    dest->entry_hi  = src->entry_hi;
	dest->cause     = src->cause;
	dest->status    = src->status;
	dest->pc_epc    = src->pc_epc;
	dest->hi        = src->hi;
	dest->lo        = src->lo;
    for (int i = 0; i < STATE_GPR_LEN; i++) {
        dest->gpr[i] = src->gpr[i];
    }
}

unsigned int new_pid() {
    unsigned int new_pid = lastpid;
    lastpid++;
    return new_pid;
}

void process_spawn(pcb_t *process) {
    process->p_pid = new_pid();
    insertProcQ(&ready_queue, process);
    process_count++;
}

pcb_t* out_pcb_in_all(pcb_t* pcb) {
    if (pcb == current_process) {
        current_process = NULL;
        return pcb;
    }
    pcb_t* retpcb = NULL;
    if ((retpcb = outProcQ(&ready_queue, pcb)) == NULL) {
        for (int i = 0; i < BLOCKED_QUEUE_NUM; i++) {
            if ((retpcb = outProcQ(&blocked_pcbs[i], pcb)) != NULL) {
                soft_block_count--;
                break;
            }
        }
    }
    return retpcb;
}

void process_killall(pcb_t *process) {
    if (process == NULL || isInPcbFree_h(process->p_pid)) {
        return;
    }

    outChild(process); // detach from parent, now parent have one less children

    pcb_t* child;
    while (!emptyChild(process)) {
        child = removeChild(process);
        process_killall(child);
    }

    if (out_pcb_in_all(process) == NULL) { // extract from all queues, cannot run again
        KLOG_PANIC("pcb not found");
    }

    freePcb(process); // pcb is now reusable
    process_count--;
}

int isInPcbFree_h(unsigned int pid) {
    return is_pid_in_list(pid, &pcbFree_h);
}

int calcBlockedQueueNo(int interruptline, int devno) {
    return ((interruptline-3)*8) + devno;
}

cpu_t get_elapsed_time() {
    cpu_t time_now_TOD;
    STCK(time_now_TOD);
    return (time_now_TOD - acc_cpu_time);
}