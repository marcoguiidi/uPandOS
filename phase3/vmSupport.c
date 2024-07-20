/**
vmSupport.c: This module implements the TLB exception handler (The Pager). Since reading
and writing to each U-proc’s flash device is limited to supporting paging, this module should
also contain the function(s) for reading and writing flash devices. Additionally, the Swap Pool
table is local to this module. Instead of declaring them globally in initProc.c they can be
declared module-wide in vmSupport.c. The test function will now invoke a new “public” function
initSwapStructs which will do the work of initializing the Swap Pool table.
Technical Point: Since the code for the TLB-Refill event handler was replaced (without
relocating the function), uTLB_RefillHandler should still be found in the Level 3/Phase 2
exceptions.c file.
*/