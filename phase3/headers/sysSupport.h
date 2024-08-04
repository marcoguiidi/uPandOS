#ifndef SYS_SUPPORT_H
#define SYS_SUPPORT_H

#include "../../headers/types.h"

void support_syscall_exception_handler();

void support_trap_exception_handler();

void support_general_exception_handler();


void debung_program_running(pcb_PTR pcb);

#endif