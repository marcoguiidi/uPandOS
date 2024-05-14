#ifndef MISC_H
#define MISC_H

#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"

#define IN_KERNEL_MODE(status)  (!((status) & STATUS_KUc))

pcb_t* get_pid_in_list(unsigned int pid, struct list_head* list);

int is_pid_in_list(unsigned int pid, struct list_head* list);

void memory_copy(void* src, void* dst, unsigned int len);

void copy_state_t(state_t* src, state_t* dest);

void process_kill(pcb_t*  process);

unsigned int new_pid();

void process_spawn(pcb_t* process);

void terminateprocess(pcb_t* process);

pcb_t* pid_to_pcb(unsigned int pid);

int isInPcbFree_h(unsigned int pid);

int isInBlocked_pcbs(unsigned int pid);

pcb_t* getBlocked_pcbs(unsigned int pid);

#endif