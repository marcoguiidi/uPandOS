#ifndef MISC_H
#define MISC_H

#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"

/*
return true if the process was in kernel mode
before an exception jump, fase otherwise
*/
int in_kernel_mode(unsigned int status);

/*
copy len byte of memory from src to dst
*/
void memory_copy(void* src, void* dst, unsigned int len);

/*
copy state_t type from src to dest
*/
void copy_state_t(state_t* src, state_t* dest);


/*
get a new unique process id identifier
Warning: overflow
*/
unsigned int new_pid();

/*
insert process in the ready queue and increase process_count
*/
void process_spawn(pcb_t* process);

/*
return TRUE if exists a process with its pid == pid in list
*/
int is_pid_in_list(unsigned int pid, struct list_head* list);

/*
retrun TRUE if exists a process with its pid == pid in pcbFree_h
*/
int isInPcbFree_h(unsigned int pid);

/*
remove a process from all places (current_process, ready_queue, blocked_pcbs[])
if is in blocked_pcbs[] blocked_pcbs--;
*/
pcb_t* out_pcb_in_all(pcb_t* pcb);

/*
return the blocked queue number associated at the device/interruptline
*/
int calcBlockedQueueNo(int interruptline, int devno);

/*
get time elapsed from TOD forom last STCK(acc_cpu_time)
*/
cpu_t get_elapsed_time();

/*
kill a process and it's progeny
*/
void process_killall(pcb_t *process);

#endif