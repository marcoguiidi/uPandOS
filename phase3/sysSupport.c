/**
sysSupport.c: This module implements the Support Level’s:
- General exception handler [Section 7].
- SYSCALL exception handler [Section 8].
- Program Trap exception handler [Section 9].
*/

#include "./headers/sysSupport.h"
#include "./headers/misc.h"
#include "../phase2/headers/misc.h"
#include "../phase2/headers/interrupts.h"
#include "../phase2/headers/scheduler.h"
#include "headers/initProc.h"
#include <umps3/umps/types.h>
#include <umps3/umps/const.h>
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/cp0.h>
#include "../klog.h"

// for debugging purposes
void debug_pcb_name(pcb_PTR pcb) {
    klog_print("is exec ");
    if (pcb == NULL) {
        klog_print("kernel or error\n");
        return;
    }
    if (pcb == ssi_pcb) {
        klog_print("ssi\n");
        return;
    }
    if (pcb == test_pcb) {
        klog_print("test\n");
        return;
    }
    if (pcb == swap_mutex) {
        klog_print("swap mutex\n");
        return;
    }
    for (int i = 0; i < UPROCMAX; i++) {
        if (pcb == sst_pcb[i]) {
            klog_print("sst[");
            klog_print_dec(i);
            klog_print("]\n");
            return;
        }
    }
    for (int i = 0; i < UPROCMAX; i++) {
        if (pcb == uproc_pbc[i]) {
            klog_print("uproc[");
            klog_print_dec(i);
            klog_print("]\n");
            return;
        }
    }
    klog_print("unknown\n");
}

// for debugging purposes
void debug_trap(unsigned int ExcCode) {
    switch (ExcCode) {
        case 0: {
            KLOG_ERROR("External Device Interrupt")
            break;
        }
        case 1: {
            KLOG_ERROR("TLB-Modification Exception")
            break;
        }
        case 2: {
            KLOG_ERROR("TLB Invalid Exception: on a Load instr. or instruction fetch")
            break;
        }
        case 3: {
            KLOG_ERROR("TLB Invalid Exception: on a Store instr.")
            break;
        }
        case 4: {
            KLOG_ERROR("Address Error Exception: on a Load or instruction fetch")
            break;
        }
        case 5: {
            KLOG_ERROR("Address Error Exception: on a Store instr.")
            break;
        }
        case 6: {
            KLOG_ERROR("Bus Error Exception: on an instruction fetch")
            break;
        }
        case 7: {
            KLOG_ERROR("Bus Error Exception: on a Load/Store data access")
            break;
        }
        case 8: {
            KLOG_ERROR("Syscall Exception")
            break;
        }
        case 9: {
            KLOG_ERROR("Breakpoint Exception")
            break;
        }
        case 10: {
            KLOG_ERROR("Reserved Instruction Exception")
            break;
        }
        case 11: {
            KLOG_ERROR("Coprocessor Unusable Exception")
            break;
        }
        case 12: {
            KLOG_ERROR("Arithmetic Overflow Exception")
            break;
        }
    }
}

void support_trap_exception_handler(support_t* support) {
    // debug
    state_t* state = &support->sup_exceptState[GENERALEXCEPT];
    unsigned int ExcCode = CAUSE_GET_EXCCODE(state->cause);
    debug_pcb_name(current_process);
    debug_trap(ExcCode);
    // send message to swap mutex
    releaseSwap();
    // kill process
    kill_process(SELF);
}

void support_general_exception_handler() {
    support_t* support = get_support_data();
    state_t* state = &support->sup_exceptState[GENERALEXCEPT];
    unsigned int ExcCode = CAUSE_GET_EXCCODE(state->cause);

    if (ExcCode == SYSEXCEPTION) {
        support_syscall_exception_handler(support);
    } else {
        support_trap_exception_handler(support);
    }

}

void support_syscall_exception_handler(support_t* support) {
    // get state register at time of syscall
    state_t* exceptionState = &support->sup_exceptState[GENERALEXCEPT];

    // syscall type
    int reg_A0 = exceptionState->reg_a0;
    // dest process
    unsigned int reg_A1 = exceptionState->reg_a1;
    // payload of the mex
    unsigned int reg_A2 = exceptionState->reg_a2;

    switch (reg_A0) {
    //perform a multi-way branching depending on the type of exception
        case SENDMSG: {
            if (reg_A1 == PARENT) reg_A1 = (unsigned int)sst_pcb[support->sup_asid];
            SYSCALL(SENDMESSAGE, reg_A1, reg_A2, 0);
            break;
        }
        case RECEIVEMSG: {
            SYSCALL(RECEIVEMESSAGE, reg_A1, reg_A2, 0);
            break;
        }
        default:
            // service code not valid, kill
            support_trap_exception_handler(support);
            break;
    }
    // return from non blocking
    exceptionState->pc_epc += WORDLEN; //pc updated
    /*move the PC to the next instruction, avoiding entering an 
    infinite loop that would repeat the same syscall*/
    LDST(exceptionState);
    //restores processor state
}