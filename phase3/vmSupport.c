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

swap_t swap_pool[POOLSIZE];

void initSwapStructs(void) {
    for (int i = 0; i < POOLSIZE; i++) {
        swap_pool[i].sw_asid = NOPROC;
    }
}

// 5.4
unsigned int pageReplacementAlgorithm(void) {
    static unsigned int index = 0; // static variable inside a function keeps its value between invocations
    
    index = (index + 1) % POOLSIZE;
    return index;
}

int isSwapPoolFrameOccupied(unsigned int framenum) {
    if (swap_pool[framenum].sw_asid != NOPROC) return TRUE;
    else return FALSE;
}

unsigned int flashDevWrite(memaddr* write_address, memaddr* flash_dev_address) {
    unsigned int command = (unsigned int)((unsigned int)write_address << 7) | FLASHWRITE;
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

unsigned int writeFrameToFlashDev(unsigned int frame_number, memaddr *dest_address) {
    // calculate right flash dev
    memaddr* dev_addr = (memaddr*)(DEV_REG_ADDR(IL_FLASH, swap_pool[frame_number].sw_asid));
    
    // load start ram address to write in flash
    devreg_t* devReg =  (devreg_t*)dev_addr;
    devReg->dtp.data0 = swap_pool[frame_number].sw_pageNo * PAGESIZE; // ?? bho
    return flashDevWrite(dest_address, dev_addr);
}

unsigned int flashDevRead(memaddr *read_address, memaddr *flash_dev_address) {
    unsigned int status = 0;
    unsigned int value = (unsigned int)((unsigned int)read_address << 7) | FLASHREAD;

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

unsigned readFrameToFlashDev(memaddr *missing_page_address, pteEntry_t *page) {
    // calculate right flash dev
    memaddr* dev_addr = (memaddr*)(DEV_REG_ADDR(IL_FLASH, (page->pte_entryHI >> ASIDSHIFT) & 7));
    
    // load start ram address to write to flash
    devreg_t* devReg =  (devreg_t*)dev_addr;
    devReg->dtp.data0 = ((page->pte_entryHI & GETPAGENO) >> VPNSHIFT) * PAGESIZE; // ?? bho
    return flashDevWrite(missing_page_address, dev_addr);
}

void updateTLB(void) {
    TLBCLR(); // TODO: 5.2 update tlb when all debugged
}

void pager(void) {
    unsigned int saved_status;

    // 1 get support data
    support_t* support_data = get_support_data();

    // 2 determine the cause of the TLB exception
    state_t* exceptstate = &(support_data->sup_exceptState[0]);

    // 3 
    if (exceptstate->cause == TLBMOD) TrapExceptionHandler(exceptstate);

    // 4
    gainSwap();

    // 5 Determine the missing page number (p)
    unsigned int missing_page_num = (exceptstate->entry_hi & GETPAGENO) >> VPNSHIFT;

    // 6 pick a frame i from the swap pool..
    unsigned int frame_victim_num = pageReplacementAlgorithm();
    memaddr* swap_pool_start = (memaddr*)0x20020000;
    memaddr* frame_victim_address = swap_pool_start + (frame_victim_num * PAGESIZE);

    // 7 determine if frame i is occupied
    if (isSwapPoolFrameOccupied(frame_victim_num)) {
        // 8
        // ATOMIC start
        saved_status = getSTATUS();
        setSTATUS(saved_status & ~IECON); // disable interrupts

        int belogns_to_ASID = swap_pool[frame_victim_num].sw_asid;

        // (a)
        swap_pool[frame_victim_num].sw_pte->pte_entryLO &= !VALIDON;

        // (b)
        updateTLB();

        // (d)
        unsigned int write_status = writeFrameToFlashDev(frame_victim_num, frame_victim_address);
        if (write_status != 3 && write_status != 1) { // if is not ready or busy
            // trap
            TrapExceptionHandler(write_status); // ??
        }
    }

    // 9
    unsigned int read_status = readFrameToFlashDev(frame_victim_address, &support_data->sup_privatePgTbl[missing_page_num]);
    if (read_status != 3 && read_status != 1) { // if is not ready or busy
        // trap
        TrapExceptionHandler(read_status); // ??
    }

    // 10 update swap pool page table
    swap_pool[frame_victim_num].sw_asid     = support_data->sup_asid;
    swap_pool[frame_victim_num].sw_pageNo   = missing_page_num;
    swap_pool[frame_victim_num].sw_pte      = &support_data->sup_privatePgTbl[missing_page_num];

    // 11 update the current process's page table entry
    support_data->sup_privatePgTbl[missing_page_num].pte_entryLO = ((unsigned int)frame_victim_address << VPNSHIFT) | DIRTYON | VALIDON;

    // 12 upadte the tlb
    updateTLB();

    // 13
    setSTATUS(saved_status);
    // ATOMIC end

    // 14 return control to che current process
    LDST(exceptstate);
}
