/**
sysSupport.c: This module implements the Support Level’s:
- General exception handler [Section 7].
- SYSCALL exception handler [Section 8].
- Program Trap exception handler [Section 9].
*/

#include "./headers/sysSupport.h"
#include "./headers/misc.h"
#include <umps3/umps/types.h>
#include <umps3/umps/const.h>
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/cp0.h>

void support_general_exception_handler() {
    state_t* exception_state = (state_t*)BIOSDATAPAGE; // ??
    unsigned int cause  = exception_state->cause;
    unsigned int ExcCode = CAUSE_GET_EXCCODE(cause);

    if (ExcCode >= 1) { // ?
        syscall_exception_handler(ExcCode);
    } else {
        trap_exception_handler(ExcCode);
    }
}

void syscall_exception_handler(unsigned int ExcCode) {

}

void trap_exception_handler(unsigned int ExcCode) {

}

void 