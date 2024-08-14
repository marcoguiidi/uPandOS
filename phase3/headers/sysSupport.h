#ifndef SYS_SUPPORT_H
#define SYS_SUPPORT_H

#include "../../headers/types.h"

/**
if a U-proc executes a SYSCALL instruction and a0 contained a valid positive value
then the Support Level should perform one of the services described below.

SendMsg (USYS1)
ReceiveMsg (USYS2)
 */
void support_syscall_exception_handler();

/**
The Support Level’s Program Trap exception handler is to terminate the process in an orderly
fashion; perform the same operations as a SYS2 request.
Important: If the process to be terminated is currently holding mutual exclusion on a Support
Level structure (e.g. Swap Pool table), mutual exclusion must first be released (send a message) before
request the process termination.
 */
void support_trap_exception_handler();


/**
The Support Level general exception handler will process all passed up non-TLB exceptions:

All SYSCALL (SYSCALL) exceptions numbered 1 and above (positive number).
All Program Trap exceptions; all exception causes exclusive of those for SYSCALL exceptions
and those related to TLB exceptions.
 */
void support_general_exception_handler();

#endif