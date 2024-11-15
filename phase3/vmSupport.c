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
#include "headers/sysSupport.h"

swap_t swap_pool_table[POOLSIZE];

void initSwapStruct(void) {
    for (int i = 0; i < POOLSIZE; i++) {
        swap_pool_table[i].sw_asid = NOPROC;
    }
}

// 5.4
// Small Support Level Optimizations n.2
//Improve the μPandOS page replacement algorithm to first check for an unoccupied frame before
//selecting an occupied frame to use. This will turn an O(1) operation into an O(n) operation in
//exchange for fewer I/O (write) operations.
unsigned int pageReplacementAlgorithm(void) {
    static unsigned int index = 0; // static variable inside a function keeps its value between invocations
    
    for (int i = 0; i < POOLSIZE -1; i++) {
        index = (index + 1) % POOLSIZE;
        if (swap_pool_table[index].sw_asid == NOPROC) break;
    }
    
    return index;
}

// Small Support Level Optimizations n.1
//When a U-proc terminates, mark all of the frames it occupied as unoccupied [Section 4.1]. This
//has the potential to eliminate extraneous writes to the backing store.
void free_occupied_frames(unsigned int asid) {
    for (int i = 0; i < POOLSIZE; i++) {
        if (swap_pool_table[i].sw_asid == asid) swap_pool_table[i].sw_asid = NOPROC;
    }
}

int isSwapPoolFrameOccupied(unsigned int framenum) {
    if (swap_pool_table[framenum].sw_asid != NOPROC) return TRUE;
    else return FALSE;
}

unsigned int writeFrameToFlashDev(unsigned int asid, memaddr mem_addr_to_read, unsigned int block_to_write) {
    // calculate right flash dev
    memaddr* dev_addr = (memaddr*)(DEV_REG_ADDR(IL_FLASH, asid));
    devreg_t* devReg =  (devreg_t*)dev_addr;

    // load start ram address to write to flash
    devReg->dtp.data0 = mem_addr_to_read;
    
    // calculate right command
    unsigned int command = (unsigned int)((unsigned int)block_to_write << 8) | FLASHWRITE;
    unsigned status = 0;
    
    // request doio to ssi
    ssi_do_io_t do_io = {
        .commandAddr = &devReg->dtp.command,
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

unsigned int readFrameToFlashDev(unsigned int asid, memaddr mem_addr_to_write, unsigned int block_to_read) {
    // calculate right flash dev
    memaddr* dev_addr = (memaddr*)(DEV_REG_ADDR(IL_FLASH, asid));
    devreg_t* devReg =  (devreg_t*)dev_addr;

    // load start ram address to read from flash
    devReg->dtp.data0 = mem_addr_to_write;
    unsigned int status = 0;
    
    // calculate right command
    unsigned int value = (unsigned int)((unsigned int)block_to_read << 8) | FLASHREAD;

    // request doio to ssi
    ssi_do_io_t do_io = {
        .commandAddr = &devReg->dtp.command,
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
    unsigned int saved_status;
    
    // 1 get support data
    support_t* support_data = get_support_data();

    // 2 determine the cause of the TLB exception
    state_t* exceptstate = &(support_data->sup_exceptState[PGFAULTEXCEPT]);
    
    unsigned int ExcCode = CAUSE_GET_EXCCODE(exceptstate->cause) ;
    if (ExcCode == TLBMOD) { // 3 If the Cause is a TLB-Modification exception, treat this exception as a program trap
        TrapExceptionHandler(exceptstate);
    }

    // 4 Gain mutual exclusion over the Swap Pool table
    gainSwap();
    
    // 5 Determine the missing page number (p)
    unsigned int missing_page_num = (exceptstate->entry_hi & GETPAGENO) >> VPNSHIFT;
    if (missing_page_num > USERPGTBLSIZE) { // make stay in range
        missing_page_num = USERPGTBLSIZE -1;
    }
    
    // 6 pick a frame i from the swap pool..
    unsigned int frame_victim_num = pageReplacementAlgorithm();
    
    memaddr swap_pool_start = (memaddr)0x20020000;
    memaddr frame_victim_address = swap_pool_start + (frame_victim_num * PAGESIZE);
    unsigned int frame_victim = frame_victim_address >> 12;

    // 7 determine if frame i is occupied
    if (isSwapPoolFrameOccupied(frame_victim_num)) {
        // 8
        // ATOMIC start
        saved_status = getSTATUS();
        setSTATUS(saved_status & ~IECON); // disable interrupts

        // (a) Update process x’s Page Table: mark Page Table entry k as not valid
        swap_pool_table[frame_victim_num].sw_pte->pte_entryLO &= !VALIDON;
        

        // Update the TLB by using TLBP and TLBWI instead of TLBCLR [Section 5.2]
        // (b) Update the TLB, if needed = mark it invalid if present
        setENTRYHI(swap_pool_table[frame_victim_num].sw_pte->pte_entryHI);
        TLBP();
        if ((getINDEX() & (1 << 31)) == 0) { //matching found
            setENTRYHI(swap_pool_table[frame_victim_num].sw_pte->pte_entryHI);
            setENTRYLO(swap_pool_table[frame_victim_num].sw_pte->pte_entryLO);
            TLBWI();
        } 
        

        setSTATUS(saved_status);
        // ATOMIC end

        // (c) Update process x’s backing store. Write the contents of frame i to the correct location on
        //     process x’s backing store/flash device
        unsigned int write_status = writeFrameToFlashDev(swap_pool_table[frame_victim_num].sw_asid, frame_victim_address, frame_victim_num);
        if (write_status != READY) { // if is not ready or busy
            flash_status_debug(write_status);
            // trap
            support_trap_exception_handler(support_data);
            KLOG_ERROR("flash writr not succesful")
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
        support_trap_exception_handler(support_data);
        KLOG_ERROR("flash read not succesful")
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

    // Update the TLB by using TLBP and TLBWI instead of TLBCLR [Section 5.2]
    // 12 update the tlb by writing in a random spot or replacing wrong entry
    setENTRYHI(support_data->sup_privatePgTbl[missing_page_num].pte_entryHI);
    setENTRYLO(support_data->sup_privatePgTbl[missing_page_num].pte_entryLO);
    TLBP();
    
    if ((getINDEX() & (1 << 31)) == 0) { // found
        TLBWI();
    } else {
        TLBWR();
    }
    
    setSTATUS(saved_status);
    // ATOMIC end

    // 13
    releaseSwap();
    
    // 14 return control to the current process
    LDST(exceptstate);
}
