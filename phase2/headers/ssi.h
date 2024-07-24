#ifndef SSI_H_INCLUDED
#define SSI_H_INCLUDED

#include "/usr/include/umps3/umps/libumps.h"
#include "/usr/include/umps3/umps/const.h"
#include "/usr/include/umps3/umps/cp0.h"

#include "../../phase1/headers/msg.h"
#include "../../phase1/headers/pcb.h"

/*
while (TRUE) {
    receive a request;              [X]
    satisfy the received request;
    send back the results;
}
*/
void SSI_function_entry_point();


/*
while (TRUE) {
    receive a request;
    satisfy the received request;   [X]
    send back the results;          [X]
}
*/
void SSIRequest(pcb_t* sender, int service, void* arg);

#define DISK_DEVICES_START       (memaddr*)DEV_REG_ADDR(3, 0)
#define FLASH_DEVICES_START      (memaddr*)DEV_REG_ADDR(4, 0)
#define ETHERNET_DEVICES_START   (memaddr*)DEV_REG_ADDR(5, 0)
#define PRINTER_DEVICES_START    (memaddr*)DEV_REG_ADDR(6, 0)
#define TERMINAL_DEVICES_START   (memaddr*)DEV_REG_ADDR(7, 0)

int devaddr_get_lineno_devno_regno(memaddr* devaddr, int* lineno, int* devno, int* regno);

#endif