#ifndef MISC_H
#define MISC_H

#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"

int IN_KERNEL_MODE(unsigned int status);

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

/*
* return the blocked queue number associated at the device/interruptline
*/
int calcBlockedQueueNo(int interruptline, int devno);

#endif