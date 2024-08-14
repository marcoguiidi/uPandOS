#ifndef SST_H
#define  SST_H

/**
The System Service Thread (SST) is a per-process thread that provide is child process useful services.
Each SST child process can send a message to its SST to request a service.
SST create a child process that executes one of the test U-proc. SST shares the same ASID and
support structure with its child U-proc. After its child initialization, the SST will wait for service
requests from its child process (just like the SSI of the previous phase).

GetTOD
Terminate
WritePrinter
WriteTerminal
 */
void SST_function_entry_point();

#endif