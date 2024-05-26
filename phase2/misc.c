#include "headers/misc.h"
#include "headers/initial.h"
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include "../phase1/headers/msg.h"
#include "../phase1/headers/pcb.h"
#include "../klog.h"
#include "headers/p2test.h"


int IN_KERNEL_MODE(unsigned int status) {
    if ((status & STATUS_KUp) == 0) 
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

void process_kill(pcb_t *process) {
    if (process == ssi_pcb) {
        KLOG_PANIC("someone is murdering the ssi :(");
    }
    if (process == current_process) {
        KLOG_ERROR("?? killing current process");
        current_process = NULL;
    } else if (out_pcb_in_all(process) == NULL) {
        KLOG_PANIC("pcb not found");
    }
    process_count--;
    
    freePcb(process);
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

int is_pcb_in_list(pcb_t* pcb, struct list_head* list) {
    pcb_t* tmp;
    list_for_each_entry(tmp, list, p_list) {
        if (tmp == pcb)
            return 1;
    }
    return 0;
}

int is_in_pcbfee_sas(pcb_t* process) {
    return is_pcb_in_list(process, &pcbFree_h);
}

int calcBlockedQueueNo(int interruptline, int devno) {
    return ((interruptline-3)*8) + devno;
}

cpu_t get_elapsed_time() {
    cpu_t time_now_TOD;
    STCK(time_now_TOD);
    return (time_now_TOD - acc_cpu_time);
}


void klogprint_current_pcb_name() {
    if (current_process == test_pcb) {klog_print("test_pcb! ");}
    else if (current_process == print_pcb ) {klog_print("print_pcb! ");}
    else if (current_process == p2_pcb) {klog_print("p2_pcb! ");}
    else if (current_process == p3_pcb) {klog_print("p3_pcb! ");}
    else if (current_process == p4_pcb_v1 ) {klog_print("p4_pcb_v1! ");}
    else if (current_process == p4_pcb_v2 ) {klog_print("p4_pcb_v2! ");}
    else if (current_process == p5_pcb) {klog_print("p5_pcb! ");}
    else if (current_process == p6_pcb) {klog_print("p6_pcb! ");}
    else if (current_process == p7_pcb) {klog_print("p7_pcb! ");}
    else if (current_process == p8_pcb) {klog_print("p8_pcb! ");}
    else if (current_process == p8root_pcb) {klog_print("p8root_pcb! ");}
    else if (current_process == child1_pcb) {klog_print("child1_pcb! ");}
    else if (current_process == child2_pcb) {klog_print("child2_pcb! ");}
    else if (current_process == gchild1_pcb) {klog_print("gchild1_pcb! ");}
    else if (current_process == gchild2_pcb) {klog_print("gchild2_pcb! ");}
    else if (current_process == gchild3_pcb) {klog_print("gchild3_pcb! ");}
    else if (current_process == gchild4_pcb) {klog_print("gchild4_pcb! ");}
    else if (current_process == p9_pcb) {klog_print("p9_pcb! ");}
    else if (current_process == p10_pcb) {klog_print("p10_pcb! ");}
    else {
        klog_print("unknown! ");
    }
}
