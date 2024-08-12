/**
sst.c: This module implements the System Service Thread:
- Initialize the corresponding U-proc [Section 10.1].
- Wait for service requests and manage them [Section 6].
*/
#include "../headers/const.h"
#include "../headers/types.h"
#include "../klog.h"
#include <umps3/umps/types.h>
#include <umps3/umps/const.h>
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/cp0.h>

#include "headers/initProc.h"
#include "headers/misc.h"
#include "../phase2/headers/initial.h"

int printer_write_string(int lenght, char* string, devreg_t* printer_base_addr) {
    int ret = 0; // no errors
    unsigned int status;
    for (int i = 0; i < lenght; i++) {
        // place the character to print in DATA0
        printer_base_addr->dtp.data0 = string[i];

        ssi_do_io_t do_io = {
            .commandAddr = (memaddr*)(&printer_base_addr->dtp.command),
            .commandValue = PRINTCHR,
        };
        ssi_payload_t payload = {
            .service_code = DOIO,
            .arg = &do_io,
        };
        SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
        SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&status), 0);

        if (status != READY) {
            ret = -1;
            break;
        }
    }
    return ret;
}

int terminal_write_string(int lenght, char* string, devreg_t* terminal_base_addr) {
    int ret = 0; // no errors
    unsigned int *base = (unsigned int*)(terminal_base_addr);
    unsigned int *command = base + 3;
    unsigned int status;
    for (int i = 0; i < lenght; i++) {
        // prepare command value
        unsigned int value = PRINTCHR | (((unsigned int)string[i]) << 8);
        
        ssi_do_io_t do_io = {
            .commandAddr = command,
            .commandValue = value,
        };
        ssi_payload_t payload = {
            .service_code = DOIO,
            .arg = &do_io,
        };
        SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
        SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&status), 0);

        if ((status & 0xFF) != RECVD) { // character is not received
            ret = -1; // error
            break;
        }
    }
    return ret;
}

void SSTRequest(pcb_PTR sender, int service_code, void* arg) {
    switch (service_code) {
        case GET_TOD: {
            cpu_t time;
            STCK(time);
            SYSCALL(SENDMESSAGE, (unsigned int)sender, (unsigned int)(time), 0);
            break;
        }
        case TERMINATE: {
            KLOG_ERROR(" [uproc terminated] ")
            // send a message to test process to tell that one SST is killed 
            SYSCALL(SENDMESSAGE, (unsigned int)test_pcb, 0, 0); // TODO: this make kill all
            // kill sst and so his child
            kill_process(SELF);
            break;
        }
        case WRITEPRINTER: {
            sst_print_PTR print_payload = (sst_print_PTR)arg;
            // calculate the base addr of printer
            int line = sender->p_supportStruct->sup_asid;
            devreg_t* printer_base_addr = (devreg_t*)DEV_REG_ADDR(IL_PRINTER, line);
            int ret = printer_write_string(print_payload->length, print_payload->string, printer_base_addr);
            if (ret != 0) {
                KLOG_PANIC("write printer not succesful")
            }
            // send an empty respose to notify and of print
            SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
            break;
        }
        case WRITETERMINAL: {
            sst_print_PTR print_payload = (sst_print_PTR)arg;
            // calculate the base addr of printer
            int line = sender->p_supportStruct->sup_asid;
            devreg_t* terminal_base_addr = (devreg_t*)DEV_REG_ADDR(IL_TERMINAL, line);
            int ret = terminal_write_string(print_payload->length, print_payload->string, terminal_base_addr);
            if (ret != 0) {
                KLOG_PANIC("write terminal not succesful")
            }
            // send an empty respose to notify and of print
            SYSCALL(SENDMESSAGE, (unsigned int)sender, 0, 0);
            break;
        }
        default: {
            KLOG_PANIC("service code not found")
        }
    }
}

void SST_function_entry_point() {
    //child u-proc start and initialization
    // support data is the same
    support_t* support = get_support_data();

    int asid = support->sup_asid;
    pcb_PTR uproc = create_process(&state_t_pool[asid], &support_t_pool[asid]);
    uproc_pbc[asid] = uproc;

    // manage requests
    ssi_payload_t* payload;
    pcb_t* sender;
    while (TRUE) {
        // wait for a service request
        sender = (pcb_t*)SYSCALL(RECEIVEMESSAGE, ANYMESSAGE, (unsigned int)(&payload), 0);

        SSTRequest(sender, payload->service_code, payload->arg);
    }
}