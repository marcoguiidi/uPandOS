#include "headers/misc.h"
#include "headers/initial.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include "../phase1/headers/msg.h"
#include "../phase1/headers/pcb.h"


int IN_KERNEL_MODE(unsigned int status) {
    if ((status & STATUS_KUc) == 0) 
        return TRUE;
    else
        return FALSE;
}

pcb_t* get_pid_in_list(unsigned int pid, struct list_head* list) {
    pcb_t* tmp;
    list_for_each_entry(tmp, list, p_list) {
        if (tmp->p_pid == pid)
            return tmp;
    }
    return NULL; // non ci deve mai andare
}

int is_pid_in_list(unsigned int pid, struct list_head* list) {
    pcb_t* tmp;
    list_for_each_entry(tmp, list, p_list) {
        if (tmp->p_pid == pid)
            return 1;
    }
    return 0; // non ci deve mai andare
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

void process_kill(pcb_t *process) {
    if (outProcQ(&ready_queue, process) == NULL) { //not in ready queue
        // check blocked pbc
        for (int i = 0; i < SEMDEVLEN; i++) {
            if (outProcQ(&blocked_pcbs[i], process) != NULL) {
                soft_block_count--;
                break; // found it
            }
        }
    } else {
        process_count--;
    }
    freePcb(process);
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

void terminateprocess(pcb_t* process) {
    while (!emptyChild(process)) {
        pcb_t* child = removeChild(process);
        terminateprocess(child);
    }
    process_kill(process);
}

int isInPcbFree_h(unsigned int pid) {
    return is_pid_in_list(pid, &pcbFree_h);
}

pcb_t* out_pcb_in_all(pcb_t* pcb) {
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

int calcBlockedQueueNo(int interruptline, int devno) {
    return ((interruptline-3)*8) + devno;
}