/**
vmSupport.c: This module implements:
- the TLB exception handler (The Pager).
Since reading and writing to each U-proc’s flash device is limited to supporting paging, 
- this module should also contain the function(s) for reading and writing flash devices. TODO: do ssi part
Additionally, the Swap Pool table is local to this module. Instead of declaring them globally in initProc.c they can be
declared module-wide in vmSupport.c. The test function will now invoke a new “public” function
- initSwapStructs which will do the work of initializing the Swap Pool table. [x]
Technical Point: Since the code for the TLB-Refill event handler was replaced (without
relocating the function), uTLB_RefillHandler should still be found in the Level 3/Phase 2
exceptions.c file.
*/

#include "../headers/types.h"
#include "../headers/const.h"
#include "./headers/vmSupport.h"
#include "./headers/misc.h"
#include "../phase2/headers/exceptions.h"
#include "headers/initProc.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>
#include "../klog.h"

swap_t swap_pool_table[POOLSIZE];

void initSwapStruct(void) {
    for (int i = 0; i < POOLSIZE; i++) {
        swap_pool_table[i].sw_asid = NOPROC;
    }
}

// 5.4
unsigned int pageReplacementAlgorithm(void) {
    static unsigned int index = 0; // static variable inside a function keeps its value between invocations
    
    index = (index + 1) % POOLSIZE;
    return index;
}

int isSwapPoolFrameOccupied(unsigned int framenum) {
    if (swap_pool_table[framenum].sw_asid != NOPROC) return TRUE;
    else return FALSE;
}

unsigned int flashDevWrite(unsigned int block_to_write, memaddr *flash_dev_address) {
    unsigned int command = (unsigned int)((unsigned int)block_to_write << 8) | FLASHWRITE;
    unsigned status = 0;
    ssi_do_io_t do_io = {
        .commandAddr = flash_dev_address,
        .commandValue = command,
    };
    ssi_payload_t payload = {
        .service_code = DOIO,
        .arg = &do_io,
    };

    SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
    SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&status), 0);
    return status;
}

unsigned int writeFrameToFlashDev(unsigned int asid, memaddr mem_addr_to_read, unsigned int block_to_write) {
    // calculate right flash dev
    memaddr* dev_addr = (memaddr*)(DEV_REG_ADDR(IL_FLASH, asid));
    
    // load start ram address to write to flash
    devreg_t* devReg =  (devreg_t*)dev_addr;

    devReg->dtp.data0 = mem_addr_to_read; // ?? bho
    return flashDevWrite(block_to_write, &devReg->dtp.command);
}

unsigned int flashDevRead(unsigned int block_to_read, memaddr *flash_dev_address) {
    unsigned int status = 0;
    unsigned int value = (unsigned int)((unsigned int)block_to_read << 8) | FLASHREAD;

  ssi_do_io_t do_io = {
      .commandAddr = flash_dev_address,
      .commandValue = value,
  };
  ssi_payload_t payload = {
      .service_code = DOIO,
      .arg = &do_io,
  };
  SYSCALL(SENDMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&payload), 0);
  SYSCALL(RECEIVEMESSAGE, (unsigned int)ssi_pcb, (unsigned int)(&status), 0);
  return status;
}

unsigned int readFrameToFlashDev(unsigned int asid, memaddr mem_addr_to_write, unsigned int block_to_read) {
    // calculate right flash dev
    memaddr* dev_addr = (memaddr*)(DEV_REG_ADDR(IL_FLASH, asid));
    
    // load start ram address to write to flash
    devreg_t* devReg =  (devreg_t*)dev_addr;

    devReg->dtp.data0 = mem_addr_to_write; // ?? bho
    return flashDevRead(block_to_read, &devReg->dtp.command);
}

void updateTLB(void) {
    TLBCLR(); // TODO: 5.2 update tlb when all debugged
}

void flash_status_debug(unsigned int status) {
    switch (status) {
        case 0 :{
            KLOG_ERROR("Device Not Installed")
            break;
        }
        case 1 :{
            KLOG_ERROR("Device Ready")
            break;
        }
        case 2 :{
            KLOG_ERROR("Illegal Operation Code Error")
            break;
        }
        case 3 :{
            KLOG_ERROR("Device Busy")
            break;
        }
        case 4 :{
            KLOG_ERROR("Read Error")
            break;
        }
        case 5 :{
            KLOG_ERROR("Write Error")
            break;
        }
        case 6 :{
            KLOG_ERROR("DMA Transfer Error")
            break;
        }
        default: {
            KLOG_ERROR("status?")
        }
    }
}

void pager(void) {
    //klog_print("[entry not valid -> pager]");
    unsigned int saved_status;
    
    // 1 get support data
    support_t* support_data = get_support_data();

    // 2 determine the cause of the TLB exception
    state_t* exceptstate = &(support_data->sup_exceptState[PGFAULTEXCEPT]);
    
    // 3 If the Cause is a TLB-Modification exception, treat this exception as a program trap
    unsigned int ExcCode = CAUSE_GET_EXCCODE(exceptstate->cause) ;
    if (ExcCode == TLBMOD) {
        KLOG_PANIC("not TLBMOD allowed")
        TrapExceptionHandler(exceptstate);
    }

    // 4 Gain mutual exclusion over the Swap Pool table
    gainSwap();
    
    // 5 Determine the missing page number (p)
    
    unsigned int missing_page_num = (exceptstate->entry_hi & GETPAGENO) >> VPNSHIFT;
    if (missing_page_num > USERPGTBLSIZE) {
        missing_page_num = USERPGTBLSIZE -1;
    }
    
    // 6 pick a frame i from the swap pool..
    unsigned int frame_victim_num = pageReplacementAlgorithm();
    
    // swap pool ???
    memaddr swap_pool_start = (memaddr)0x20020000;
    memaddr frame_victim_address = swap_pool_start + (frame_victim_num * PAGESIZE);
    unsigned int frame_victim = frame_victim_address >> 12;
    
    //klog_print("[missing page num ");
    //klog_print_dec(missing_page_num);
    //klog_print(" frame victim num ");
    //klog_print_dec(frame_victim_num);
    //klog_print(" frame victim addr ");
    //klog_print_hex(frame_victim_address);
    //klog_print(" frame victim ");
    //klog_print_hex(frame_victim);
    //klog_print("]");

    // 7 determine if frame i is occupied
    if (isSwapPoolFrameOccupied(frame_victim_num)) {
        KLOG_ERROR("warnzone")
        // 8
        // ATOMIC start
        saved_status = getSTATUS();
        setSTATUS(saved_status & ~IECON); // disable interrupts

        // (a) Update process x’s Page Table: mark Page Table entry k as not valid
        swap_pool_table[frame_victim_num].sw_pte->pte_entryLO &= !VALIDON;

        // (b) Update the TLB, if needed
        updateTLB();

        setSTATUS(saved_status);
        // ATOMIC end

        // (c) Update process x’s backing store. Write the contents of frame i to the correct location on
        //     process x’s backing store/flash device
        unsigned int write_status = writeFrameToFlashDev(swap_pool_table[frame_victim_num].sw_asid, frame_victim_address, frame_victim_num);
        if (write_status != READY) { // if is not ready or busy
            flash_status_debug(write_status);
            // trap
            //TrapExceptionHandler((unsigned int)read_status); // ??
            KLOG_PANIC("flash writr not succesful")
        }
    }
    
    // 9 Read the contents of the Current Process’s backing store/flash device logical page p into frame i 
    unsigned int block_to_read = missing_page_num;
    if (block_to_read < 0 || block_to_read > 32) {
        KLOG_PANIC("BLOCK ERROR")
    }
    unsigned int read_status = readFrameToFlashDev(support_data->sup_asid, frame_victim_address, block_to_read);
    if (read_status != READY) { // if is not ready or busy
        flash_status_debug(read_status);
        // trap
        //TrapExceptionHandler((unsigned int)read_status); // ??
        KLOG_PANIC("flash read not succesful")
    }

    // 10 update swap pool page table
    swap_pool_table[frame_victim_num].sw_asid     = support_data->sup_asid;
    swap_pool_table[frame_victim_num].sw_pageNo   = missing_page_num;
    swap_pool_table[frame_victim_num].sw_pte      = &support_data->sup_privatePgTbl[missing_page_num];

    // ATOMIC start
    saved_status = getSTATUS();
    setSTATUS(saved_status & ~IECON); // disable interrupts

    // 11 update the current process's page table entry
    support_data->sup_privatePgTbl[missing_page_num].pte_entryLO = ((unsigned int)frame_victim << VPNSHIFT) | DIRTYON | VALIDON;

    // 12 upadte the tlb
    updateTLB();

    setSTATUS(saved_status);
    // ATOMIC end

    // 13
    releaseSwap();
    
    // 14 return control to the current process
    LDST(exceptstate);
}
